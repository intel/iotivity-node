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

#include <node_api_helpers.h>
#include <stdlib.h>
#include <string.h>
#include <memory>
#include <string>

#define SOURCE_LOCATION                                        \
  (std::string("    at ") + std::string(__func__) +            \
   std::string(" (" __FILE__ ":") + std::to_string(__LINE__) + \
   std::string(")\n"))

#define LOCAL_MESSAGE(message) \
  (std::string("") + message + "\n" + SOURCE_LOCATION)

#define RESULT_CALL(theCall, ...)     \
  do {                                \
    std::string __resultingStatus;    \
    theCall;                          \
    if (!__resultingStatus.empty()) { \
      __VA_ARGS__;                    \
    }                                 \
  } while (0)

#define NAPI_CALL(theCall, ...)                                       \
  RESULT_CALL(                                                        \
      napi_status __napiStatus = theCall;                             \
      if (!(__napiStatus == napi_ok ||                                \
            __napiStatus == napi_pending_exception)) {                \
        __resultingStatus =                                           \
            (std::string(napi_get_last_error_info()->error_message) + \
             std::string("\n"));                                      \
      },                                                              \
      __VA_ARGS__)

#define HELPER_CALL(theCall, ...) \
  RESULT_CALL(__resultingStatus = theCall, __VA_ARGS__)

#define DECLARE_PROPERTY(varName, env, value, ...) \
  napi_propertyname varName;                       \
  NAPI_CALL(napi_property_name((env), value, &varName), __VA_ARGS__)

#define DECLARE_VALUE_TYPE(varName, env, value, ...) \
  napi_valuetype varName;                            \
  NAPI_CALL(napi_get_type_of_value((env), (value), &varName), __VA_ARGS__);

#define J2C_VALIDATE_VALUE_TYPE(env, value, typecheck, message, ...) \
  RESULT_CALL(                                                       \
      DECLARE_VALUE_TYPE(theType, (env), value, __VA_ARGS__);        \
      if (theType != (typecheck)) {                                  \
        __resultingStatus =                                          \
            std::string() + message + " is not a " #typecheck "\n";  \
      },                                                             \
      __VA_ARGS__)

#define J2C_GET_PROPERTY_JS(varName, env, source, name, ...)        \
  napi_value varName;                                               \
  do {                                                              \
    DECLARE_PROPERTY(jsName, (env), name, __VA_ARGS__);             \
    NAPI_CALL(napi_get_property((env), (source), jsName, &varName), \
              __VA_ARGS__);                                         \
  } while (0)

#define J2C_ASSIGN_VALUE_JS(cType, destination, env, source, jsType, message, \
                            getterSuffix, jsParameterType, ...)               \
  do {                                                                        \
    jsParameterType fromJSValue;                                              \
    J2C_VALIDATE_VALUE_TYPE((env), (source), (jsType), message, __VA_ARGS__); \
    NAPI_CALL(napi_get_value_##getterSuffix((env), (source), &fromJSValue),   \
              __VA_ARGS__);                                                   \
    (destination) = (cType)fromJSValue;                                       \
  } while (0)

#define J2C_GET_VALUE_JS(cType, variableName, env, source, jsType, message,  \
                         getterSuffix, jsParameterType, ...)                 \
  cType variableName;                                                        \
  J2C_ASSIGN_VALUE_JS(cType, variableName, (env), (source), jsType, message, \
                      getterSuffix, jsParameterType, __VA_ARGS__);

#define J2C_GET_STRING_JS(env, destination, source, nullOk, message, ...) \
  RESULT_CALL(                                                            \
      DECLARE_VALUE_TYPE(valueType, env, (source), __VA_ARGS__);          \
      if (nullOk && valueType == napi_null) {                             \
        (destination) = nullptr;                                          \
      } else if (valueType == napi_string) {                              \
        int cString__length;                                              \
        NAPI_CALL(napi_get_value_string_utf8_length((env), (source),      \
                                                    &cString__length),    \
                  __VA_ARGS__);                                           \
        std::unique_ptr<char> cString(new char[cString__length + 1]());   \
        if (cString.get()) {                                              \
          int bytesWritten;                                               \
          NAPI_CALL(                                                      \
              napi_get_value_string_utf8((env), (source), cString.get(),  \
                                         cString__length, &bytesWritten), \
              __VA_ARGS__);                                               \
          (destination) = cString.release();                              \
        } else {                                                          \
          __resultingStatus = std::string("") +                           \
                              "Failed to allocate memory for" + message + \
                              "\n";                                       \
        }                                                                 \
      } else {                                                            \
        __resultingStatus =                                               \
            (std::string("") + message + " is not a string\n");           \
      },                                                                  \
      __VA_ARGS__)

#define J2C_GET_STRING(env, destination, source, nullOk, name, ...)   \
  do {                                                                \
    (destination) = nullptr;                                          \
    J2C_GET_PROPERTY_JS(jsValue, (env), (source), name, __VA_ARGS__); \
    J2C_GET_STRING_JS((env), (destination), jsValue, (nullOk),        \
                      #destination "." name, __VA_ARGS__);            \
  } while (0)

#define C2J_SET_PROPERTY_JS(env, destination, name, jsValue, ...)         \
  do {                                                                    \
    DECLARE_PROPERTY(jsName, env, name, __VA_ARGS__);                     \
    NAPI_CALL(napi_set_property((env), (destination), jsName, (jsValue)), \
              __VA_ARGS__);                                               \
  } while (0)

#define J2C_VALIDATE_IS_ARRAY(env, theValue, nullOk, message, ...)            \
  RESULT_CALL(                                                                \
      DECLARE_VALUE_TYPE(jsType, (env), theValue, __VA_ARGS__);               \
      if (!((nullOk) && jsType == napi_null)) {                               \
        bool isArray;                                                         \
        NAPI_CALL(napi_is_array((env), (theValue), &isArray), __VA_ARGS__);   \
        if (!isArray) {                                                       \
          __resultingStatus = std::string() + message + " is not an array\n"; \
        }                                                                     \
      },                                                                      \
      __VA_ARGS__)

// Macros used in helpers - they cause the function to return a std::string

#define FAIL_STATUS_RETURN (__resultingStatus + SOURCE_LOCATION)

#define NAPI_CALL_RETURN(theCall) NAPI_CALL(theCall, return FAIL_STATUS_RETURN)

#define HELPER_CALL_RETURN(theCall) \
  HELPER_CALL(theCall, return FAIL_STATUS_RETURN)

#define J2C_GET_PROPERTY_JS_RETURN(varName, env, source, name) \
  J2C_GET_PROPERTY_JS(varName, env, source, name, return FAIL_STATUS_RETURN)

#define J2C_VALIDATE_VALUE_TYPE_RETURN(env, value, typecheck, message) \
  J2C_VALIDATE_VALUE_TYPE((env), (value), (typecheck),                 \
                          return FAIL_STATUS_RETURN)

#define J2C_VALIDATE_IS_ARRAY_RETURN(env, theValue, nullOk, message) \
  J2C_VALIDATE_IS_ARRAY((env), (theValue), (nullOk), message,        \
                        return FAIL_STATUS_RETURN)

#define J2C_GET_STRING_RETURN(env, destination, source, nullOk, name) \
  J2C_GET_STRING((env), (destination), (source), (nullOk), name,      \
                 return FAIL_STATUS_RETURN)

#define J2C_GET_STRING_JS_RETURN(env, destination, source, nullOk, message) \
  J2C_GET_STRING_JS((env), (destination), (source), (nullOk), message,      \
                    return FAIL_STATUS_RETURN)

#define J2C_GET_VALUE_JS_RETURN(cType, variableName, env, source, jsType, \
                                message, getterSuffix, jsParameterType)   \
  J2C_GET_VALUE_JS(cType, variableName, (env), (source), jsType, message, \
                   getterSuffix, jsParameterType, return FAIL_STATUS_RETURN)

#define J2C_ASSIGN_VALUE_JS_RETURN(cType, destination, env, source, jsType, \
                                   message, getterSuffix, jsParameterType)  \
  J2C_ASSIGN_VALUE_JS(cType, destination, env, source, jsType, message,     \
                      getterSuffix, jsParameterType,                        \
                      return FAIL_STATUS_RETURN)

#define J2C_ASSIGN_MEMBER_VALUE_RETURN(env, destination, source, cType, name, \
                                       jsType, message, getterSuffix,         \
                                       jsParameterType)                       \
  do {                                                                        \
    J2C_GET_PROPERTY_JS_RETURN(jsValue, (env), (source), #name);              \
    J2C_ASSIGN_VALUE_JS(cType, (destination)->name, (env), jsValue, jsType,   \
                        std::string() + message + "." + #name, getterSuffix,  \
                        jsParameterType, return FAIL_STATUS_RETURN);          \
  } while (0)

#define J2C_ASSIGN_MEMBER_STRING_RETURN(env, destination, source, name) \
  J2C_GET_STRING_RETURN((env), (destination)->name, source, true, #name)

#define C2J_SET_PROPERTY_JS_RETURN(env, destination, name, jsValue) \
  C2J_SET_PROPERTY_JS((env), (destination), name, (jsValue),        \
                      return FAIL_STATUS_RETURN)

#define C2J_SET_PROPERTY_CALL_RETURN(env, destination, name, call)   \
  do {                                                               \
    napi_value jsValue;                                              \
    call;                                                            \
    C2J_SET_PROPERTY_JS_RETURN((env), (destination), name, jsValue); \
  } while (0)

#define C2J_SET_PROPERTY_RETURN(env, destination, name, type, ...) \
  C2J_SET_PROPERTY_CALL_RETURN(                                    \
      (env), (destination), name,                                  \
      NAPI_CALL_RETURN(napi_create_##type((env), __VA_ARGS__, &jsValue)))

#define C2J_SET_NUMBER_MEMBER_RETURN(env, destination, source, name) \
  C2J_SET_PROPERTY_RETURN((env), (destination), #name, number, (source)->name)

#define C2J_SET_STRING_IF_NOT_NULL_RETURN(env, destination, source, name) \
  if ((source)->name) {                                                   \
    C2J_SET_PROPERTY_RETURN((env), (destination), #name, string_utf8,     \
                            (source)->name, strlen((source)->name));      \
  }

// Macros used in bindings - they cause the function to throw and return void

#define THROW_BODY(env, returnValue)                   \
  napi_throw_error((env), FAIL_STATUS_RETURN.c_str()); \
  return returnValue;

#define NAPI_CALL_THROW(env, theCall) NAPI_CALL(theCall, THROW_BODY((env), ))

#define HELPER_CALL_THROW(env, theCall) \
  HELPER_CALL(theCall, THROW_BODY((env), ))

#define J2C_VALIDATE_VALUE_TYPE_THROW(env, value, typecheck, message) \
  J2C_VALIDATE_VALUE_TYPE((env), (value), (typecheck), message,       \
                          THROW_BODY((env), ))

#define J2C_GET_ARGUMENTS(env, info, count)                                  \
  do {                                                                       \
    int length;                                                              \
    NAPI_CALL_THROW((env), napi_get_cb_args_length((env), (info), &length)); \
    if (length != (count)) {                                                 \
      napi_throw_error(                                                      \
          (env), LOCAL_MESSAGE("expected " #count " arguments").c_str());    \
      return;                                                                \
    }                                                                        \
  } while (0);                                                               \
  napi_value arguments[count];                                               \
  NAPI_CALL_THROW((env), napi_get_cb_args((env), (info), arguments, (count)));

#define J2C_GET_VALUE_JS_THROW(cType, variableName, env, source, jsType,  \
                               message, getterSuffix, jsParameterType)    \
  J2C_GET_VALUE_JS(cType, variableName, (env), (source), jsType, message, \
                   getterSuffix, jsParameterType, THROW_BODY((env), ))

#define J2C_GET_STRING_JS_THROW(env, destination, source, nullOk, message) \
  J2C_GET_STRING_JS((env), (destination), (source), (nullOk), message,     \
                    THROW_BODY((env), ))

#define J2C_GET_STRING_ARGUMENT_THROW(varName, env, source, nullOk, message) \
  std::unique_ptr<char> __##varName##__tracker;                              \
  char *varName = nullptr;                                                   \
  J2C_GET_STRING_JS_THROW((env), varName, (source), (nullOk), message);      \
  __##varName##__tracker.reset(varName);

#define J2C_VALIDATE_IS_ARRAY_THROW(env, theValue, nullOk, message) \
  J2C_VALIDATE_IS_ARRAY((env), (theValue), (nullOk), message,       \
                        THROW_BODY((env), ))

#define C2J_SET_PROPERTY_JS_THROW(env, destination, name, jsValue) \
  C2J_SET_PROPERTY_JS((env), (destination), (name), (jsValue),     \
                      THROW_BODY((env), ))

#define C2J_SET_PROPERTY_THROW(env, destination, name, type, ...)        \
  do {                                                                   \
    napi_value __jsValue;                                                \
    NAPI_CALL_THROW((env),                                               \
                    napi_create_##type((env), __VA_ARGS__, &__jsValue)); \
    C2J_SET_PROPERTY_JS_THROW((env), (destination), name, __jsValue);    \
  } while (0)

#define C2J_SET_RETURN_VALUE(env, info, type, ...)                            \
  do {                                                                        \
    napi_value __jsResult;                                                    \
    NAPI_CALL_THROW((env),                                                    \
                    napi_create_##type((env), __VA_ARGS__, &__jsResult));     \
    NAPI_CALL_THROW((env), napi_set_return_value((env), (info), __jsResult)); \
  } while (0)

#define ENTER_C_CALLBACK         \
  napi_handle_scope scope;       \
  napi_env env = napi_get_env(); \
  NAPI_CALL(napi_open_handle_scope(env, &scope), abort())

#define EXIT_C_CALLBACK(returnValue)               \
  NAPI_CALL(napi_close_handle_scope((env), scope), \
            THROW_BODY((env), returnValue));       \
  return returnValue

napi_env napi_get_env();
std::string js_ArrayFromBytes(napi_env env, unsigned char *bytes,
                              uint32_t length, napi_value *array);
std::string c_ArrayFromBytes(napi_env env, napi_value array,
                             unsigned char *bytes, uint32_t length);

#endif /* __IOTIVITY_NODE_FUNCTIONS_INTERNAL_H__ */
