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

#include <stdio.h>
#include "../common.h"
#include "../structures/handles.h"
#include "../structures/oc-client-response.h"
#include "../structures/oc-dev-addr.h"
/*
#include "../structures/oc-header-option-array.h"
#include "../structures/oc-payload.h"
*/
extern "C" {
#include <ocpayload.h>
#include <ocstack.h>
}

static void deleteCallback(void *data) {
  NapiHandleScope scope;
  HELPER_CALL_THROW(scope.env, JSOCDoHandle::Destroy(scope.env, (JSOCDoHandle *)data));
}

static OCStackApplicationResult defaultOCClientResponseHandler(
    void *context, OCDoHandle, OCClientResponse *clientResponse) {
  NapiHandleScope scope;
  OCStackApplicationResult failReturn = OC_STACK_KEEP_TRANSACTION;

  JSOCDoHandle *cData = (JSOCDoHandle *)context;
  napi_value jsContext, jsCallback, jsReturnValue;
  NAPI_CALL(napi_get_null(scope.env, &jsContext), THROW_BODY(scope.env, failReturn));
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

  J2C_GET_VALUE_JS(OCStackApplicationResult, cResult, scope.env, jsReturnValue,
                   napi_number, "OCDoResource response callback return value",
                   uint32, uint32_t, THROW_BODY(scope.env, failReturn));

  return cResult;
}

NAPI_METHOD(bind_OCDoResource) {
  J2C_GET_ARGUMENTS(env, info, 9);
  J2C_VALIDATE_VALUE_TYPE_THROW(env, arguments[0], napi_object, "handle");
  J2C_GET_VALUE_JS_THROW(OCMethod, method, env, arguments[1], napi_number,
                         "method", uint32, uint32_t);
  J2C_GET_STRING_ARGUMENT_THROW(requestUri, env, arguments[2], false,
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

  J2C_GET_VALUE_JS_THROW(OCConnectivityType, connectivityType, env,
                         arguments[5], napi_number, "connectivityType", uint32,
                         uint32_t);
  J2C_GET_VALUE_JS_THROW(OCQualityOfService, qos, env, arguments[6],
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

/*
static void deleteNanCallback(CallbackInfo<OCDoHandle> *handle) {
  Nan::HandleScope scope;

  delete handle;
}

// Create an object containing the information from an OCClientResponse
// structure
static OCStackApplicationResult defaultOCClientResponseHandler(
    void *context, OCDoHandle handle, OCClientResponse *clientResponse) {
  CallbackInfo<OCDoHandle> *callbackInfo = (CallbackInfo<OCDoHandle> *)context;

  CALL_JS(
      &(callbackInfo->callback), Nan::GetCurrentContext()->Global(), 2,
      OC_STACK_KEEP_TRANSACTION, IsUint32,
      "OCClientResponseHandler return value",
      return ((OCStackApplicationResult)Nan::To<uint32_t>(jsReturn).FromJust()),
      Nan::New(callbackInfo->jsHandle), js_OCClientResponse(clientResponse));
}

NAN_METHOD(bind_OCDoResource) {
  VALIDATE_ARGUMENT_COUNT(info, 8);
  VALIDATE_ARGUMENT_TYPE(info, 0, IsObject);
  VALIDATE_ARGUMENT_TYPE(info, 1, IsUint32);
  VALIDATE_ARGUMENT_TYPE(info, 2, IsString);
  VALIDATE_ARGUMENT_TYPE_OR_NULL(info, 3, IsObject);
  VALIDATE_ARGUMENT_TYPE_OR_NULL(info, 4, IsObject);
  VALIDATE_ARGUMENT_TYPE(info, 5, IsUint32);
  VALIDATE_ARGUMENT_TYPE(info, 6, IsUint32);
  VALIDATE_ARGUMENT_TYPE(info, 7, IsFunction);
  VALIDATE_ARGUMENT_TYPE_OR_NULL(info, 8, IsArray);

  OCDevAddr *destination = 0, destinationToFillIn;
  OCPayload *payload = 0;
  OCHeaderOption *options = 0;
  uint8_t optionCount = 0;
  OCCallbackData data;

  if (info[8]->IsArray()) {
    Local<Array> optionArray = Local<Array>::Cast(info[8]);
    size_t length = optionArray->Length();

    if (length > 0) {
      options = (OCHeaderOption *)malloc(length * sizeof(OCHeaderOption));
      if (!options) {
        Nan::ThrowError(
            "Ran out of memory attempting to allocate header options");
        return;
      }
      if (!c_OCHeaderOption(optionArray, options, &optionCount)) {
        free(options);
        return;
      }
    }
  }

  // If a destination is given, we only use it if it can be converted to a
  // OCDevAddr structure
  if (info[3]->IsObject()) {
    if (c_OCDevAddr(Nan::To<Object>(info[3]).ToLocalChecked(),
                    &destinationToFillIn)) {
      destination = &destinationToFillIn;
    } else {
      free(options);
      return;
    }
  }

  // If a payload is given, we only use it if it can be converted to a
  // OCPayload*
  if (info[4]->IsObject()) {
    if (!c_OCPayload(Nan::To<Object>(info[4]).ToLocalChecked(), &payload)) {
      free(options);
      return;
    }
  }

  CallbackInfo<OCDoHandle> *callbackInfo = new CallbackInfo<OCDoHandle>;
  if (!callbackInfo) {
    Nan::ThrowError("OCDoResource: Failed to allocate callback info");
    free(options);
    return;
  }

  data.context = (void *)callbackInfo;
  data.cb = defaultOCClientResponseHandler;
  data.cd = (OCClientContextDeleter)deleteNanCallback;

  OCStackResult returnValue = OCDoResource(
      &(callbackInfo->handle), (OCMethod)Nan::To<uint32_t>(info[1]).FromJust(),
      (const char *)*String::Utf8Value(info[2]), destination, payload,
      (OCConnectivityType)Nan::To<uint32_t>(info[5]).FromJust(),
      (OCQualityOfService)Nan::To<uint32_t>(info[6]).FromJust(), &data, options,
      (uint8_t)Nan::To<uint32_t>(info[9]).FromJust());

  free(options);

  // We need not free the payload because it seems iotivity takes ownership.
  // Similarly, if OCDoResource() fails, iotivity calls the callback that frees
  // the data on our behalf.

  if (returnValue == OC_STACK_OK) {
    Nan::Set(Nan::To<Object>(info[0]).ToLocalChecked(),
             Nan::New("handle").ToLocalChecked(),
             callbackInfo->Init(JSOCDoHandle::New(callbackInfo),
                                Local<Function>::Cast(info[7])));
  }

  info.GetReturnValue().Set(Nan::New(returnValue));
}

NAN_METHOD(bind_OCCancel) {
  VALIDATE_ARGUMENT_COUNT(info, 3);
  VALIDATE_ARGUMENT_TYPE(info, 0, IsObject);
  VALIDATE_ARGUMENT_TYPE(info, 1, IsUint32);
  VALIDATE_ARGUMENT_TYPE_OR_NULL(info, 2, IsArray);

  OCHeaderOption headerOptions[MAX_HEADER_OPTIONS] = {{OC_INVALID_ID, 0, 0, 0}};
  uint8_t numberOfOptions = 0;

  if (info[2]->IsArray()) {
    if (!c_OCHeaderOption(Local<Array>::Cast(info[2]), headerOptions,
                          &numberOfOptions)) {
      return;
    }
  }

  CallbackInfo<OCDoHandle> *callbackInfo;
  JSCALLBACKHANDLE_RESOLVE(JSOCDoHandle, callbackInfo,
                           Nan::To<Object>(info[0]).ToLocalChecked());
  info.GetReturnValue().Set(Nan::New(OCCancel(

      callbackInfo->handle,
      (OCQualityOfService)Nan::To<uint32_t>(info[1]).FromJust(), headerOptions,
      numberOfOptions)));
}
*/
