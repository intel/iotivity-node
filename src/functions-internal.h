#ifndef __IOTIVITY_NODE_FUNCTIONS_INTERNAL_H__
#define __IOTIVITY_NODE_FUNCTIONS_INTERNAL_H__

#include <v8.h>

#define THROW_TYPECHECK_EXCEPTION( isolate, message ) \
		(isolate)->ThrowException( Exception::TypeError( \
			String::NewFromUtf8( (isolate), message ) ) )

#define VALIDATE_CALLBACK_RETURN_VALUE_TYPE( isolate, value, typecheck ) \
	if ( !value->typecheck() ) { \
		THROW_TYPECHECK_EXCEPTION( isolate, \
			"Callback return value type must satisfy " #typecheck "()" ); \
	}

#define VALIDATE_ARGUMENT_COUNT( isolate, args, length ) \
	if ( (args).Length() < (length) ) { \
		(isolate)->ThrowException( Exception::RangeError( \
			String::NewFromUtf8( (isolate), "Need " #length " arguments" ) ) ); \
		return; \
	}

#define VALIDATE_ARGUMENT_TYPE( isolate, args, index, typecheck  ) \
	if ( !(args)[ (index) ]->typecheck() ) { \
		THROW_TYPECHECK_EXCEPTION( isolate, \
			"Argument " #index " must satisfy " #typecheck "()" ); \
		return; \
	}

#define VALIDATE_ARGUMENT_TYPE_OR_NULL( isolate, args, index, typecheck  ) \
	if ( !( (args)[ (index) ]->typecheck() || (args)[ (index) ]->IsNull() ) ) { \
		THROW_TYPECHECK_EXCEPTION( isolate, \
			"Argument " #index " must satisfy " #typecheck "() or IsNull()" ); \
		return; \
	}

// Hash for storing JS callbacks indexed by UUID
extern v8::Persistent<v8::Object> *_callbacks;
extern int _uuidCounter;

#endif /* __IOTIVITY_NODE_FUNCTIONS_INTERNAL_H__ */
