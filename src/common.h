/*
 * Copyright 2016 Intel Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __IOTIVITY_NODE_FUNCTIONS_INTERNAL_H__
#define __IOTIVITY_NODE_FUNCTIONS_INTERNAL_H__

#include <memory>
#include <string>
extern "C" {
#include <node_api.h>
#include <ocstack.h>
}

#define PENDING_EXCEPTION "Pending exception"

// A line that looks like a stack frame from a JS exception
#define SOURCE_LOCATION                                        \
  (std::string("\n    at ") + std::string(__func__) +          \
   std::string(" (" __FILE__ ":") + std::to_string(__LINE__) + \
   std::string(")"))

// The top of what looks like a JS exception, but with native addresses
#define LOCAL_MESSAGE(message) (std::string("") + message + SOURCE_LOCATION)

// Appends a frame to what looks like a JS exception. __resultingStatus is a
// variable that carries the stack trace information. Most of the time
// __VA_ARGS__ is either a return statement or a throw followed by a void
// return. Both refer to __resultingStatus.
#define FAIL_STATUS (__resultingStatus + SOURCE_LOCATION)

// Goes down the bail-with-stack-trace-like-string path if the condition fails.
#define JS_ASSERT(condition, message, ...)         \
  do {                                             \
    std::string __resultingStatus;                 \
    if (!((condition))) {                          \
      __resultingStatus = std::string() + message; \
      __VA_ARGS__;                                 \
    }                                              \
  } while (0)

// Converts a non-napi_ok status to a string that looks like an exception.
#define NAPI_CALL(env, theCall, ...)                                       \
  do {                                                                     \
    napi_status status = theCall;                                          \
    if (status != napi_ok) {                                               \
      const napi_extended_error_info *error_info = 0;                      \
      napi_get_last_error_info((env), &error_info);                        \
      std::string __resultingStatus = std::string(                         \
          (status == napi_pending_exception) ? PENDING_EXCEPTION           \
                                             : error_info->error_message); \
      __VA_ARGS__;                                                         \
    }                                                                      \
  } while (0)

// Bails if a helper returns a non-empty std::string. The __VA_ARGS__ either
// append the frame info or throw a JS exception.
#define HELPER_CALL(theCall, ...)            \
  do {                                       \
    std::string __resultingStatus = theCall; \
    if (!__resultingStatus.empty()) {        \
      __VA_ARGS__;                           \
    }                                        \
  } while (0)

#define DECLARE_VALUE_TYPE(varName, env, value, ...) \
  napi_valuetype varName;                            \
  NAPI_CALL((env), napi_typeof((env), (value), &varName), __VA_ARGS__);

#define J2C_VALIDATE_VALUE_TYPE(env, value, typecheck, message, ...)   \
  do {                                                                 \
    DECLARE_VALUE_TYPE(theType, (env), value, __VA_ARGS__);            \
    JS_ASSERT(theType == (typecheck),                                  \
              std::string() + message + " is not a " #typecheck " (" + \
                  std::to_string(theType) + ")",                       \
              __VA_ARGS__);                                            \
  } while (0)

#define J2C_ASSIGN_PROPERTY_JS(env, source, name, destination, ...)          \
  do {                                                                       \
    NAPI_CALL((env),                                                         \
              napi_get_named_property((env), (source), name, (destination)), \
              __VA_ARGS__);                                                  \
  } while (0)

#define J2C_DECLARE_PROPERTY_JS(varName, env, source, name, ...) \
  napi_value varName;                                            \
  J2C_ASSIGN_PROPERTY_JS((env), (source), (name), &varName, __VA_ARGS__)

#define J2C_ASSIGN_VALUE_JS(cType, destination, env, source, jsType, message, \
                            getterSuffix, jsParameterType, ...)               \
  do {                                                                        \
    jsParameterType fromJSValue;                                              \
    J2C_VALIDATE_VALUE_TYPE((env), (source), (jsType), message, __VA_ARGS__); \
    NAPI_CALL((env),                                                          \
              napi_get_value_##getterSuffix((env), (source), &fromJSValue),   \
              __VA_ARGS__);                                                   \
    (destination) = (cType)fromJSValue;                                       \
  } while (0)

#define J2C_DECLARE_VALUE_JS(cType, variableName, env, source, jsType,       \
                             message, getterSuffix, jsParameterType, ...)    \
  cType variableName;                                                        \
  J2C_ASSIGN_VALUE_JS(cType, variableName, (env), (source), jsType, message, \
                      getterSuffix, jsParameterType, __VA_ARGS__);

#define J2C_ASSIGN_STRING_JS(env, destination, source, message, ...)         \
  do {                                                                       \
    size_t length;                                                           \
    NAPI_CALL((env), napi_get_value_string_utf8((env), (source), nullptr, 0, \
                                                &length),                    \
              __VA_ARGS__);                                                  \
    std::unique_ptr<char> cString(new char[length + 1]());                   \
    JS_ASSERT(cString.get(),                                                 \
              std::string("") + "Failed to allocate memory for" + message,   \
              __VA_ARGS__);                                                  \
    size_t bytesWritten;                                                     \
    NAPI_CALL((env),                                                         \
              napi_get_value_string_utf8((env), (source), cString.get(),     \
                                         length + 1, &bytesWritten),         \
              __VA_ARGS__);                                                  \
    (destination) = cString.release();                                       \
  } while (0)

#define J2C_GET_STRING_JS(env, destination, source, nullOk, message, ...)     \
  do {                                                                        \
    DECLARE_VALUE_TYPE(valueType, env, (source), __VA_ARGS__);                \
    if (nullOk && (valueType == napi_null || valueType == napi_undefined)) {  \
      (destination) = nullptr;                                                \
    } else {                                                                  \
      JS_ASSERT((valueType == napi_string),                                   \
                std::string("") + message + " is not a string", __VA_ARGS__); \
      J2C_ASSIGN_STRING_JS((env), (destination), (source), message,           \
                           __VA_ARGS__);                                      \
    }                                                                         \
  } while (0)

#define J2C_GET_STRING(env, destination, source, nullOk, name, ...)       \
  do {                                                                    \
    (destination) = nullptr;                                              \
    J2C_DECLARE_PROPERTY_JS(jsValue, (env), (source), name, __VA_ARGS__); \
    J2C_GET_STRING_JS((env), (destination), jsValue, (nullOk),            \
                      #destination "." name, __VA_ARGS__);                \
  } while (0)

#define J2C_GET_STRING_TRACKED_JS(varName, env, source, nullOk, message, ...)  \
  std::unique_ptr<char> __##varName##__tracker;                                \
  char *varName = nullptr;                                                     \
  J2C_GET_STRING_JS((env), varName, (source), (nullOk), message, __VA_ARGS__); \
  __##varName##__tracker.reset(varName);

#define J2C_VALIDATE_IS_ARRAY(env, theValue, nullOk, message, ...)          \
  do {                                                                      \
    DECLARE_VALUE_TYPE(jsType, (env), theValue, __VA_ARGS__);               \
    if (!((nullOk) && (jsType == napi_null || jsType == napi_undefined))) { \
      bool isArray;                                                         \
      NAPI_CALL((env), napi_is_array((env), (theValue), &isArray),          \
                __VA_ARGS__);                                               \
      JS_ASSERT(isArray, std::string() + message + " is not an array",      \
                __VA_ARGS__);                                               \
    }                                                                       \
  } while (0)

// Macros used in helpers - they cause the function to return a std::string

#define NAPI_CALL_RETURN(env, theCall) \
  NAPI_CALL((env), theCall, return FAIL_STATUS)

#define HELPER_CALL_RETURN(theCall) HELPER_CALL(theCall, return FAIL_STATUS)

#define DECLARE_VALUE_TYPE_RETURN(varName, env, value) \
  DECLARE_VALUE_TYPE(varName, (env), (value), return FAIL_STATUS)

#define J2C_ASSIGN_PROPERTY_JS_RETURN(env, source, name, destination) \
  J2C_ASSIGN_PROPERTY_JS((env), (source), (name), (destination),      \
                         return FAIL_STATUS)

#define J2C_DECLARE_PROPERTY_JS_RETURN(varName, env, source, name) \
  J2C_DECLARE_PROPERTY_JS(varName, env, source, name, return FAIL_STATUS)

#define J2C_VALIDATE_VALUE_TYPE_RETURN(env, value, typecheck, message) \
  J2C_VALIDATE_VALUE_TYPE((env), (value), (typecheck), message,        \
                          return FAIL_STATUS)

#define J2C_VALIDATE_IS_ARRAY_RETURN(env, theValue, nullOk, message) \
  J2C_VALIDATE_IS_ARRAY((env), (theValue), (nullOk), message,        \
                        return FAIL_STATUS)

#define J2C_GET_STRING_RETURN(env, destination, source, nullOk, name) \
  J2C_GET_STRING((env), (destination), (source), (nullOk), name,      \
                 return FAIL_STATUS)

#define J2C_ASSIGN_STRING_JS_RETURN(env, destination, source, message) \
  J2C_ASSIGN_STRING_JS((env), (destination), (source), message,        \
                       return FAIL_STATUS)

#define J2C_GET_STRING_JS_RETURN(env, destination, source, nullOk, message) \
  J2C_GET_STRING_JS((env), (destination), (source), (nullOk), message,      \
                    return FAIL_STATUS)

#define J2C_DECLARE_VALUE_JS_RETURN(cType, variableName, env, source, jsType, \
                                    message, getterSuffix, jsParameterType)   \
  J2C_DECLARE_VALUE_JS(cType, variableName, (env), (source), jsType, message, \
                       getterSuffix, jsParameterType, return FAIL_STATUS)

#define J2C_ASSIGN_VALUE_JS_RETURN(cType, destination, env, source, jsType, \
                                   message, getterSuffix, jsParameterType)  \
  J2C_ASSIGN_VALUE_JS(cType, destination, env, source, jsType, message,     \
                      getterSuffix, jsParameterType, return FAIL_STATUS)

#define J2C_ASSIGN_MEMBER_VALUE_RETURN(env, destination, source, cType, name, \
                                       jsType, message, getterSuffix,         \
                                       jsParameterType)                       \
  do {                                                                        \
    J2C_DECLARE_PROPERTY_JS_RETURN(jsValue, (env), (source), #name);          \
    J2C_ASSIGN_VALUE_JS(cType, (destination)->name, (env), jsValue, jsType,   \
                        std::string() + message + "." + #name, getterSuffix,  \
                        jsParameterType, return FAIL_STATUS);                 \
  } while (0)

#define C2J_SET_PROPERTY_CALL_RETURN(env, destination, name, call)        \
  do {                                                                    \
    napi_value jsValue;                                                   \
    call;                                                                 \
    NAPI_CALL_RETURN((env), napi_set_named_property((env), (destination), \
                                                    (name), jsValue));    \
  } while (0)

#define C2J_SET_PROPERTY_RETURN(env, destination, name, type, ...) \
  C2J_SET_PROPERTY_CALL_RETURN(                                    \
      (env), (destination), name,                                  \
      NAPI_CALL_RETURN((env),                                      \
                       napi_create_##type((env), __VA_ARGS__, &jsValue)))

#define C2J_SET_NUMBER_MEMBER_RETURN(env, destination, source, name) \
  C2J_SET_PROPERTY_RETURN((env), (destination), #name, double, (source)->name)

#define C2J_SET_STRING_IF_NOT_NULL_RETURN(env, destination, source, name) \
  if ((source)->name) {                                                   \
    C2J_SET_PROPERTY_RETURN((env), (destination), #name, string_utf8,     \
                            (source)->name, strlen((source)->name));      \
  }

#define J2C_GET_STRING_TRACKED_JS_RETURN(varName, env, source, nullOk,   \
                                         message)                        \
  J2C_GET_STRING_TRACKED_JS(varName, (env), (source), (nullOk), message, \
                            return FAIL_STATUS)

// Macros used in bindings - they cause the function to throw and return void

#define THROW_BODY(env, returnValue)                               \
  if (strcmp(__resultingStatus.c_str(), PENDING_EXCEPTION)) {      \
    napi_throw_error((env), "iotivity-node", FAIL_STATUS.c_str()); \
  }                                                                \
  return returnValue;

#define NAPI_CALL_THROW(env, theCall) \
  NAPI_CALL((env), theCall, THROW_BODY((env), 0))

#define HELPER_CALL_THROW(env, theCall) \
  HELPER_CALL(theCall, THROW_BODY((env), 0))

#define J2C_VALIDATE_VALUE_TYPE_THROW(env, value, typecheck, message) \
  J2C_VALIDATE_VALUE_TYPE((env), (value), typecheck, message,         \
                          THROW_BODY((env), 0))

#define J2C_DECLARE_ARGUMENTS(env, info, count)                                \
  do {                                                                         \
    size_t length;                                                             \
    NAPI_CALL_THROW((env), napi_get_cb_info((env), (info), &length, 0, 0, 0)); \
    JS_ASSERT((length == (count)), "expected " #count " arguments",            \
              THROW_BODY((env), 0));                                           \
  } while (0);                                                                 \
  napi_value arguments[count];                                                 \
  do {                                                                         \
    size_t length = (count);                                                   \
    NAPI_CALL_THROW(                                                           \
        (env), napi_get_cb_info((env), (info), &(length), arguments, 0, 0));   \
  } while (0)

#define J2C_DECLARE_VALUE_JS_THROW(cType, variableName, env, source, jsType,  \
                                   message, getterSuffix, jsParameterType)    \
  J2C_DECLARE_VALUE_JS(cType, variableName, (env), (source), jsType, message, \
                       getterSuffix, jsParameterType, THROW_BODY((env), 0))

#define J2C_GET_STRING_TRACKED_JS_THROW(varName, env, source, nullOk, message) \
  J2C_GET_STRING_TRACKED_JS(varName, (env), (source), (nullOk), message,       \
                            THROW_BODY((env), 0))

#define J2C_VALIDATE_IS_ARRAY_THROW(env, theValue, nullOk, message) \
  J2C_VALIDATE_IS_ARRAY((env), (theValue), (nullOk), message,       \
                        THROW_BODY((env), 0))

#define J2C_DECLARE_PROPERTY_JS_THROW(varName, env, source, name) \
  J2C_DECLARE_PROPERTY_JS(varName, env, source, name, THROW_BODY((env), 0))

#define C2J_SET_PROPERTY_THROW(env, destination, name, type, ...)              \
  do {                                                                         \
    napi_value __jsValue;                                                      \
    NAPI_CALL_THROW((env),                                                     \
                    napi_create_##type((env), __VA_ARGS__, &__jsValue));       \
    NAPI_CALL_THROW((env), napi_set_named_property((env), (destination), name, \
                                                   __jsValue));                \
  } while (0)

#define C2J_SET_RETURN_VALUE(env, info, type, ...)                             \
  do {                                                                         \
    napi_value jsResult;                                                       \
    NAPI_CALL_THROW((env), napi_create_##type((env), __VA_ARGS__, &jsResult)); \
    return jsResult;                                                           \
  } while (0)

class NapiHandleScope {
  napi_handle_scope scope;

 public:
  napi_env env;
  std::string open();
  NapiHandleScope(napi_env _env);
  ~NapiHandleScope();
};

#define DECLARE_HANDLE_SCOPE(varName, env, ...) \
  NapiHandleScope varName((env));               \
  HELPER_CALL(varName.open(), THROW_BODY((env), __VA_ARGS__));

std::string js_StringArray(napi_env env, OCStringLL *source, napi_value *ar);

std::string js_ArrayFromBytes(napi_env env, unsigned char *bytes,
                              uint32_t length, napi_value *array);
std::string c_ArrayFromBytes(napi_env env, napi_value array,
                             unsigned char *bytes, uint32_t length);

#endif /* __IOTIVITY_NODE_FUNCTIONS_INTERNAL_H__ */
