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
  VALIDATE_ARGUMENT_COUNT(info, 3);
  VALIDATE_ARGUMENT_TYPE(info, 0, IsArray);
  VALIDATE_ARGUMENT_TYPE(info, 1, IsUint32);
  VALIDATE_ARGUMENT_TYPE_OR_NULL(info, 2, IsArray);

  OCDoHandle handle;
  OCHeaderOption headerOptions[MAX_HEADER_OPTIONS] = {
      {OC_INVALID_ID, 0, 0, {0}}};
  uint8_t numberOfOptions = 0;

  if (!c_OCDoHandle(Local<Array>::Cast(info[0]), &handle)) {
    return;
  }

  if (info[2]->IsArray()) {
    if (!c_OCHeaderOption(Local<Array>::Cast(info[2]), headerOptions,
                          &numberOfOptions)) {
      return;
    }
  }

  info.GetReturnValue().Set(
      Nan::New(OCCancel(handle, (OCQualityOfService)info[1]->Uint32Value(),
                        headerOptions, numberOfOptions)));
}
