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

#include "oc-entity-handler-response.h"
#include "../common.h"
#include "handles.h"
#include "oc-dev-addr.h"
#include "oc-payload.h"

extern "C" {
#include <ocpayload.h>
#include <string.h>
}

std::string c_OCEntityHandlerResponse(napi_env env, napi_value value,
                                      OCEntityHandlerResponse *response) {
  // requestHandle
  J2C_DECLARE_PROPERTY_JS_RETURN(requestHandle, env, value, "requestHandle");
  JSOCRequestHandle *cRequestData;
  HELPER_CALL_RETURN(JSOCRequestHandle::Get(env, requestHandle, &cRequestData));
  response->requestHandle = cRequestData->data;

  // resourceHandle
  response->resourceHandle = nullptr;
  J2C_DECLARE_PROPERTY_JS_RETURN(resourceHandle, env, value, "resourceHandle");
  DECLARE_VALUE_TYPE(resHandleType, env, resourceHandle, FAIL_STATUS);
  if (!(resHandleType == napi_null || resHandleType == napi_undefined)) {
    JSOCResourceHandle *cResData;
    HELPER_CALL_RETURN(JSOCResourceHandle::Get(env, resourceHandle, &cResData));
    response->resourceHandle = cResData->data;
  }

  // ehResult
  J2C_ASSIGN_MEMBER_VALUE_RETURN((env), response, value, OCEntityHandlerResult,
                                 ehResult, napi_number, "response", uint32,
                                 uint32_t);

  // header options are ignored
  response->numSendVendorSpecificHeaderOptions = 0;

  // resourceUri
  J2C_DECLARE_PROPERTY_JS_RETURN(resourceUri, env, value, "resourceUri");
  J2C_VALIDATE_VALUE_TYPE_RETURN(env, resourceUri, napi_string,
                                 "response.resourceUri");
  size_t len;
  NAPI_CALL_RETURN(
      env, napi_get_value_string_utf8(env, resourceUri, nullptr, 0, &len));
  JS_ASSERT(len <= MAX_URI_LENGTH,
            "length of response.resourceUri exceeds MAX_URI_LENGTH",
            FAIL_STATUS);
  char *cResourceUri;
  J2C_GET_STRING_JS_RETURN(env, cResourceUri, resourceUri, false,
                           "response.resourceUri");
  strncpy(response->resourceUri, cResourceUri, MAX_URI_LENGTH - 1);
  free(cResourceUri);

  // persistent buffer flag is ignored
  response->persistentBufferFlag = false;

  // payload
  // This is the only deep property so we set it last.
  J2C_DECLARE_PROPERTY_JS_RETURN(jsPayload, env, value, "payload");
  HELPER_CALL_RETURN(c_OCPayload(env, jsPayload, &(response->payload)));

  return std::string();
}
