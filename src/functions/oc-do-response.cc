#include "oc-do-response.h"
#include "../common.h"
#include "../structures/oc-entity-handler-response.h"

extern "C" {
#include <stdlib.h>
#include <ocstack.h>
#include <ocpayload.h>
}

using namespace v8;
using namespace node;

NAN_METHOD(bind_OCDoResponse) {
  VALIDATE_ARGUMENT_COUNT(info, 1);
  VALIDATE_ARGUMENT_TYPE(info, 0, IsObject);

  OCEntityHandlerResponse response;
  OCStackResult result;

  if (!c_OCEntityHandlerResponse(info[0]->ToObject(), &response)) {
    return;
  }

  result = OCDoResponse(&response);
  if (response.payload) {
    OCPayloadDestroy(response.payload);
  }
  info.GetReturnValue().Set(Nan::New(result));
}
