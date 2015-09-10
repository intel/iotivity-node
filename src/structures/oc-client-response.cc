#include <nan.h>
#include "oc-client-response.h"
#include "oc-dev-addr.h"
#include "../common.h"
#include "oc-header-option-array.h"
#include "oc-payload.h"

extern "C" {
#include <string.h>
}

using namespace v8;

Local<Object> js_OCClientResponse(OCClientResponse *response) {
  Local<Object> returnValue = NanNew<Object>();

  // response.devAddr
  returnValue->Set(NanNew<String>("devAddr"),
                   js_OCDevAddr(&(response->devAddr)));

  // response.addr
  if (response->addr) {
    returnValue->Set(NanNew<String>("addr"), js_OCDevAddr(response->addr));
  }

  // response.payload
  if (response->payload) {
    returnValue->Set(NanNew<String>("payload"),
                     js_OCPayload(response->payload));
  }

  SET_VALUE_ON_OBJECT(returnValue, Number, response, connType);
  SET_VALUE_ON_OBJECT(returnValue, Number, response, result);
  SET_VALUE_ON_OBJECT(returnValue, Number, response, sequenceNumber);

  // FIXME - iotivity has a bug whereby these fields are left uninitialized in
  // a presence response
  if (!(response->payload &&
        response->payload->type == PAYLOAD_TYPE_PRESENCE)) {
    SET_STRING_IF_NOT_NULL(returnValue, response, resourceUri);

    // response.rcvdVendorSpecificHeaderOptions
    returnValue->Set(
        NanNew<String>("rcvdVendorSpecificHeaderOptions"),
        js_OCHeaderOption(response->rcvdVendorSpecificHeaderOptions,
                          response->numRcvdVendorSpecificHeaderOptions));
  }

  return returnValue;
}
