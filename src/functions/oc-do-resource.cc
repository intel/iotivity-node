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

#include <vector>

#include "../common.h"
#include "../structures/handles.h"
#include "../structures/oc-client-response.h"
#include "../structures/oc-dev-addr.h"
#include "../structures/oc-payload.h"

extern "C" {
#include <ocpayload.h>
#include <ocstack.h>
#include <rd_client.h>
#include <stdint.h>
}

static void deleteCallback(void *data) {
  JSOCDoHandle *cData = (JSOCDoHandle *)data;
  NapiHandleScope scope(cData->env);
  HELPER_CALL(scope.open(), THROW_BODY(scope.env, ));
  HELPER_CALL(JSOCDoHandle::Destroy(scope.env, cData), THROW_BODY(scope.env, ));
}

static OCStackApplicationResult defaultOCClientResponseHandler(
    void *context, OCDoHandle, OCClientResponse *clientResponse) {
  JSOCDoHandle *cData = (JSOCDoHandle *)context;
  OCStackApplicationResult failReturn = OC_STACK_KEEP_TRANSACTION;
  DECLARE_HANDLE_SCOPE(scope, cData->env, failReturn);

  napi_value jsContext, jsCallback, jsReturnValue;
  NAPI_CALL(cData->env, napi_get_null(scope.env, &jsContext),
            THROW_BODY(scope.env, failReturn));
  NAPI_CALL(cData->env,
            napi_get_reference_value(scope.env, cData->callback, &jsCallback),
            THROW_BODY(scope.env, failReturn));

  napi_value arguments[2];
  NAPI_CALL(cData->env,
            napi_get_reference_value(scope.env, cData->self, &arguments[0]),
            THROW_BODY(scope.env, failReturn));
  HELPER_CALL(js_OCClientResponse(scope.env, clientResponse, &arguments[1]),
              THROW_BODY(scope.env, failReturn));
  NAPI_CALL(cData->env, napi_call_function(scope.env, jsContext, jsCallback, 2,
                                           arguments, &jsReturnValue),
            THROW_BODY(scope.env, failReturn));

  J2C_DECLARE_VALUE_JS(OCStackApplicationResult, cResult, scope.env,
                       jsReturnValue, napi_number,
                       "OCDoResource response callback return value", uint32,
                       uint32_t, THROW_BODY(scope.env, failReturn));

  return cResult;
}

#define OC_DO_CALL(env, info, arguments, conTypeIndex, qosIndex, cbIndex,      \
                   doCall)                                                     \
  do {                                                                         \
    J2C_VALIDATE_VALUE_TYPE_THROW(env, (arguments)[0], napi_object, "handle"); \
    J2C_VALIDATE_VALUE_TYPE_THROW(env, (arguments)[(cbIndex)], napi_function,  \
                                  "callback");                                 \
    J2C_DECLARE_VALUE_JS_THROW(OCConnectivityType, connectivityType, env,      \
                               arguments[(conTypeIndex)], napi_number,         \
                               "connectivityType", uint32, uint32_t);          \
    J2C_DECLARE_VALUE_JS_THROW(OCQualityOfService, qos, env,                   \
                               arguments[(qosIndex)], napi_number, "qos",      \
                               uint32, uint32_t);                              \
                                                                               \
    JSOCDoHandle *cData;                                                       \
    napi_value jsHandle;                                                       \
    HELPER_CALL_THROW(env, JSOCDoHandle::New(env, &jsHandle, &cData));         \
                                                                               \
    OCCallbackData cbData;                                                     \
    cbData.context = cData;                                                    \
    cbData.cb = defaultOCClientResponseHandler;                                \
    cbData.cd = deleteCallback;                                                \
                                                                               \
    OCStackResult result = doCall;                                             \
                                                                               \
    if (result == OC_STACK_OK) {                                               \
      HELPER_CALL_THROW(env,                                                   \
                        cData->Init(env, (arguments)[(cbIndex)], jsHandle));   \
      NAPI_CALL_THROW(env, napi_set_named_property(env, arguments[0],          \
                                                   "handle", jsHandle));       \
    }                                                                          \
    C2J_SET_RETURN_VALUE(env, info, double, ((double)result));                 \
  } while (0)

napi_value bind_OCDoResource(napi_env env, napi_callback_info info) {
  J2C_DECLARE_ARGUMENTS(env, info, 9);
  J2C_DECLARE_VALUE_JS_THROW(OCMethod, method, env, arguments[1], napi_number,
                             "method", uint32, uint32_t);
  J2C_GET_STRING_TRACKED_JS_THROW(requestUri, env, arguments[2], false,
                                  "requestUri");

  OCDevAddr localAddr;
  OCDevAddr *destination = nullptr;
  DECLARE_VALUE_TYPE(addrType, env, arguments[3], THROW_BODY(env, 0));
  if (addrType != napi_null) {
    J2C_VALIDATE_VALUE_TYPE_THROW(env, arguments[3], napi_object,
                                  "destination");
    HELPER_CALL_THROW(env, c_OCDevAddr(env, arguments[3], &localAddr));
    destination = &localAddr;
  }

  OCPayload *payload = nullptr;
  HELPER_CALL_THROW(env, c_OCPayload(env, arguments[4], &payload));

  J2C_VALIDATE_IS_ARRAY_THROW(env, arguments[8], true, "options");

  OC_DO_CALL(env, info, arguments, 5, 6, 7,
             OCDoResource(&(cData->data), method, requestUri, destination,
                          payload, connectivityType, qos, &cbData, nullptr, 0));
}

napi_value bind_OCRDDiscover(napi_env env, napi_callback_info info) {
  J2C_DECLARE_ARGUMENTS(env, info, 4);
  OC_DO_CALL(env, info, arguments, 1, 3, 2,
             OCRDDiscover(&(cData->data), connectivityType, &cbData, qos));
}

static std::string c_OCResourceArray(
    napi_env env, napi_value source,
    std::vector<OCResourceHandle> &destination) {
  uint32_t length, index;
  napi_handle_scope scope;
  napi_value jsHandle;
  JSOCResourceHandle *cResData;

  J2C_VALIDATE_IS_ARRAY_RETURN(env, source, false, "resource array");

  NAPI_CALL_RETURN(env, napi_get_array_length(env, source, &length));
  if (length > UINT8_MAX) {
    return LOCAL_MESSAGE("resource array length exceeds UINT8_MAX");
  }

  destination.resize(length);
  for (index = 0; index < length; index++) {
    NAPI_CALL_RETURN(env, napi_open_handle_scope(env, &scope));
    NAPI_CALL_RETURN(env, napi_get_element(env, source, index, &jsHandle));
    HELPER_CALL_RETURN(JSOCResourceHandle::Get(env, jsHandle, &cResData));
    destination[index] = cResData->data;
    NAPI_CALL_RETURN(env, napi_close_handle_scope(env, scope));
  }

  return std::string();
}

#define OC_DO_RESOURCE_LIST_CALL(env, info, arguments, hostIndex,           \
                                 conTypeIndex, resourceListIndex, cbIndex,  \
                                 qosIndex, doCall)                          \
  do {                                                                      \
    J2C_GET_STRING_TRACKED_JS_THROW(host, env, (arguments)[(hostIndex)],    \
                                    false, "host");                         \
    std::vector<OCResourceHandle> handles;                                  \
    HELPER_CALL_THROW(                                                      \
        env,                                                                \
        c_OCResourceArray(env, (arguments)[(resourceListIndex)], handles)); \
                                                                            \
    OC_DO_CALL((env), (info), (arguments), (conTypeIndex), (qosIndex),      \
               (cbIndex), doCall);                                          \
  } while (0)

napi_value bind_OCRDPublish(napi_env env, napi_callback_info info) {
  J2C_DECLARE_ARGUMENTS(env, info, 7);
  J2C_DECLARE_VALUE_JS_THROW(uint32_t, ttl, env, arguments[4], napi_number,
                             "ttl", uint32, uint32_t);
  OC_DO_RESOURCE_LIST_CALL(
      env, info, arguments, 1, 2, 3, 5, 6,
      OCRDPublish(&(cData->data), host, connectivityType, handles.data(),
                  handles.size(), ttl, &cbData, qos));
}

napi_value bind_OCRDPublishWithDeviceId(napi_env env, napi_callback_info info) {
  J2C_DECLARE_ARGUMENTS(env, info, 8);
  J2C_GET_STRING_TRACKED_JS_THROW(id, env, arguments[2], false, "id");
  J2C_DECLARE_VALUE_JS_THROW(uint32_t, ttl, env, arguments[5], napi_number,
                             "ttl", uint32, uint32_t);
  OC_DO_RESOURCE_LIST_CALL(
      env, info, arguments, 1, 3, 4, 6, 7,
      OCRDPublishWithDeviceId(&(cData->data), host, (const unsigned char *)id,
                              connectivityType, handles.data(), handles.size(),
                              ttl, &cbData, qos));
}

napi_value bind_OCRDDelete(napi_env env, napi_callback_info info) {
  J2C_DECLARE_ARGUMENTS(env, info, 6);
  OC_DO_RESOURCE_LIST_CALL(
      env, info, arguments, 1, 2, 3, 4, 5,
      OCRDDelete(&(cData->data), host, connectivityType, handles.data(),
                 handles.size(), &cbData, qos));
}

napi_value bind_OCCancel(napi_env env, napi_callback_info info) {
  J2C_DECLARE_ARGUMENTS(env, info, 3);
  J2C_VALIDATE_VALUE_TYPE_THROW(env, arguments[1], napi_number, "qos");
  J2C_VALIDATE_IS_ARRAY_THROW(env, arguments[2], true, "header options");

  JSOCDoHandle *cData;
  J2C_VALIDATE_VALUE_TYPE_THROW(env, arguments[0], napi_object, "handle");
  HELPER_CALL_THROW(env, JSOCDoHandle::Get(env, arguments[0], &cData));
  JS_ASSERT(cData, "Native handle is invalid", THROW_BODY(env, 0));

  J2C_DECLARE_VALUE_JS_THROW(OCQualityOfService, qos, env, arguments[1],
                             napi_number, "qos", uint32, uint32_t);

  // header options ignored
  C2J_SET_RETURN_VALUE(env, info, double,
                       ((double)OCCancel(cData->data, qos, nullptr, 0)));
}
