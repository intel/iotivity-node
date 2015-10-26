#include <nan.h>
#include "oc-entity-handler-response.h"
#include "oc-dev-addr.h"
#include "handles.h"
#include "../common.h"
#include "oc-header-option-array.h"
#include "oc-payload.h"

extern "C" {
#include <string.h>
#include <ocpayload.h>
}

using namespace v8;

bool c_OCEntityHandlerResponse(Local<Object> jsResponse,
                               OCEntityHandlerResponse *p_response) {
  OCEntityHandlerResponse response = {
      0, 0, OC_EH_ERROR, 0, 0, {{OC_INVALID_ID, 0, 0, {0}}}, {0}, false};

  // requestHandle
  Local<Value> requestHandle =
      Nan::Get(jsResponse, Nan::New("requestHandle").ToLocalChecked())
          .ToLocalChecked();
  VALIDATE_VALUE_TYPE(requestHandle, IsArray,
                      "entitiy handler response request handle", false);
  if (!c_OCRequestHandle(Local<Array>::Cast(requestHandle),
                         &(response.requestHandle))) {
    return false;
  }

  // resourceHandle
  Local<Value> resourceHandle =
      Nan::Get(jsResponse, Nan::New("resourceHandle").ToLocalChecked())
          .ToLocalChecked();
  VALIDATE_VALUE_TYPE(requestHandle, IsArray,
                      "entitiy handler response resource handle", false);
  if (!c_OCRequestHandle(Local<Array>::Cast(resourceHandle),
                         &(response.resourceHandle))) {
    return false;
  }

  // ehResult
  VALIDATE_AND_ASSIGN(response, ehResult, OCEntityHandlerResult, IsUint32,
                      "(entity handler response)", false, jsResponse,
                      Uint32Value);

  // payload
  response.payload = 0;
  Local<Value> payload =
      Nan::Get(jsResponse, Nan::New("payload").ToLocalChecked())
          .ToLocalChecked();
  if (!payload->IsNull()) {
    VALIDATE_VALUE_TYPE(payload, IsObject, "entity handler response payload",
                        false);
    if (!c_OCPayload(payload->ToObject(), &(response.payload))) {
      return false;
    }
  }

  //
  // After this point we need to free the payload whenever we bail.
  //

  // sendVendorSpecificHeaderOptions and numSendVendorSpecificHeaderOptions
  Local<Value> sendVendorSpecificHeaderOptions =
      Nan::Get(jsResponse, Nan::New("sendVendorSpecificHeaderOptions")
                               .ToLocalChecked()).ToLocalChecked();
  VALIDATE_VALUE_TYPE_OR_FREE(sendVendorSpecificHeaderOptions, IsArray,
                              "entity handler response header options", false,
                              response.payload, OCPayloadDestroy);
  if (!c_OCHeaderOption(Local<Array>::Cast(sendVendorSpecificHeaderOptions),
                        response.sendVendorSpecificHeaderOptions,
                        &(response.numSendVendorSpecificHeaderOptions))) {
    OCPayloadDestroy(response.payload);
    return false;
  }

  // resourceUri
  Local<Value> resourceUri =
      Nan::Get(jsResponse, Nan::New("resourceUri").ToLocalChecked())
          .ToLocalChecked();
  VALIDATE_VALUE_TYPE_OR_FREE(resourceUri, IsString,
                              "entity handler response resource URI", false,
                              response.payload, OCPayloadDestroy);
  size_t length = strlen((const char *)*String::Utf8Value(resourceUri));
  if (length >= MAX_URI_LENGTH) {
    Nan::ThrowRangeError(
        "entity handler response resource URI cannot fit inside MAX_URI_LENGTH "
        "(remember terminating zero)");
    OCPayloadDestroy(response.payload);
    return false;
  }
  strcpy(response.resourceUri, (const char *)*String::Utf8Value(resourceUri));

  memcpy(p_response, &response, sizeof(OCEntityHandlerResponse));
  return true;
}
