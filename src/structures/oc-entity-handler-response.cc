#include <nan.h>
#include "oc-entity-handler-response.h"
#include "oc-dev-addr.h"
#include "handles.h"
#include "../common.h"
#include "oc-header-option-array.h"
#include "oc-payload.h"

extern "C" {
#include <string.h>
}

using namespace v8;

bool c_OCEntityHandlerResponse(Local<Object> jsResponse,
                               OCEntityHandlerResponse *p_response) {
  OCEntityHandlerResponse response;

  // requestHandle
  Local<Value> requestHandle = jsResponse->Get(NanNew<String>("requestHandle"));
  VALIDATE_VALUE_TYPE(requestHandle, IsArray,
                      "entitiy handler response request handle", false);
  if (!c_OCRequestHandle(Local<Array>::Cast(requestHandle),
                         &(response.requestHandle))) {
    return false;
  }

  // resourceHandle
  Local<Value> resourceHandle =
      jsResponse->Get(NanNew<String>("resourceHandle"));
  VALIDATE_VALUE_TYPE(requestHandle, IsArray,
                      "entitiy handler response resource handle", false);
  if (!c_OCRequestHandle(Local<Array>::Cast(resourceHandle),
                         &(response.resourceHandle))) {
    return false;
  }

  // ehResult
  Local<Value> ehResult = jsResponse->Get(NanNew<String>("ehResult"));
  VALIDATE_VALUE_TYPE(ehResult, IsUint32, "entity handler response result",
                      false);
  response.ehResult = (OCEntityHandlerResult)ehResult->Uint32Value();

  // payload
  Local<Value> payload = jsResponse->Get(NanNew<String>("payload"));
  VALIDATE_VALUE_TYPE(payload, IsObject, "entity handler response payload",
                      false);
  if (!c_OCPayload(payload->ToObject(), &(response.payload))) {
    return false;
  }

  // sendVendorSpecificHeaderOptions and numSendVendorSpecificHeaderOptions
  Local<Value> sendVendorSpecificHeaderOptions =
      jsResponse->Get(NanNew<String>("sendVendorSpecificHeaderOptions"));
  VALIDATE_VALUE_TYPE(sendVendorSpecificHeaderOptions, IsArray,
                      "entity handler response header options", false);
  if (!c_OCHeaderOption(Local<Array>::Cast(sendVendorSpecificHeaderOptions),
                        response.sendVendorSpecificHeaderOptions,
                        &(response.numSendVendorSpecificHeaderOptions))) {
    return false;
  }

  // resourceUri
  Local<Value> resourceUri = jsResponse->Get(NanNew<String>("resourceUri"));
  VALIDATE_VALUE_TYPE(resourceUri, IsString,
                      "entity handler response resource URI", false);
  size_t length = strlen((const char *)*String::Utf8Value(resourceUri));
  if (length >= MAX_URI_LENGTH) {
    NanThrowRangeError(
        "entity handler response resource URI cannot fit inside MAX_URI_LENGTH "
        "(remember terminating zero)");
    return false;
  }
  strcpy(response.resourceUri, (const char *)*String::Utf8Value(resourceUri));

  memcpy(p_response, &response, sizeof(OCEntityHandlerResponse));
  return true;
}
