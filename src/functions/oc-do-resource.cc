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
#include "../structures/handles.h"
#include "../structures/oc-client-response.h"
#include "../structures/oc-dev-addr.h"
#include "../structures/oc-payload.h"
extern "C" {
#include <ocpayload.h>
#include <ocstack.h>
}

static void deleteCallback(void *data) {
  NapiHandleScope scope;
  HELPER_CALL_THROW(scope.env,
                    JSOCDoHandle::Destroy(scope.env, (JSOCDoHandle *)data));
}

static OCStackApplicationResult defaultOCClientResponseHandler(
    void *context, OCDoHandle, OCClientResponse *clientResponse) {
  NapiHandleScope scope;
  OCStackApplicationResult failReturn = OC_STACK_KEEP_TRANSACTION;

  JSOCDoHandle *cData = (JSOCDoHandle *)context;
  napi_value jsContext, jsCallback, jsReturnValue;
  NAPI_CALL(napi_get_null(scope.env, &jsContext),
            THROW_BODY(scope.env, failReturn));
  NAPI_CALL(napi_get_reference_value(scope.env, cData->callback, &jsCallback),
            THROW_BODY(scope.env, failReturn));

  napi_value arguments[2];
  NAPI_CALL(napi_get_reference_value(scope.env, cData->self, &arguments[0]),
            THROW_BODY(scope.env, failReturn));
  HELPER_CALL(js_OCClientResponse(scope.env, clientResponse, &arguments[1]),
              THROW_BODY(scope.env, failReturn));
  NAPI_CALL(napi_call_function(scope.env, jsContext, jsCallback, 2, arguments,
                               &jsReturnValue),
            THROW_BODY(scope.env, failReturn));

  J2C_DECLARE_VALUE_JS(OCStackApplicationResult, cResult, scope.env,
                       jsReturnValue, napi_number,
                       "OCDoResource response callback return value", uint32,
                       uint32_t, THROW_BODY(scope.env, failReturn));

  return cResult;
}

void bind_OCDoResource(napi_env env, napi_callback_info info) {
  J2C_DECLARE_ARGUMENTS(env, info, 9);
  J2C_VALIDATE_VALUE_TYPE_THROW(env, arguments[0], napi_object, "handle");
  J2C_DECLARE_VALUE_JS_THROW(OCMethod, method, env, arguments[1], napi_number,
                             "method", uint32, uint32_t);
  J2C_GET_STRING_TRACKED_JS_THROW(requestUri, env, arguments[2], false,
                                  "requestUri");

  OCDevAddr localAddr;
  OCDevAddr *destination = nullptr;
  DECLARE_VALUE_TYPE(addrType, env, arguments[3], THROW_BODY(env, ));
  if (addrType != napi_null) {
    J2C_VALIDATE_VALUE_TYPE_THROW(env, arguments[3], napi_object,
                                  "destination");
    HELPER_CALL_THROW(env, c_OCDevAddr(env, arguments[3], &localAddr));
    destination = &localAddr;
  }

  OCPayload *payload = nullptr;
  HELPER_CALL_THROW(env, c_OCPayload(env, arguments[4], &payload));

  J2C_DECLARE_VALUE_JS_THROW(OCConnectivityType, connectivityType, env,
                             arguments[5], napi_number, "connectivityType",
                             uint32, uint32_t);
  J2C_DECLARE_VALUE_JS_THROW(OCQualityOfService, qos, env, arguments[6],
                             napi_number, "qos", uint32, uint32_t);

  J2C_VALIDATE_VALUE_TYPE_THROW(env, arguments[7], napi_function, "callback");

  J2C_VALIDATE_IS_ARRAY_THROW(env, arguments[8], true, "options");

  JSOCDoHandle *cData;
  napi_value jsHandle;
  HELPER_CALL_THROW(env, JSOCDoHandle::New(env, &jsHandle, &cData));

  OCCallbackData cbData;
  cbData.context = cData;
  cbData.cb = defaultOCClientResponseHandler;
  cbData.cd = deleteCallback;
  OCStackResult result =
      OCDoResource(&(cData->data), method, requestUri, destination, payload,
                   connectivityType, qos, &cbData, nullptr, 0);

  if (result == OC_STACK_OK) {
    HELPER_CALL_THROW(env, cData->Init(env, arguments[7], jsHandle));
    C2J_SET_PROPERTY_JS_THROW(env, arguments[0], "handle", jsHandle);
  }
  C2J_SET_RETURN_VALUE(env, info, number, ((double)result));
}

void bind_OCCancel(napi_env env, napi_callback_info info) {
  J2C_DECLARE_ARGUMENTS(env, info, 3);
  J2C_VALIDATE_VALUE_TYPE_THROW(env, arguments[1], napi_number, "qos");
  J2C_VALIDATE_IS_ARRAY_THROW(env, arguments[2], true, "header options");

  JSOCDoHandle *cData;
  J2C_VALIDATE_VALUE_TYPE_THROW(env, arguments[0], napi_object, "handle");
  HELPER_CALL_THROW(env, JSOCDoHandle::Get(env, arguments[0], &cData));
  JS_ASSERT(cData, "Native handle is invalid", THROW_BODY(env, ));

  J2C_DECLARE_VALUE_JS_THROW(OCQualityOfService, qos, env, arguments[1],
                             napi_number, "qos", uint32, uint32_t);

  // header options ignored
  C2J_SET_RETURN_VALUE(env, info, number,
                       ((double)OCCancel(cData->data, qos, nullptr, 0)));
}
