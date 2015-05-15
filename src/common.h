#ifndef __IOTIVITY_NODE_FUNCTIONS_INTERNAL_H__
#define __IOTIVITY_NODE_FUNCTIONS_INTERNAL_H__

#include <nan.h>

#define VALIDATE_CALLBACK_RETURN_VALUE_TYPE( isolate, value, typecheck ) \
	if ( !value->typecheck() ) { \
		NanThrowTypeError( "Callback return value type must satisfy " #typecheck "()" ); \
	}

#define VALIDATE_ARGUMENT_COUNT( args, length ) \
	if ( (args).Length() < (length) ) { \
		return NanThrowRangeError( "Need " #length " arguments" ); \
	}

#define VALIDATE_ARGUMENT_TYPE( args, index, typecheck  ) \
	if ( !(args)[ (index) ]->typecheck() ) { \
		return NanThrowTypeError( "Argument " #index " must satisfy " #typecheck "()" ); \
	}

#define VALIDATE_VALUE_TYPE( value, typecheck, message, failReturn ) \
	if ( !(value)->typecheck() ) { \
		NanThrowTypeError( message " must satisfy " #typecheck "()" ); \
		return failReturn; \
	}

#define VALIDATE_ARGUMENT_TYPE_OR_NULL( args, index, typecheck  ) \
	if ( !( (args)[ (index) ]->typecheck() || (args)[ (index) ]->IsNull() ) ) { \
		return NanThrowTypeError( "Argument " #index " must satisfy " #typecheck "() or IsNull()" ); \
	}

#endif /* __IOTIVITY_NODE_FUNCTIONS_INTERNAL_H__ */
