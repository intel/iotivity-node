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
  NanScope();

  OCEntityHandlerResponse response;
  OCStackResult result;

  VALIDATE_ARGUMENT_COUNT(args, 1);
  VALIDATE_ARGUMENT_TYPE(args, 0, IsObject);

  if (!c_OCEntityHandlerResponse(args[0]->ToObject(), &response)) {
    NanReturnUndefined();
  }

  result = OCDoResponse(&response);
  if (response.payload) {
    OCPayloadDestroy(response.payload);
  }
  NanReturnValue(NanNew<Number>(result));
}
