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
#include "oc-sid.h"
#include "../common.h"

extern "C" {
#include <string.h>
#include <ocrandom.h>
}

using namespace v8;

void fillJSSid(Local<Array> jsSid, char *sid) {
  int index;
  for (index = 0; index < UUID_SIZE; index++) {
    jsSid->Set(index, Nan::New(sid[index]));
  }
}

Local<Array> js_SID(char *sid) {
  Local<Array> returnValue = Nan::New<Array>(UUID_SIZE);

  fillJSSid(returnValue, sid);

  return returnValue;
}

bool c_SID(Local<Array> jsSid, char *sid) {
  int index;
  char result[UUID_SIZE];

  if (jsSid->Length() > UUID_SIZE) {
    Nan::ThrowRangeError("SID length must be UUID_SIZE");
    return false;
  }

  for (index = 0; index < UUID_SIZE; index++) {
    Local<Value> oneByte = Nan::Get(jsSid, index).ToLocalChecked();
    VALIDATE_VALUE_TYPE(oneByte, IsUint32, "SID byte", false);
    result[index] = oneByte->Uint32Value();
  }

  memcpy(sid, result, UUID_SIZE * sizeof(char));
  return true;
}
