#include <nan.h>

extern "C" {
#include <string.h>
}

#include "structures.h"
#include "structures/oc-header-option-array.h"
#include "structures/oc-dev-addr.h"
#include "structures/oc-payload.h"
#include "structures/handles.h"
#include "common.h"

using namespace v8;
using namespace node;

Local<Object> js_OCEntityHandlerRequest(OCEntityHandlerRequest *request) {
  Local<Object> jsRequest = Nan::New<Object>();

  Nan::Set(jsRequest, Nan::New("resource").ToLocalChecked(),
           js_OCResourceHandle(request->resource));

  Nan::Set(jsRequest, Nan::New("requestHandle").ToLocalChecked(),
           js_OCRequestHandle(request->requestHandle));

  SET_VALUE_ON_OBJECT(jsRequest, Number, request, method);
  SET_STRING_IF_NOT_NULL(jsRequest, request, query);

  Local<Object> obsInfo = Nan::New<Object>();
  SET_VALUE_ON_OBJECT(obsInfo, Number, &(request->obsInfo), action);
  SET_VALUE_ON_OBJECT(obsInfo, Number, &(request->obsInfo), obsId);
  Nan::Set(jsRequest, Nan::New("obsInfo").ToLocalChecked(), obsInfo);

  Nan::Set(jsRequest,
           Nan::New("rcvdVendorSpecificHeaderOptions").ToLocalChecked(),
           js_OCHeaderOption(request->rcvdVendorSpecificHeaderOptions,
                             request->numRcvdVendorSpecificHeaderOptions));

  Nan::Set(jsRequest, Nan::New("devAddr").ToLocalChecked(),
           js_OCDevAddr(&(request->devAddr)));
  if (request->payload) {
    Nan::Set(jsRequest, Nan::New("payload").ToLocalChecked(),
             js_OCPayload(request->payload));
  }

  return jsRequest;
}
