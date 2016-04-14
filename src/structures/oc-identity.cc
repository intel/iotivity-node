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

#include <nan.h>
#include "oc-identity.h"
#include "handles.h"
#include "../common.h"

extern "C" {
#include <string.h>
}

using namespace v8;

Local<Array> js_OCIdentity(OCIdentity *identity) {
  return jsArrayFromBytes(identity->id, identity->id_length);
}

bool c_OCIdentity(Local<Array> jsIdentity, OCIdentity *identity) {
  size_t length = jsIdentity->Length();
  if (length > MAX_IDENTITY_SIZE) {
    Nan::ThrowError("OCIdentity array length exceed MAX_IDENTITY_SIZE");
    return false;
  }

  if (!fillCArrayFromJSArray(identity->id, length, jsIdentity)) {
    return false;
  }

  identity->id_length = (uint16_t)length;
  return true;
}
