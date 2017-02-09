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
#include <string>

extern "C" {
#include <ocstack.h>
}

#define SOURCE_LOCATION                                        \
  (std::string(" at ") + std::string(__func__) +               \
   std::string(" (" __FILE__ ":") + std::to_string(__LINE__) + \
   std::string(")\n"))

#define LOCAL_MESSAGE(message) \
  (std::string(message) + std::string("\n") + SOURCE_LOCATION)

#define RESULT_CALL(theCall, ...)     \
  do {                                \
    std::string __resultingStatus;    \
    theCall;                          \
    if (!__resultingStatus.empty()) { \
      __VA_ARGS__;                    \
    }                                 \
  } while (0)

#define NAPI_CALL(theCall, ...)                                         \
  RESULT_CALL(                                                          \
      do {                                                              \
        napi_status __napiStatus = theCall;                             \
        if (!(__napiStatus == napi_ok ||                                \
              __napiStatus == napi_pending_exception)) {                \
          __resultingStatus =                                           \
              LOCAL_MESSAGE(napi_get_last_error_info()->error_message); \
        }                                                               \
      } while (0),                                                      \
      __VA_ARGS__);

#define HELPER_CALL(theCall, ...) \
  RESULT_CALL(__resultingStatus = theCall, __VA_ARGS__)

#define SET_PROPERTY_JS(env, destination, name, jsValue, ...)               \
  do {                                                                      \
    napi_propertyname __jsName;                                             \
    NAPI_CALL(napi_property_name((env), name, &__jsName), __VA_ARGS__);     \
    NAPI_CALL(napi_set_property((env), (destination), __jsName, (jsValue)), \
              __VA_ARGS__);                                                 \
  } while (0)

#define VALIDATE_VALUE_TYPE(env, value, typecheck, nullOk, ...)               \
  do {                                                                        \
    napi_valuetype theType;                                                   \
    NAPI_CALL(napi_get_type_of_value((env), (value), &theType), __VA_ARGS__); \
    if (theType != (typecheck) || (nullOk && theType == napi_null)) {         \
      __VA_ARGS__;                                                            \
    }                                                                         \
  } while (0)

// Macros used in helpers - they cause the function to return a std::string

#define NAPI_CALL_RETURN(theCall) NAPI_CALL(theCall, return __resultingStatus)

#define HELPER_CALL_RETURN(theCall) \
  HELPER_CALL(theCall, return (__resultingStatus + SOURCE_LOCATION))

#define SET_PROPERTY_RETURN(env, destination, name, type, ...)            \
  do {                                                                    \
    napi_value __jsValue;                                                 \
    NAPI_CALL_RETURN(napi_create_##type((env), __VA_ARGS__, &__jsValue)); \
    SET_PROPERTY_JS((env), (destination), name, __jsValue,                \
                    return __resultingStatus);                            \
  } while (0)

// Macros used in bindings - they cause the function to throw and return void

#define DECLARE_ARGUMENTS(env, info, count)                                  \
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

#define SET_PROPERTY_THROW(env, destination, name, type, ...)            \
  do {                                                                   \
    napi_value __jsValue;                                                \
    NAPI_CALL_THROW((env),                                               \
                    napi_create_##type((env), __VA_ARGS__, &__jsValue)); \
    SET_PROPERTY_JS((env), (destination), name, __jsValue, do {          \
      napi_throw_error((env), __resultingStatus.c_str());                \
      return;                                                            \
    } while (0));                                                        \
  } while (0)

#define NAPI_CALL_THROW(env, theCall)                   \
  NAPI_CALL(theCall, do {                               \
    napi_throw_error((env), __resultingStatus.c_str()); \
    return;                                             \
  } while (0))

#define HELPER_CALL_THROW(env, theCall)                 \
  HELPER_CALL(theCall, do {                             \
    napi_throw_error((env), __resultingStatus.c_str()); \
    return;                                             \
  } while (0))

#define VALIDATE_VALUE_TYPE_THROW(env, value, typecheck, nullOk, message) \
  VALIDATE_VALUE_TYPE((env), (value), (typecheck), nullOk, do {           \
    napi_throw_error((env), LOCAL_MESSAGE(std::string(message) +          \
                                          " is not of type " #typecheck)  \
                                .c_str());                                \
    return;                                                               \
  } while (0))

#define SET_RETURN_VALUE(env, info, type, ...)                                \
  do {                                                                        \
    napi_value __jsResult;                                                    \
    NAPI_CALL_THROW((env),                                                    \
                    napi_create_##type((env), __VA_ARGS__, &__jsResult));     \
    NAPI_CALL_THROW((env), napi_set_return_value((env), (info), __jsResult)); \
  } while (0)

#endif /* __IOTIVITY_NODE_FUNCTIONS_INTERNAL_H__ */
