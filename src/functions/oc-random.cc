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
#include "../common.h"
#include "../structures/oc-sid.h"

extern "C" {
#include <ocrandom.h>
}

using namespace v8;

NAN_METHOD(bind_OCConvertUuidToString) {
  VALIDATE_ARGUMENT_COUNT(info, 2);
  VALIDATE_ARGUMENT_TYPE(info, 0, IsArray);
  VALIDATE_ARGUMENT_TYPE(info, 1, IsObject);

  OCRandomUuidResult result;
  char sid[UUID_SIZE] = {0};
  char stringSid[UUID_STRING_SIZE] = "";

  if (!c_SID(Local<Array>::Cast(info[0]), sid)) {
    return;
  }

  result = OCConvertUuidToString((const uint8_t *)sid, stringSid);

  Local<Object> receptacle = Local<Object>::Cast(info[1]);

  receptacle->Set(Nan::New("sid").ToLocalChecked(),
                  Nan::New(stringSid).ToLocalChecked());

  info.GetReturnValue().Set(Nan::New(result));
}

NAN_METHOD(bind_OCFillRandomMem) {
  VALIDATE_ARGUMENT_COUNT(info, 1);
  VALIDATE_ARGUMENT_TYPE(info, 0, IsArray);

  Local<Array> destination = Local<Array>::Cast(info[0]);
  ;
  uint16_t length = destination->Length();
  uint8_t *cDestination = (uint8_t *)malloc(length * sizeof(uint8_t));
  memset(cDestination, 0, length * sizeof(uint8_t));
  OCFillRandomMem(cDestination, length);
  for (int index = 0; index < length; index++) {
    destination->Set(index, Nan::New(cDestination[index]));
  }
  free(cDestination);
}

NAN_METHOD(bind_OCGenerateUuid) {
  VALIDATE_ARGUMENT_COUNT(info, 1);
  VALIDATE_ARGUMENT_TYPE(info, 0, IsArray);

  Local<Array> destination = Local<Array>::Cast(info[0]);
  uint32_t length = destination->Length();
  if (length != UUID_SIZE) {
    Nan::ThrowRangeError("Array is not of length UUID_SIZE");
    return;
  }
  char cDestination[UUID_SIZE] = {0};
  OCRandomUuidResult result = OCGenerateUuid((uint8_t *)cDestination);
  fillJSSid(destination, cDestination);
  info.GetReturnValue().Set(Nan::New(result));
}

NAN_METHOD(bind_OCGenerateUuidString) {
  VALIDATE_ARGUMENT_COUNT(info, 1);
  VALIDATE_ARGUMENT_TYPE(info, 0, IsObject);

  char resultingString[UUID_STRING_SIZE] = "";
  OCRandomUuidResult result = OCGenerateUuidString(resultingString);
  Local<Object> destination = Local<Object>::Cast(info[0]);
  destination->Set(Nan::New("sid").ToLocalChecked(),
                   Nan::New(resultingString).ToLocalChecked());

  info.GetReturnValue().Set(Nan::New(result));
}

NAN_METHOD(bind_OCGetRandom) {
  VALIDATE_ARGUMENT_COUNT(info, 0);

  info.GetReturnValue().Set(Nan::New(OCGetRandom()));
}

NAN_METHOD(bind_OCGetRandomByte) {
  VALIDATE_ARGUMENT_COUNT(info, 0);

  info.GetReturnValue().Set(Nan::New(OCGetRandomByte()));
}

NAN_METHOD(bind_OCGetRandomRange) {
  VALIDATE_ARGUMENT_COUNT(info, 2);
  VALIDATE_ARGUMENT_TYPE(info, 0, IsUint32);
  VALIDATE_ARGUMENT_TYPE(info, 1, IsUint32);

  info.GetReturnValue().Set(Nan::New(
      OCGetRandomRange(info[0]->Uint32Value(), info[1]->Uint32Value())));
}

NAN_METHOD(bind_OCSeedRandom) {
  VALIDATE_ARGUMENT_COUNT(info, 0);

  info.GetReturnValue().Set(Nan::New(OCSeedRandom()));
}
