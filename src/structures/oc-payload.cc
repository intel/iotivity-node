#include <nan.h>
#include "../common.h"
#include "oc-payload.h"

extern "C" {
#include <string.h>
#include <ocpayload.h>
}

// FIXME: Remove the definition below when the stack starts providing it
#define UUID_SIZE (16)

using namespace v8;

static bool c_OCRepPayload( OCRepPayload **p_payload, Local<Object> jsPayload );
static Local<Object> js_OCRepPayload( OCRepPayload *payload );

static Local<Value> stringOrUndefined( char *str ) {
	return ( str ? Local<Value>::Cast( NanNew<String>( str ) ) : Local<Value>::Cast( NanUndefined() ) );
}

static Local<Value> objectOrUndefined( OCRepPayload *payload ) {
	return ( payload ? Local<Value>::Cast( js_OCRepPayload( payload ) ) : Local<Value>::Cast( NanUndefined() ) );
}

static Local<Array> createPayloadValueArrayRecursively( OCRepPayloadValueArray *array, size_t *p_dataIndex, int dimensionIndex ) {
	size_t index;
	Local<Array> returnValue = NanNew<Array>( array->dimensions[ dimensionIndex ] );

	for ( index = 0 ; index < array->dimensions[ dimensionIndex ]; index++ ) {
		returnValue->Set( index,
			( dimensionIndex < MAX_REP_ARRAY_DEPTH - 1 && array->dimensions[ dimensionIndex + 1 ] > 0 ) ?

				// Fill with arrays
				Local<Value>::Cast( createPayloadValueArrayRecursively( array, p_dataIndex, dimensionIndex + 1 ) ) :

				// Fill with data
				( array->type == OCREP_PROP_INT ? Local<Value>::Cast( NanNew<Number>( array->iArray[ (*p_dataIndex)++ ] ) ) :
				array->type == OCREP_PROP_DOUBLE ? Local<Value>::Cast( NanNew<Number>( array->dArray[ (*p_dataIndex)++ ] ) ) :
				array->type == OCREP_PROP_BOOL ? Local<Value>::Cast( NanNew<Boolean>( array->bArray[ (*p_dataIndex)++ ] ) ) :
				array->type == OCREP_PROP_STRING ? stringOrUndefined( array->strArray[ ( *p_dataIndex)++ ] ) :

				// If the type is none of the above, we assume it must be an object
				objectOrUndefined( array->objArray[ ( *p_dataIndex)++ ] ) ) );
	}

	return returnValue;
}

static Local<Array> js_OCRepPayloadValueArray( OCRepPayloadValueArray *array ) {
	size_t dataIndex = 0;
	return createPayloadValueArrayRecursively( array, &dataIndex, 0 );
}

static Local<Object> js_OCRepPayload( OCRepPayload *payload ) {
	Local<Object> returnValue = NanNew<Object>();
	int count, index;
	OCStringLL *current;
	OCRepPayloadValue *value;

	// payload.type
	returnValue->Set( NanNew<String>( "type" ), NanNew<Number>( payload->base.type ) );

	// payload.uri
	if ( payload->uri ) {
		returnValue->Set( NanNew<String>( "uri" ), NanNew<String>( payload->uri ) );
	}

	// payload.types
	if ( payload->types ) {

		// Count the items
		for ( current = payload->types, count = 0; current; current = current->next, count++ );

		Local<Array> types = NanNew<Array>( count );
		for ( current = payload->types, index = 0; current; current = current->next, index++ ) {
			types->Set( index, current->value ? Local<Value>::Cast( NanNew<String>( current->value ) ) : Local<Value>::Cast( NanUndefined() ) );
		}

		returnValue->Set( NanNew<String>( "types" ), types );
	}

	// payload.interfaces
	if ( payload->interfaces ) {

		// Count the items
		for ( current = payload->interfaces, count = 0; current; current = current->next, count++ );

		Local<Array> interfaces = NanNew<Array>( count );
		for ( current = payload->interfaces, index = 0; current; current = current->next, index++ ) {
			interfaces->Set( index, current->value ? Local<Value>::Cast( NanNew<String>( current->value ) ) : Local<Value>::Cast( NanUndefined() ) );
		}

		returnValue->Set( NanNew<String>( "interfaces" ), interfaces );
	}

	// payload.values
	if ( payload->values ) {
		Local<Object> values = NanNew<Object>();
		for ( value = payload->values ; value ; value = value->next ) {
			values->Set( NanNew<String>( value->name ),
				OCREP_PROP_INT == value->type ? Local<Value>::Cast( NanNew<Number>( value->i ) ) :
				OCREP_PROP_DOUBLE == value->type ? Local<Value>::Cast( NanNew<Number>( value->d ) ) :
				OCREP_PROP_BOOL == value->type ? Local<Value>::Cast( NanNew<Boolean>( value->b ) ) :
				OCREP_PROP_STRING == value->type ?
					( value->str ?
						Local<Value>::Cast( NanNew<String>( value->str ) ) :
						Local<Value>::Cast( NanUndefined() ) ) :
				OCREP_PROP_OBJECT == value->type ?
					( value->obj ?
						Local<Value>::Cast( js_OCRepPayload( value->obj ) ) :
						Local<Value>::Cast( NanUndefined() ) ) :
				OCREP_PROP_ARRAY == value->type ? Local<Value>::Cast( js_OCRepPayloadValueArray( &( value->arr ) ) ) :

				// If value->type is not any of the above, then we assume it must be OCREP_PROP_NULL
				Local<Value>::Cast( NanNull() ) );
		}
		returnValue->Set( NanNew<String>( "values" ), values );
	}

	// payload.next
	if ( payload->next ) {
		returnValue->Set( NanNew<String>( "next" ), js_OCRepPayload( payload->next ) );
	}

	return returnValue;
}

Local<Object> js_OCResourcePayload( OCResourcePayload *payload ) {
	int index, length;
	OCStringLL *item;
	Local<Object> returnValue = NanNew<Object>();

	// payload.uri
	returnValue->Set( NanNew<String>( "uri" ), NanNew<String>( payload->uri ) );

	// payload.sid
	Local<Array> sid = NanNew<Array>( UUID_SIZE );
	for ( index = 0 ; index < UUID_SIZE ; index++ ) {
		sid->Set( index, NanNew<Number>( payload->sid[ index ] ) );
	}
	returnValue->Set( NanNew<String>( "sid" ), sid );

	// payload.types
	for ( item = payload->types, length = 0; item; item = item->next, length++ );
	Local<Array> types = NanNew<Array>( length );
	for ( item = payload->types, index = 0; item; item = item->next, index++ ) {
		types->Set( index, NanNew<String>( item->value ) );
	}
	returnValue->Set( NanNew<String>( "types" ), types );

	// payload.interfaces
	for ( item = payload->interfaces, length = 0; item; item = item->next, length++ );
	Local<Array> interfaces = NanNew<Array>( length );
	for ( item = payload->interfaces, index = 0; item; item = item->next, index++ ) {
		interfaces->Set( index, NanNew<String>( item->value ) );
	}
	returnValue->Set( NanNew<String>( "interfaces" ), interfaces );

	// payload.bitmap
	returnValue->Set( NanNew<String>( "bitmap" ), NanNew<Number>( payload->bitmap ) );

	// payload.secure
	returnValue->Set( NanNew<String>( "secure" ), NanNew<Boolean>( payload->secure ) );

	// payload.port
	returnValue->Set( NanNew<String>( "port" ), NanNew<Number>( payload->port ) );

	return returnValue;
}

Local<Object> js_OCDiscoveryPayload( OCDiscoveryPayload *payload ) {
	Local<Object> returnValue = NanNew<Object>();
	OCResourcePayload *resource = payload->resources;
	int counter = 0;

	returnValue->Set( NanNew<String>( "type" ), NanNew<Number>( payload->base.type ) );

	// Count the resources
	while( resource ) {
		counter++;
		resource = resource->next;
	}

	Local<Array> resources = NanNew<Array>( counter );

	for ( resource = payload->resources, counter = 0; resource; resource = resource->next, counter++ ) {
		resources->Set( counter, js_OCResourcePayload( resource ) );
	}

	returnValue->Set( NanNew<String>( "resources" ), resources );

	return returnValue;
}

Local<Value> js_OCPayload( OCPayload *payload ) {
	switch( payload->type ) {
		case PAYLOAD_TYPE_DISCOVERY:
			return js_OCDiscoveryPayload( ( OCDiscoveryPayload * )payload );

		case PAYLOAD_TYPE_REPRESENTATION:
			return js_OCRepPayload( ( OCRepPayload * )payload );

		case PAYLOAD_TYPE_INVALID:
		default:
			printf( "js_OCPayload: payload->type: %d\n", payload->type );
			break;
	}
	return NanNull();
}

// Convert the type of the Javascript value to OCRepPayloadPropType, but only for those types that
// may appear in arrays. Throw an exception otherwise.
static bool jsTypeToOCRepPayloadPropTypeValidForArray( Local<Value> value, OCRepPayloadPropType *p_type ) {

	// Always check integer before number
	if ( value->IsUint32() ) {
		*p_type = OCREP_PROP_INT;
		return true;
	} else if ( value->IsNumber() ) {
		*p_type = OCREP_PROP_DOUBLE;
		return true;
	} else if ( value->IsBoolean() ) {
		*p_type = OCREP_PROP_BOOL;
		return true;
	} else if ( value->IsString() ) {
		*p_type = OCREP_PROP_STRING;
		return true;
	} else if ( value->IsObject() ) {
		*p_type = OCREP_PROP_OBJECT;
		return true;
	}

	NanThrowError( "Value type not allowed in rep payload" );
	return false;
}

// Validate a multidimensional array of items of a given type
//
// If the array passed in contains arrays, their validity is checked recursively, and it is
// asserted that they all have the same length as the first one encountered. Recursions are limited
// to MAX_REP_ARRAY_DEPTH.
//
// If the array passed in contains primitive types, the type of the array is established from the
// first item, and it is asserted that subsequent items have the same type.
//
// If the array is valid, p_typeEstablished, p_arrayType, and dimensions are set and the function
// returns true. Otherwise, it throws an exception and returns false.
static bool validateRepPayloadArray(
		Local<Array> array,
		bool *p_typeEstablished,
		OCRepPayloadPropType *p_arrayType,
		size_t dimensions[ MAX_REP_ARRAY_DEPTH ],
		int index ) {
	if ( index >= MAX_REP_ARRAY_DEPTH ) {
		NanThrowError( "Rep payload array has too many dimensions" );
		return false;
	}

	size_t length = array->Length();

	if ( length > 0 )  {
		Local<Value> firstValue = array->Get( 0 );
		if ( firstValue->IsArray() ) {
			size_t child_length = Local<Array>::Cast( firstValue )->Length();
			for ( size_t arrayIndex = 0 ; arrayIndex < length ; arrayIndex++ ) {
				Local<Value> member = array->Get( arrayIndex );
				if ( !member->IsArray() ) {
					NanThrowError( "Rep payload array is heterogeneous" );
					return false;
				}

				bool child_established;
				OCRepPayloadPropType child_type;
				Local<Array> child_array = Local<Array>::Cast( member );
				if ( child_array->Length() != child_length ) {
					NanThrowError( "Rep payload array contains child arrays of different lengths" );
					return false;
				}
				if ( !validateRepPayloadArray( child_array, &child_established, &child_type, dimensions, index + 1 ) ) {
					return false;
				}

				// Reconcile array types
				if ( *p_typeEstablished ) {
					if ( !child_established || child_type != *p_arrayType ) {
						NanThrowError( "Rep payload array is heterogeneous" );
						return false;
					}
				} else {
					if ( child_established ) {
						*p_typeEstablished = true;
						*p_arrayType = child_type;
					}
				}
			}
		} else {
			OCRepPayloadPropType valueType;

			if ( !jsTypeToOCRepPayloadPropTypeValidForArray( firstValue, &valueType ) ) {
				return false;
			}

			if ( *p_typeEstablished ) {
				if ( valueType != *p_arrayType ) {
					NanThrowError( "Rep payload array is heterogeneous" );
					return false;
				}
			} else {
				*p_typeEstablished = true;
				*p_arrayType = valueType;
			}

			for ( size_t arrayIndex = 1 ; arrayIndex < length ; arrayIndex++ ) {
				if ( !jsTypeToOCRepPayloadPropTypeValidForArray( array->Get( arrayIndex ), &valueType ) ) {
					return false;
				}
				if ( valueType != *p_arrayType ) {
					NanThrowError( "Rep payload array is heterogeneous" );
				}
			}
		}
	}

	dimensions[ index ] = length;
	return true;
}

// Fill the array
// p_index contains the index into flatArray where the next item will land. It is passed by
// reference into the recursive instances so that it advances monotonically.
static bool fillArray( void *flatArray, int *p_index, Local<Array> array, OCRepPayloadPropType arrayType ) {
	size_t length = array->Length();

	for ( size_t localIndex = 0 ; localIndex < length ; localIndex++ ) {
		Local<Value> member = array->Get( localIndex );
		if ( member->IsArray() ) {
			if ( !fillArray( flatArray, p_index, array, arrayType ) ) {
				return false;
			} else {
				switch( arrayType ) {
					case OCREP_PROP_INT:
						( ( uint64_t * )flatArray )[ (*p_index)++ ] = ( uint64_t )member->Uint32Value();
						break;

					case OCREP_PROP_DOUBLE:
						( ( double * )flatArray )[ (*p_index)++ ] = member->NumberValue();
						break;

					case OCREP_PROP_BOOL:
						( ( bool * )flatArray )[ (*p_index)++ ] = member->BooleanValue();
						break;

					case OCREP_PROP_STRING:
						{
							size_t length = strlen( (const char *)*String::Utf8Value( member ) ) + 1;
							char *theString = (char *)malloc( length );
							if ( theString ) {
								theString[ length - 1 ] = 0;
								strncpy( theString, (const char *)*String::Utf8Value( member ), length - 1 );
								( ( char ** )flatArray )[ (*p_index)++ ] = theString;
							} else {

								// If we fail to copy the string, we free all strings allocated so far and we quit.
								for ( int freeIndex = 0 ; freeIndex < (*p_index) ; freeIndex++ ) {
									free( ( ( char ** )flatArray )[ freeIndex ] );
								}
								return false;
							}
							break;
						}

					case OCREP_PROP_OBJECT:
						{
							OCRepPayload *theObject = 0;
							if ( c_OCRepPayload( &theObject, member->ToObject() ) ) {
								( ( OCRepPayload ** )flatArray )[ (*p_index)++ ] = theObject;
							} else {

								// If we fail to create the object, we free all objects allocated so far and we quit.
								for ( int freeIndex = 0 ; freeIndex < (*p_index) ; freeIndex++ ) {
									OCRepPayloadDestroy( ( ( OCRepPayload ** )flatArray )[ freeIndex ] );
								}
								return false;
							}
							break;
						}

					// The validation should prevent these from occurring
					case OCREP_PROP_NULL:
					case OCREP_PROP_ARRAY:
					default:
						break;
				}
			}
		}
	}

	return true;
}

static bool flattenArray( Local<Array> array, void **flatArray, size_t dimensions[ MAX_REP_ARRAY_DEPTH ], OCRepPayloadPropType arrayType ) {
	size_t totalElements = 1;
	void *returnValue = 0;

	for ( size_t dimensionIndex = 0 ; dimensionIndex < MAX_REP_ARRAY_DEPTH && dimensions[ dimensionIndex ] > 0; dimensionIndex++ ) {
		totalElements *= dimensions[ dimensionIndex ];
	}

	returnValue = malloc(
		( arrayType == OCREP_PROP_INT ? sizeof( uint64_t ) :
			arrayType == OCREP_PROP_DOUBLE ? sizeof( double ) :
			arrayType == OCREP_PROP_BOOL ? sizeof( bool ) :
			arrayType == OCREP_PROP_STRING ? sizeof( char * ) :
			arrayType == OCREP_PROP_OBJECT ? sizeof( OCRepPayload * ) :

			// The validation ensures that the array type is always valid
			0 ) * totalElements );

	if ( !returnValue ) {
		NanThrowError( "Not enough memory for flattening rep payload array" );
		return false;
	}

	int index = 0;
	if ( !fillArray( returnValue, &index, array, arrayType ) ) {
		free( returnValue );
		return false;
	}

	*flatArray = returnValue;
	return true;
}

static bool c_OCRepPayload( OCRepPayload **p_payload, Local<Object> jsPayload ) {
	uint32_t index, length;
	OCRepPayload *payload = OCRepPayloadCreate();

	// reppayload.uri
	if ( jsPayload->Has( NanNew<String>( "uri" ) ) ) {
		Local<Value> uri = jsPayload->Get( NanNew<String>( "uri" ) );
		VALIDATE_VALUE_TYPE_OR_FREE( uri, IsString, "reppayload.uri", false, payload, OCRepPayloadDestroy );
		OCRepPayloadSetUri( payload, (const char *)*String::Utf8Value( uri ) );
	}

	// reppayload.types
	if ( jsPayload->Has( NanNew<String>( "types" ) ) ) {
		Local<Value> types = jsPayload->Get( NanNew<String>( "types" ) );
		VALIDATE_VALUE_TYPE_OR_FREE( types, IsArray, "reppayload.types", false, payload, OCRepPayloadDestroy );
		Local<Array> typesArray = Local<Array>::Cast( types );
		length = typesArray->Length();
		for ( index = 0 ; index < length ; index++ ) {
			Local<Value> singleType = typesArray->Get( index );
			VALIDATE_VALUE_TYPE_OR_FREE( singleType, IsString, "reppayload.types item", false, payload, OCRepPayloadDestroy );
			OCRepPayloadAddResourceType( payload, ( const char *)*String::Utf8Value( singleType ) );
		}
	}

	// reppayload.interfaces
	if ( jsPayload->Has( NanNew<String>( "interfaces" ) ) ) {
		Local<Value> interfaces = jsPayload->Get( NanNew<String>( "interfaces" ) );
		VALIDATE_VALUE_TYPE_OR_FREE( interfaces, IsArray, "reppayload.interfaces", false, payload, OCRepPayloadDestroy );
		Local<Array> interfacesArray = Local<Array>::Cast( interfaces );
		length = interfacesArray->Length();
		for ( index = 0 ; index < length ; index++ ) {
			Local<Value> singleInterface = interfacesArray->Get( index );
			VALIDATE_VALUE_TYPE_OR_FREE( singleInterface, IsString, "reppayload.interfaces item", false, payload, OCRepPayloadDestroy );
			OCRepPayloadAddInterface( payload, ( const char *)*String::Utf8Value( singleInterface ) );
		}
	}

	// reppayload.values
	if ( jsPayload->Has( NanNew<String>( "values" ) ) ) {
		Local<Value> values = jsPayload->Get( NanNew<String>( "values" ) );
		VALIDATE_VALUE_TYPE_OR_FREE( values, IsObject, "reppayload.values", false, payload, OCRepPayloadDestroy );
		Local<Object> valuesObject = Local<Object>::Cast( values );
		Local<Array> keys = valuesObject->GetPropertyNames();
		length = keys->Length();
		for ( index = 0 ; index < length ; index++ ) {
			Local<Value> value = valuesObject->Get( keys->Get( index )->ToString() );
			if ( value->IsNull() ) {
				if ( !OCRepPayloadSetNull( payload, (const char *)*String::Utf8Value( keys->Get( index ) ) ) ) {
					NanThrowError( "reppayload: Failed to set null property" );
					OCRepPayloadDestroy( payload );
					return false;
				}
			} else if ( value->IsUint32() ) {
				if ( !OCRepPayloadSetPropInt( payload, (const char *)*String::Utf8Value( keys->Get( index ) ),
						(int64_t)value->Uint32Value() ) ) {
					OCRepPayloadDestroy( payload );
					NanThrowError( "reppayload: Failed to set integer property" );
					return false;
				}
			} else if ( value->IsNumber() ) {
				if ( !OCRepPayloadSetPropDouble( payload, (const char *)*String::Utf8Value( keys->Get( index ) ),
						value->NumberValue() ) ) {
					OCRepPayloadDestroy( payload );
					NanThrowError( "reppayload: Failed to set floating point property" );
					return false;
				}
			} else if ( value->IsBoolean() ) {
				if ( !OCRepPayloadSetPropBool( payload, (const char *)*String::Utf8Value( keys->Get( index ) ),
						value->BooleanValue() ) ) {
					NanThrowError( "reppayload: Failed to set boolean property" );
					OCRepPayloadDestroy( payload );
					return false;
				}
			} else if ( value->IsString() ) {
				if ( !OCRepPayloadSetPropString( payload, (const char *)*String::Utf8Value( keys->Get( index ) ),
						(const char *)*String::Utf8Value( value ) ) ) {
					NanThrowError( "reppayload: Failed to set string property" );
					OCRepPayloadDestroy( payload );
					return false;
				}
			} else if ( value->IsObject() ) {
				OCRepPayload *child_payload = 0;

				if ( c_OCRepPayload( &child_payload, value->ToObject() ) ) {
					if ( !OCRepPayloadSetPropObjectAsOwner( payload, (const char *)*String::Utf8Value( keys->Get( index ) ),
							child_payload ) ) {
						NanThrowError( "reppayload: Failed to set object property" );
						OCRepPayloadDestroy( payload );
						return false;
					}
				} else {
					OCRepPayloadDestroy( payload );
					return false;
				}
			} else if ( value->IsArray() ) {
				size_t dimensions[ MAX_REP_ARRAY_DEPTH ] = { 0 };
				bool typeEstablished = false;
				OCRepPayloadPropType arrayType;
				Local<Array> array = Local<Array>::Cast( value );
				if ( !validateRepPayloadArray( array, &typeEstablished, &arrayType, dimensions, 0 ) ) {
					OCRepPayloadDestroy( payload );
					return false;
				}

				if ( dimensions[ 0 ] > 0 ) {
					void *flatArray;

					if ( !flattenArray( array, &flatArray, dimensions, arrayType ) ) {
						OCRepPayloadDestroy( payload );
						return false;
					}
				}
			}
		}
	}

	if ( jsPayload->Has( NanNew<String>( "next" ) ) ) {
		Local<Value> next = jsPayload->Get( NanNew<String>( "next" ) );
		VALIDATE_VALUE_TYPE_OR_FREE( next, IsObject, "reppayload.next", false, payload, OCRepPayloadDestroy );
		OCRepPayload *next_payload = 0;
		if ( !c_OCRepPayload( &next_payload, next->ToObject() ) ) {
			OCRepPayloadDestroy( payload );
			return false;
		}
		OCRepPayloadAppend( payload, next_payload );
	}

	( *p_payload ) = payload;
	return true;
}

bool c_OCPayload( OCPayload **p_payload, Local<Object> jsPayload ) {
	if ( !jsPayload->IsNull() ) {
		Local<Value> type = jsPayload->Get( NanNew<String>( "type" ) );
		VALIDATE_VALUE_TYPE( type, IsNumber, "payload.type", false );

		switch( type->Uint32Value() ) {
			case PAYLOAD_TYPE_REPRESENTATION:
				return c_OCRepPayload( ( OCRepPayload ** )p_payload, jsPayload );

			default:
				NanThrowError( "Support for this payload type not implemented" );
				return false;
		}
	} else {
		*p_payload = 0;
	}
	return true;
}
