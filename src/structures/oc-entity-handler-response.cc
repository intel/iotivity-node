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
  DECLARE_VALUE_TYPE(resHandleType, env, resourceHandle, RETURN_FAIL);
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
            RETURN_FAIL);
  char *cResourceUri;
  J2C_GET_STRING_JS_RETURN(env, cResourceUri, resourceUri, false,
                           "response.resourceUri");
  strncpy(response->resourceUri, cResourceUri, MAX_URI_LENGTH);
  free(cResourceUri);

  // persistent buffer flag is ignored
  response->persistentBufferFlag = false;

  // payload
  // This is the only deep property so we set it last.
  J2C_DECLARE_PROPERTY_JS_RETURN(jsPayload, env, value, "payload");
  HELPER_CALL_RETURN(c_OCPayload(env, jsPayload, &(response->payload)));

  return std::string();
}
/*
bool c_OCEntityHandlerResponse(Local<Object> jsResponse,
                               OCEntityHandlerResponse *p_response) {
  OCEntityHandlerResponse response = {
      0, 0, OC_EH_ERROR, 0, 0, {{OC_INVALID_ID, 0, 0, 0}}, {0}, false};

  // requestHandle
  Local<Value> requestHandle =
      Nan::Get(jsResponse, Nan::New("requestHandle").ToLocalChecked())
          .ToLocalChecked();
  VALIDATE_VALUE_TYPE(requestHandle, IsObject,
                      "entitiy handler response request handle", return false);
  response.requestHandle = JSOCRequestHandle::Resolve(
      Nan::To<Object>(requestHandle).ToLocalChecked());
  if (!response.requestHandle) {
    return false;
  }

  // resourceHandle
  response.resourceHandle = 0;
  Local<Value> resourceHandle =
      Nan::Get(jsResponse, Nan::New("resourceHandle").ToLocalChecked())
          .ToLocalChecked();
  if (!(resourceHandle->IsUndefined() || resourceHandle->IsNull())) {
    VALIDATE_VALUE_TYPE(resourceHandle, IsObject,
                        "entitiy handler response resource handle",
                        return false);
    CallbackInfo<OCResourceHandle> *callbackInfo;
    JSCALLBACKHANDLE_RESOLVE(JSOCResourceHandle, callbackInfo,
                             Nan::To<Object>(resourceHandle).ToLocalChecked(),
                             false);
    response.resourceHandle = callbackInfo->handle;
  }

  // ehResult
  VALIDATE_AND_ASSIGN(response, jsResponse, ehResult, OCEntityHandlerResult,
                      IsUint32, "(entity handler response)", uint32_t,
                      return false);

  // payload
  response.payload = 0;
  Local<Value> payload =
      Nan::Get(jsResponse, Nan::New("payload").ToLocalChecked())
          .ToLocalChecked();
  if (!payload->IsNull()) {
    VALIDATE_VALUE_TYPE(payload, IsObject, "entity handler response payload",
                        return false);
    if (!c_OCPayload(Nan::To<Object>(payload).ToLocalChecked(),
                     &(response.payload))) {
      return false;
    }
  }

  //
  // After this point we need to free the payload whenever we bail.
  //

  Local<Value> resourceUri;
  size_t length;

  // sendVendorSpecificHeaderOptions and numSendVendorSpecificHeaderOptions
  Local<Value> sendVendorSpecificHeaderOptions =
      Nan::Get(jsResponse,
               Nan::New("sendVendorSpecificHeaderOptions").ToLocalChecked())
          .ToLocalChecked();
  VALIDATE_VALUE_TYPE(sendVendorSpecificHeaderOptions, IsArray,
                      "entity handler response header options", goto free);
  if (!c_OCHeaderOption(Local<Array>::Cast(sendVendorSpecificHeaderOptions),
                        response.sendVendorSpecificHeaderOptions,
                        &(response.numSendVendorSpecificHeaderOptions))) {
    goto free;
  }

  // resourceUri
  resourceUri = Nan::Get(jsResponse, Nan::New("resourceUri").ToLocalChecked())
                    .ToLocalChecked();
  VALIDATE_VALUE_TYPE(resourceUri, IsString,
                      "entity handler response resource URI", goto free);
  length = strlen((const char *)*String::Utf8Value(resourceUri));
  if (length >= MAX_URI_LENGTH) {
    Nan::ThrowRangeError(
        "entity handler response resource URI cannot fit inside MAX_URI_LENGTH "
        "(remember terminating zero)");
    goto free;
  }

  strncpy(response.resourceUri, (const char *)*String::Utf8Value(resourceUri),
          MAX_URI_LENGTH);

  *p_response = response;
  return true;
free:
  OCPayloadDestroy(response.payload);
  return false;
}
*/
