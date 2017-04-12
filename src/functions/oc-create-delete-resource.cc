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

#include <map>

#include "../common.h"
#include "../structures/handles.h"
#include "entity-handler.h"
#include "oc-server-resource-utils.h"

extern "C" {
#include <ocstack.h>
}

static OCEntityHandlerResult defaultEntityHandler(
    OCEntityHandlerFlag flag, OCEntityHandlerRequest *request, void *data) {
  JSOCResourceHandle *cData = (JSOCResourceHandle *)data;
  EH_BODY(cData->env, flag, request, nullptr, cData->callback);
}

napi_value bind_OCCreateResource(napi_env env, napi_callback_info info) {
  J2C_DECLARE_ARGUMENTS(env, info, 6);

  J2C_VALIDATE_VALUE_TYPE_THROW(env, arguments[0], napi_object, "handle");
  J2C_GET_STRING_TRACKED_JS_THROW(type, env, arguments[1], false, "type");
  J2C_GET_STRING_TRACKED_JS_THROW(iface, env, arguments[2], false, "interface");
  J2C_GET_STRING_TRACKED_JS_THROW(uri, env, arguments[3], false, "uri");
  J2C_VALIDATE_VALUE_TYPE_THROW(env, arguments[4], napi_function, "callback");
  J2C_DECLARE_VALUE_JS_THROW(OCResourceProperty, properties, env, arguments[5],
                             napi_number, "properties", uint32, uint32_t);

  JSOCResourceHandle *cData;
  napi_value jsHandle;
  HELPER_CALL_THROW(env, JSOCResourceHandle::New(env, &jsHandle, &cData));

  OCStackResult result =
      OCCreateResource(&(cData->data), type, iface, uri, defaultEntityHandler,
                       cData, properties);

  if (result == OC_STACK_OK) {
    HELPER_CALL_THROW(env, cData->Init(env, arguments[4], jsHandle));
    C2J_SET_PROPERTY_JS_THROW(env, arguments[0], "handle", jsHandle);
  }
  C2J_SET_RETURN_VALUE(env, info, number, ((double)result));
}

napi_value bind_OCDeleteResource(napi_env env, napi_callback_info info) {
  FIRST_ARGUMENT_IS_HANDLE(1);

  OCStackResult result = OCDeleteResource(cData->data);
  if (result == OC_STACK_OK) {
    HELPER_CALL_THROW(env, JSOCResourceHandle::Destroy(env, cData));
  }

  C2J_SET_RETURN_VALUE(env, info, number, ((double)result));
}

// This is not really a binding since it only replaces the JS entity handler
napi_value bind_OCBindResourceHandler(napi_env env, napi_callback_info info) {
  FIRST_ARGUMENT_IS_HANDLE(2);
  J2C_VALIDATE_VALUE_TYPE_THROW(env, arguments[1], napi_function,
                                "entity handler");

  NAPI_CALL_THROW(env, napi_delete_reference(env, cData->callback));
  NAPI_CALL_THROW(
      env, napi_create_reference(env, arguments[1], 1, &(cData->callback)));

  C2J_SET_RETURN_VALUE(env, info, number, ((double)OC_STACK_OK));
}

#define BIND_STRING_TO_RESOURCE(api, message)                              \
  FIRST_ARGUMENT_IS_HANDLE(2);                                             \
  J2C_GET_STRING_TRACKED_JS_THROW(item, env, arguments[1], true, message); \
  C2J_SET_RETURN_VALUE(env, info, number, (double)api(cData->data, item));

napi_value bind_OCBindResourceTypeToResource(napi_env env,
                                             napi_callback_info info) {
  BIND_STRING_TO_RESOURCE(OCBindResourceTypeToResource, "resource type");
}

napi_value bind_OCBindResourceInterfaceToResource(napi_env env,
                                                  napi_callback_info info) {
  BIND_STRING_TO_RESOURCE(OCBindResourceInterfaceToResource, "interface");
}

// This is not actually a binding. We get the resource handler from the
// JS handle.
napi_value bind_OCGetResourceHandler(napi_env env, napi_callback_info info) {
  FIRST_ARGUMENT_IS_HANDLE(1);
  napi_value jsCB;
  NAPI_CALL_THROW(env, napi_get_reference_value(env, cData->callback, &jsCB));
  return jsCB;
}

#define BIND_UNBIND_RESOURCE(api)                                    \
  FIRST_ARGUMENT_IS_HANDLE(2);                                       \
  DECLARE_HANDLE_DATA(childData, env, arguments[1], "child handle"); \
  C2J_SET_RETURN_VALUE(env, info, number,                            \
                       (double)api(cData->data, childData->data));

napi_value bind_OCBindResource(napi_env env, napi_callback_info info) {
  BIND_UNBIND_RESOURCE(OCBindResource);
}

napi_value bind_OCUnBindResource(napi_env env, napi_callback_info info) {
  BIND_UNBIND_RESOURCE(OCUnBindResource);
}
