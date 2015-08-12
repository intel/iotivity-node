#include <node_buffer.h>
#include "oc-do-resource.h"
#include "../common.h"
#include "../structures/oc-entity-handler-response.h"

extern "C" {
#include <stdlib.h>
#include <ocstack.h>
}

using namespace v8;
using namespace node;

NAN_METHOD(bind_OCDoResponse) {
  NanScope();

  OCEntityHandlerResponse response;

  VALIDATE_ARGUMENT_COUNT(args, 1);
  VALIDATE_ARGUMENT_TYPE(args, 0, IsObject);

  if (c_OCEntityHandlerResponse(args[0]->ToObject(), &response)) {
    NanReturnValue(NanNew<Number>(OCDoResponse(&response)));
  } else {
    NanReturnUndefined();
  }
}
