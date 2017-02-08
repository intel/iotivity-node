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

#define NAPI_CALL(env, theCall)                                        \
  do {                                                                 \
    napi_status status = theCall;                                      \
    if (!(status == napi_ok || status == napi_pending_exception)) {    \
      return LOCAL_MESSAGE(napi_get_last_error_info()->error_message); \
    }                                                                  \
  } while (0)

#define HELPER_CALL(theCall)             \
  do {                                   \
    std::string result = theCall;        \
    if (!result.empty()) {               \
      return (result + SOURCE_LOCATION); \
    }                                    \
  } while (0)

#define BINDING_HELPER_CALL(env, theCall)      \
  do {                                         \
    std::string result = theCall;              \
    if (!result.empty()) {                     \
      napi_throw_error((env), result.c_str()); \
      return;                                  \
    }                                          \
  } while (0)

#define BINDING_NAPI_CALL(env, theCall)                                      \
  do {                                                                       \
    napi_status status = theCall;                                            \
    if (!(status == napi_ok || status == napi_pending_exception)) {          \
      napi_throw_error(                                                      \
          (env),                                                             \
          LOCAL_MESSAGE(napi_get_last_error_info()->error_message).c_str()); \
      return;                                                                \
    }                                                                        \
  } while (0)

#define SET_PROPERTY_JS(env, destination, name, jsValue)                 \
  do {                                                                   \
    napi_propertyname jsName;                                            \
    NAPI_CALL((env), napi_property_name((env), name, &jsName));          \
    NAPI_CALL((env),                                                     \
              napi_set_property((env), (destination), jsName, jsValue)); \
  } while (0)

#define SET_PROPERTY(env, destination, name, type, ...)                 \
  do {                                                                  \
    napi_value jsValue;                                                 \
    NAPI_CALL((env), napi_create_##type((env), __VA_ARGS__, &jsValue)); \
    SET_PROPERTY_JS((env), (destination), name, jsValue);               \
  } while (0)

#define SET_CONSTANT(env, destination, name, type) \
  SET_PROPERTY((env), (destination), #name, type, name)

#endif /* __IOTIVITY_NODE_FUNCTIONS_INTERNAL_H__ */
