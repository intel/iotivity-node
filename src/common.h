#ifndef __IOTIVITY_NODE_FUNCTIONS_INTERNAL_H__
#define __IOTIVITY_NODE_FUNCTIONS_INTERNAL_H__

#include <v8.h>
#include <nan.h>

#define VALIDATE_CALLBACK_RETURN_VALUE_TYPE(value, typecheck, message)        \
  if (!value->typecheck()) {                                                  \
    NanThrowTypeError(                                                        \
        message " callback return value type must satisfy " #typecheck "()"); \
  }

#define VALIDATE_ARGUMENT_COUNT(args, length)                \
  if ((args).Length() < (length)) {                          \
    return NanThrowRangeError("Need " #length " arguments"); \
  }

#define VALIDATE_ARGUMENT_TYPE(args, index, typecheck)                      \
  if (!(args)[(index)]->typecheck()) {                                      \
    return NanThrowTypeError("Argument " #index " must satisfy " #typecheck \
                             "()");                                         \
  }

#define VALIDATE_VALUE_TYPE(value, typecheck, message, failReturn) \
  if (!(value)->typecheck()) {                                     \
    NanThrowTypeError(message " must satisfy " #typecheck "()");   \
    return failReturn;                                             \
  }

#define VALIDATE_ARGUMENT_TYPE_OR_NULL(args, index, typecheck)              \
  if (!((args)[(index)]->typecheck() || (args)[(index)]->IsNull())) {       \
    return NanThrowTypeError("Argument " #index " must satisfy " #typecheck \
                             "() or IsNull()");                             \
  }

v8::Persistent<v8::Function> *persistentJSCallback_new(
    v8::Local<v8::Function> callback);
void persistentJSCallback_free(v8::Persistent<v8::Function> *callback);

#endif /* __IOTIVITY_NODE_FUNCTIONS_INTERNAL_H__ */
