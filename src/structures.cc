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
  Local<Object> jsRequest = NanNew<Object>();

  jsRequest->Set(NanNew<String>("resource"),
                 js_OCResourceHandle(request->resource));

  jsRequest->Set(NanNew<String>("requestHandle"),
                 js_OCRequestHandle(request->requestHandle));

  jsRequest->Set(NanNew<String>("method"), NanNew<Number>(request->method));

  SET_STRING_IF_NOT_NULL(jsRequest, request, query);

  Local<Object> obsInfo = NanNew<Object>();
  obsInfo->Set(NanNew<String>("action"),
               NanNew<Number>(request->obsInfo.action));
  obsInfo->Set(NanNew<String>("obsId"), NanNew<Number>(request->obsInfo.obsId));
  jsRequest->Set(NanNew<String>("obsInfo"), obsInfo);

  jsRequest->Set(
      NanNew<String>("rcvdVendorSpecificHeaderOptions"),
      js_OCHeaderOption(request->rcvdVendorSpecificHeaderOptions,
                        request->numRcvdVendorSpecificHeaderOptions));

  jsRequest->Set(NanNew<String>("devAddr"), js_OCDevAddr(&(request->devAddr)));
  if (request->payload) {
    jsRequest->Set(NanNew<String>("payload"), js_OCPayload(request->payload));
  }

  return jsRequest;
}
