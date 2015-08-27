#include <v8.h>
#include <nan.h>

#include "oc-cancel.h"
#include "../common.h"
#include "../structures/handles.h"
#include "../structures/oc-header-option-array.h"

extern "C" {
#include <ocstack.h>
}

using namespace v8;
using namespace node;

NAN_METHOD(bind_OCCancel) {
  NanScope();

  VALIDATE_ARGUMENT_COUNT(args, 3);
  VALIDATE_ARGUMENT_TYPE(args, 0, IsArray);
  VALIDATE_ARGUMENT_TYPE(args, 1, IsUint32);
  VALIDATE_ARGUMENT_TYPE(args, 2, IsArray);

  OCDoHandle handle;
  OCHeaderOption headerOptions[MAX_HEADER_OPTIONS];
  uint8_t numberOfOptions;

  if (!c_OCDoHandle(Local<Array>::Cast(args[0]), &handle)) {
    NanReturnUndefined();
  }

  if (!c_OCHeaderOption(Local<Array>::Cast(args[2]), headerOptions,
                        &numberOfOptions)) {
    NanReturnUndefined();
  }

  NanReturnValue(NanNew<Number>(
      OCCancel(handle, (OCQualityOfService)args[1]->Uint32Value(),
               headerOptions, numberOfOptions)));
}
