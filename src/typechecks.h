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

#define THROW_RANGE_EXCEPTION( isolate, message ) \
	(isolate)->ThrowException( Exception::RangeError( \
		String::NewFromUtf8( (isolate), message ) ) )

#define VALIDATE_ARGUMENT_COUNT( isolate, args, length ) \
	if ( (args).Length() < (length) ) { \
		THROW_RANGE_EXCEPTION( (isolate), "Need " #length " arguments" ); \
		return; \
	}

#define VALIDATE_ARGUMENT_TYPE( isolate, args, index, typecheck  ) \
	if ( !(args)[ (index) ]->typecheck() ) { \
		THROW_TYPECHECK_EXCEPTION( isolate, \
			"Argument " #index " must satisfy " #typecheck "()" ); \
		return; \
	}

#define VALIDATE_VALUE_TYPE( isolate, value, typecheck, message, failReturn ) \
	if ( !(value)->typecheck() ) { \
		THROW_TYPECHECK_EXCEPTION( isolate, \
			message " must satisfy " #typecheck "()" ); \
		return failReturn; \
	}

#define VALIDATE_ARGUMENT_TYPE_OR_NULL( isolate, args, index, typecheck  ) \
	if ( !( (args)[ (index) ]->typecheck() || (args)[ (index) ]->IsNull() ) ) { \
		THROW_TYPECHECK_EXCEPTION( isolate, \
			"Argument " #index " must satisfy " #typecheck "() or IsNull()" ); \
		return; \
	}

#endif /* __IOTIVITY_NODE_FUNCTIONS_INTERNAL_H__ */
