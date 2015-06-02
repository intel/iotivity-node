#include <node_buffer.h>
#include "oc-do-resource.h"
#include "../common.h"
#include "../structures.h"

extern "C" {
#include <stdlib.h>
#include <ocstack.h>
}

using namespace v8;
using namespace node;

NAN_METHOD(bind_OCDoResponse) {
  NanScope();

  OCEntityHandlerResponse response;
  char payload[MAX_RESPONSE_LENGTH];

  VALIDATE_ARGUMENT_COUNT(args, 1);
  VALIDATE_ARGUMENT_TYPE(args, 0, IsObject);

  response.payload = payload;

  if (c_OCEntityHandlerResponse(&response, args[0]->ToObject())) {
    NanReturnValue(NanNew<Number>(OCDoResponse(&response)));
  } else {
    NanReturnUndefined();
  }
}
