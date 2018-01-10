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
#include "../structures/oc-entity-handler-response.h"
#include "../structures/oc-rep-payload/to-c.h"
#include "oc-server-resource-utils.h"

extern "C" {
#include <ocpayload.h>
#include <ocstack.h>
}

napi_value bind_OCDoResponse(napi_env env, napi_callback_info info) {
  J2C_DECLARE_ARGUMENTS(env, info, 1);
  J2C_VALIDATE_VALUE_TYPE_THROW(env, arguments[0], napi_object, "response");
  OCEntityHandlerResponse resp;
  HELPER_CALL_THROW(env, c_OCEntityHandlerResponse(env, arguments[0], &resp));
  std::unique_ptr<OCPayload, void (*)(OCPayload *)> payloadTracker(
      resp.payload, OCPayloadDestroy);
  OCStackResult result = OCDoResponse(&resp);
  C2J_SET_RETURN_VALUE(env, info, double, ((double)result));
}

napi_value bind_OCNotifyListOfObservers(napi_env env, napi_callback_info info) {
  // handle
  FIRST_ARGUMENT_IS_HANDLE(4);

  // obsIdList and numberOfIds
  J2C_VALIDATE_IS_ARRAY_THROW(env, arguments[1], false, "observation ID list");
  uint32_t obsCount, index;
  napi_value jsObsId;
  NAPI_CALL_THROW(env, napi_get_array_length(env, arguments[1], &obsCount));
  std::unique_ptr<OCObservationId> observers(new OCObservationId[obsCount]());
  for (index = 0; index < obsCount; index++) {
    NAPI_CALL_THROW(env, napi_get_element(env, arguments[1], index, &jsObsId));
    J2C_ASSIGN_VALUE_JS(
        OCObservationId, observers.get()[index], env, jsObsId, napi_number,
        std::string("observation id[") + std::to_string(index) + "]", uint32,
        uint32_t, THROW_BODY(env, 0));
  }

  // payload
  OCRepPayload *payload = nullptr;
  std::unique_ptr<OCRepPayload, void (*)(OCRepPayload *)> payloadTracker(
      nullptr, OCRepPayloadDestroy);
  DECLARE_VALUE_TYPE(payloadType, env, arguments[2], THROW_BODY(env, 0));
  if (!(payloadType == napi_null || payloadType == napi_undefined)) {
    HELPER_CALL_THROW(env, c_OCRepPayload(env, arguments[2], &payload));
    payloadTracker.reset(payload);
  }

  // qos
  J2C_DECLARE_VALUE_JS_THROW(OCQualityOfService, qos, env, arguments[3],
                             napi_number, "qos", uint32, uint32_t);

  C2J_SET_RETURN_VALUE(
      env, info, double,
      ((double)OCNotifyListOfObservers(cData->data, observers.get(),
                                       (uint8_t)obsCount, payload, qos)));
}
