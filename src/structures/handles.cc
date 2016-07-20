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
#include "handles.h"
#include "../common.h"

using namespace v8;

template <class T>
class JSHandle {
  static Nan::Persistent<FunctionTemplate> &theTemplate() {
    static Nan::Persistent<FunctionTemplate> returnValue;

    if (returnValue.IsEmpty()) {
      Local<FunctionTemplate> theTemplate = Nan::New<FunctionTemplate>();
      theTemplate->SetClassName(Nan::New(T::jsClassName()).ToLocalChecked());
      theTemplate->InstanceTemplate()->SetInternalFieldCount(1);
      Nan::Set(Nan::GetFunction(theTemplate).ToLocalChecked(),
               Nan::New("displayName").ToLocalChecked(),
               Nan::New(T::jsClassName()).ToLocalChecked());
      returnValue.Reset(theTemplate);
    }
    return returnValue;
  }

 public:
  static Local<Object> New(void *data) {
    Local<Object> returnValue = Nan::GetFunction(Nan::New(theTemplate()))
                                    .ToLocalChecked()
                                    ->NewInstance();
    Nan::SetInternalFieldPointer(returnValue, 0, data);

    return returnValue;
  }

  // If the object is not of the expected type, or if the pointer inside the
  // object has already been removed, then we must throw an error
  static void *Resolve(Local<Object> jsObject) {
    void *returnValue = 0;

    if (Nan::New(theTemplate())->HasInstance(jsObject)) {
      returnValue = Nan::GetInternalFieldPointer(jsObject, 0);
    }
    if (!returnValue) {
      Nan::ThrowTypeError(
          (std::string("Object is not of type ") + T::jsClassName()).c_str());
    }
    return returnValue;
  }
};

Local<Array> jsArrayFromBytes(unsigned char *bytes, size_t length) {
  size_t index;
  Local<Array> returnValue = Nan::New<Array>(length);

  for (index = 0; index < length; index++) {
    Nan::Set(returnValue, index, Nan::New(bytes[index]));
  }
  return returnValue;
}

bool fillCArrayFromJSArray(unsigned char *bytes, size_t length,
                           Local<Array> array) {
  size_t index, arrayLength;

  arrayLength = array->Length();
  if (arrayLength != length) {
    Nan::ThrowError("byte array has the wrong length");
    return false;
  }

  for (index = 0; index < length; index++) {
    Local<Value> byte = Nan::Get(array, index).ToLocalChecked();
    VALIDATE_VALUE_TYPE(byte, IsUint32, "byte array item", false);
    bytes[index] = (unsigned char)(byte->Uint32Value());
  }

  return true;
}

class JSOCResourceHandle : public JSHandle<JSOCResourceHandle> {
 public:
  static const char *jsClassName() { return "OCResourceHandle"; }
};

class JSOCRequestHandle : public JSHandle<JSOCRequestHandle> {
 public:
  static const char *jsClassName() { return "OCRequestHandle"; }
};

class JSOCDoHandle : public JSHandle<JSOCDoHandle> {
 public:
  static const char *jsClassName() { return "OCDoHandle"; }
};

bool c_OCResourceHandle(Local<Object> handle, OCResourceHandle *p_cHandle) {
  OCResourceHandle local =
      (OCResourceHandle)JSOCResourceHandle::Resolve(handle);
  if (local) {
    *p_cHandle = local;
    return true;
  }
  return false;
}

bool c_OCRequestHandle(Local<Object> handle, OCRequestHandle *p_cHandle) {
  OCRequestHandle local = (OCRequestHandle)JSOCRequestHandle::Resolve(handle);
  if (local) {
    *p_cHandle = local;
    return true;
  }
  return false;
}

bool c_OCDoHandle(Local<Object> handle, OCDoHandle *p_cHandle) {
  OCDoHandle local = (OCDoHandle)JSOCDoHandle::Resolve(handle);
  if (local) {
    *p_cHandle = local;
    return true;
  }
  return false;
}

Local<Object> js_OCResourceHandle(OCResourceHandle handle) {
  return JSOCResourceHandle::New(handle);
}

Local<Object> js_OCRequestHandle(OCRequestHandle handle) {
  return JSOCRequestHandle::New(handle);
}

Local<Object> js_OCDoHandle(OCDoHandle handle) {
  return JSOCDoHandle::New(handle);
}
