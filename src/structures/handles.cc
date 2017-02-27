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

#include "handles.h"

NAPI_METHOD(JSHandle_constructor) {}

std::string InitHandles(napi_env env) {
  HELPER_CALL_RETURN(JSOCDoHandle::InitClass(env));
  HELPER_CALL_RETURN(JSOCResourceHandle::InitClass(env));
  return std::string();
}

std::map<OCResourceHandle, napi_ref> JSOCResourceHandle::handles;

/*
#include <nan.h>
#include "../common.h"
#include "handles.h"

using namespace v8;

std::map<OCResourceHandle, Nan::Persistent<v8::Object> *>
    JSOCResourceHandle::handles;

Local<Array> jsArrayFromBytes(unsigned char *bytes, uint32_t length) {
  uint32_t index;
  Local<Array> returnValue = Nan::New<Array>(length);

  for (index = 0; index < length; index++) {
    Nan::Set(returnValue, index, Nan::New(bytes[index]));
  }
  return returnValue;
}

bool fillCArrayFromJSArray(unsigned char *bytes, uint32_t length,
                           Local<Array> array) {
  uint32_t index, arrayLength;

  arrayLength = array->Length();
  if (arrayLength != length) {
    Nan::ThrowError("byte array has the wrong length");
    return false;
  }

  for (index = 0; index < length; index++) {
    Local<Value> byte = Nan::Get(array, index).ToLocalChecked();
    VALIDATE_VALUE_TYPE(byte, IsUint32, "byte array item", return false);
    bytes[index] = (unsigned char)(Nan::To<uint32_t>(byte).FromJust());
  }

  return true;
}
*/
