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

extern "C" {
#include <string.h>
}

#include "common.h"
#include "structures.h"
#include "structures/handles.h"
#include "structures/oc-dev-addr.h"
#include "structures/oc-payload.h"

std::string js_OCEntityHandlerRequest(napi_env env,
                                      OCEntityHandlerRequest *request,
                                      napi_value *result) {
  NAPI_CALL_RETURN(env, napi_create_object(env, result));

  // The resource may be null if the request refers to a non-existing resource
  // and is being passed to the default device entity handler
  if (request->resource) {
    napi_ref jsRef = JSOCResourceHandle::handles[request->resource];
    if (!jsRef) {
      return LOCAL_MESSAGE("resource handle not found");
    }
    C2J_SET_PROPERTY_CALL_RETURN(
        env, *result, "resource",
        NAPI_CALL_RETURN(env, napi_get_reference_value(env, jsRef, &jsValue)));
  }

  if (request->requestHandle) {
    napi_value jsHandle;
    JSOCRequestHandle *cData;
    HELPER_CALL_RETURN(JSOCRequestHandle::New(env, &jsHandle, &cData));
    cData->data = request->requestHandle;
    HELPER_CALL_RETURN(cData->Init(env, nullptr, jsHandle));
    NAPI_CALL_RETURN(
        env, napi_set_named_property(env, *result, "requestHandle", jsHandle));
  } else {
    C2J_SET_PROPERTY_CALL_RETURN(
        env, *result, "requestHandle",
        NAPI_CALL_RETURN(env, napi_get_null(env, &jsValue)));
  }

  C2J_SET_NUMBER_MEMBER_RETURN(env, *result, request, method);
  C2J_SET_PROPERTY_CALL_RETURN(
      env, *result, "devAddr",
      HELPER_CALL_RETURN(js_OCDevAddr(env, &(request->devAddr), &jsValue)));
  C2J_SET_STRING_IF_NOT_NULL_RETURN(env, *result, request, query);

  napi_value jsObsInfo;
  NAPI_CALL_RETURN(env, napi_create_object(env, &jsObsInfo));
  C2J_SET_NUMBER_MEMBER_RETURN(env, jsObsInfo, &(request->obsInfo), action);
  C2J_SET_NUMBER_MEMBER_RETURN(env, jsObsInfo, &(request->obsInfo), obsId);
  NAPI_CALL_RETURN(env,
                   napi_set_named_property(env, *result, "obsInfo", jsObsInfo));

  C2J_SET_NUMBER_MEMBER_RETURN(env, *result, request, messageID);
  if (request->payload) {
    C2J_SET_PROPERTY_CALL_RETURN(
        env, *result, "payload",
        HELPER_CALL_RETURN(js_OCPayload(env, request->payload, &jsValue)));
  }

  // "rcvdVendorSpecificHeaderOptions" ignored

  return std::string();
}
