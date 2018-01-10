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

#include "oc-server-resource-utils.h"
#include "../common.h"
#include "../structures/handles.h"

extern "C" {
#include <ocstack.h>
}

#define RETURN_NULL(env)                                     \
  do {                                                       \
    napi_value napiNull;                                     \
    NAPI_CALL_THROW((env), napi_get_null((env), &napiNull)); \
    return napiNull;                                         \
  } while (0)

napi_value bind_OCGetResourceUri(napi_env env, napi_callback_info info) {
  FIRST_ARGUMENT_IS_HANDLE(1);
  const char *uri = OCGetResourceUri(cData->data);
  if (uri) {
    C2J_SET_RETURN_VALUE(env, info, string_utf8, uri, strlen(uri));
  } else {
    RETURN_NULL(env);
  }
}

napi_value bind_OCGetResourceProperties(napi_env env, napi_callback_info info) {
  FIRST_ARGUMENT_IS_HANDLE(1);
  C2J_SET_RETURN_VALUE(env, info, double, OCGetResourceProperties(cData->data));
}

#define RESOURCE_BY_INDEX_ACCESSOR_BOILERPLATE                               \
  FIRST_ARGUMENT_IS_HANDLE(2);                                               \
  J2C_DECLARE_VALUE_JS_THROW(uint8_t, index, env, arguments[1], napi_number, \
                             "index", uint32, uint32_t)

#define RETURN_RESOURCE_HANDLE(env, handle)                                 \
  do {                                                                      \
    OCResourceHandle localHandle = (handle);                                \
    if (localHandle) {                                                      \
      napi_ref existingHandle = JSOCResourceHandle::handles[localHandle];   \
      JS_ASSERT(existingHandle, "JS handle not found for native handle",    \
                THROW_BODY((env), 0));                                      \
      napi_value jsHandle;                                                  \
      NAPI_CALL_THROW(                                                      \
          env, napi_get_reference_value((env), existingHandle, &jsHandle)); \
      return jsHandle;                                                      \
    } else {                                                                \
      return 0;                                                             \
    }                                                                       \
  } while (0)

napi_value bind_OCGetResourceHandleFromCollection(napi_env env,
                                                  napi_callback_info info) {
  RESOURCE_BY_INDEX_ACCESSOR_BOILERPLATE;
  RETURN_RESOURCE_HANDLE(env,
                         OCGetResourceHandleFromCollection(cData->data, index));
}

#define GET_STRING_FROM_RESOURCE_BY_INDEX(api)              \
  RESOURCE_BY_INDEX_ACCESSOR_BOILERPLATE;                   \
  const char *theString = api(cData->data, index);          \
  if (theString) {                                          \
    C2J_SET_RETURN_VALUE(env, info, string_utf8, theString, \
                         strlen(theString));                \
  } else {                                                  \
    RETURN_NULL(env);                                       \
  }

napi_value bind_OCGetResourceTypeName(napi_env env, napi_callback_info info) {
  GET_STRING_FROM_RESOURCE_BY_INDEX(OCGetResourceTypeName);
}

napi_value bind_OCGetResourceInterfaceName(napi_env env,
                                           napi_callback_info info) {
  GET_STRING_FROM_RESOURCE_BY_INDEX(OCGetResourceInterfaceName);
}

napi_value bind_OCGetResourceHandle(napi_env env, napi_callback_info info) {
  J2C_DECLARE_ARGUMENTS(env, info, 1);
  J2C_DECLARE_VALUE_JS_THROW(uint8_t, index, env, arguments[0], napi_number,
                             "index", uint32, uint32_t);
  RETURN_RESOURCE_HANDLE(env, OCGetResourceHandle(index));
}

#define GET_COUNT_FROM_RESOURCE(api)                                           \
  FIRST_ARGUMENT_IS_HANDLE(2)                                                  \
  J2C_VALIDATE_VALUE_TYPE_THROW(env, arguments[1], napi_object, "count");      \
  uint8_t count = 0;                                                           \
  OCStackResult result = api(cData->data, &count);                             \
  if (result == OC_STACK_OK) {                                                 \
    C2J_SET_PROPERTY_THROW(env, arguments[1], "count", double, (double)count); \
  }                                                                            \
  C2J_SET_RETURN_VALUE(env, info, double, ((double)result));

napi_value bind_OCGetNumberOfResourceInterfaces(napi_env env,
                                                napi_callback_info info) {
  GET_COUNT_FROM_RESOURCE(OCGetNumberOfResourceInterfaces);
}

napi_value bind_OCGetNumberOfResourceTypes(napi_env env,
                                           napi_callback_info info) {
  GET_COUNT_FROM_RESOURCE(OCGetNumberOfResourceTypes);
}
