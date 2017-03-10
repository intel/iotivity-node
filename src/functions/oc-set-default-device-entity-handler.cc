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

#include "../common.h"
#include "entity-handler.h"

extern "C" {
#include <ocstack.h>
#include <stdlib.h>
}

static napi_ref g_currentCallback = nullptr;
static napi_env g_currentEnv = nullptr;

static OCEntityHandlerResult defaultDeviceEntityHandler(
    OCEntityHandlerFlag flag, OCEntityHandlerRequest *request, char *uri,
    void *context) {
  EH_BODY(g_currentEnv, flag, request, uri, ((napi_ref)context));
}

void bind_OCSetDefaultDeviceEntityHandler(napi_env env,
                                          napi_callback_info info) {
  J2C_DECLARE_ARGUMENTS(env, info, 1);
  DECLARE_VALUE_TYPE(handlerType, env, arguments[0], THROW_BODY(env, ));

  OCDeviceEntityHandler newHandler = 0;
  napi_ref newCallback = 0, callbackToDelete = 0;
  if (handlerType == napi_function) {
    NAPI_CALL_THROW(env,
                    napi_create_reference(env, arguments[0], 1, &newCallback));
    newHandler = defaultDeviceEntityHandler;
  }
  OCStackResult result =
      OCSetDefaultDeviceEntityHandler(newHandler, (void *)newCallback);
  if (result == OC_STACK_OK) {
    callbackToDelete = g_currentCallback;
    g_currentCallback = newCallback;
	g_currentEnv = env;
  } else {
    callbackToDelete = newCallback;
  }
  if (callbackToDelete) {
    NAPI_CALL_THROW(env, napi_delete_reference(env, callbackToDelete));
  }
  C2J_SET_RETURN_VALUE(env, info, number, ((double)result));
}
