/*
 * Copyright 2016 Intel Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <v8.h>
#include <nan.h>

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
  VALIDATE_ARGUMENT_TYPE(info, 0, IsObject);
  VALIDATE_ARGUMENT_TYPE(info, 1, IsUint32);
  VALIDATE_ARGUMENT_TYPE_OR_NULL(info, 2, IsArray);

  OCDoHandle handle;
  OCHeaderOption headerOptions[MAX_HEADER_OPTIONS] = {
      {OC_INVALID_ID, 0, 0, {0}}};
  uint8_t numberOfOptions = 0;

  if (!c_OCDoHandle(Nan::To<Object>(info[0]).ToLocalChecked(), &handle)) {
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
