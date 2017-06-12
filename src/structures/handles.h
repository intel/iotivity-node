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

#ifndef __IOTIVITY_NODE_HANDLES_H__
#define __IOTIVITY_NODE_HANDLES_H__

#include <nan.h>
#include <map>
extern "C" {
#include <ocstack.h>
}

template <class jsName, typename handleType>
class JSHandle {
  static Nan::Persistent<v8::FunctionTemplate> &theTemplate() {
    static Nan::Persistent<v8::FunctionTemplate> returnValue;

    if (returnValue.IsEmpty()) {
      v8::Local<v8::FunctionTemplate> theTemplate =
          Nan::New<v8::FunctionTemplate>();
      theTemplate->SetClassName(
          Nan::New(jsName::jsClassName()).ToLocalChecked());
      theTemplate->InstanceTemplate()->SetInternalFieldCount(1);
      Nan::Set(Nan::GetFunction(theTemplate).ToLocalChecked(),
               Nan::New("displayName").ToLocalChecked(),
               Nan::New(jsName::jsClassName()).ToLocalChecked());
      returnValue.Reset(theTemplate);
    }
    return returnValue;
  }

 public:
  static v8::Local<v8::Object> New(handleType data) {
    v8::Local<v8::Object> returnValue =
        Nan::NewInstance(
            Nan::GetFunction(Nan::New(theTemplate())).ToLocalChecked())
            .ToLocalChecked();
    Nan::SetInternalFieldPointer(returnValue, 0, data);

    return returnValue;
  }

  // If the object is not of the expected type, or if the pointer inside the
  // object has already been removed, then we must throw an error
  static handleType Resolve(v8::Local<v8::Object> jsObject) {
    handleType returnValue = 0;

    if (Nan::New(theTemplate())->HasInstance(jsObject)) {
      returnValue = (handleType)Nan::GetInternalFieldPointer(jsObject, 0);
    }
    if (!returnValue) {
      Nan::ThrowTypeError(
          (std::string("Object is not of type ") + jsName::jsClassName())
              .c_str());
    }
    return returnValue;
  }
};

class JSOCRequestHandle : public JSHandle<JSOCRequestHandle, OCRequestHandle> {
 public:
  static const char *jsClassName() { return "OCRequestHandle"; }
};

template <typename handleType>
class CallbackInfo {
 public:
  handleType handle;
  Nan::Callback callback;
  Nan::Persistent<v8::Object> jsHandle;
  v8::Local<v8::Object> Init(v8::Local<v8::Object> _jsHandle,
                             v8::Local<v8::Function> jsCallback) {
    callback.Reset(jsCallback);
    jsHandle.Reset(_jsHandle);
    return _jsHandle;
  }
  CallbackInfo() : handle(0) {}
  virtual ~CallbackInfo() {
    if (!jsHandle.IsEmpty()) {
      v8::Local<v8::Object> theObject = Nan::New(jsHandle);
      Nan::SetInternalFieldPointer(theObject, 0, 0);
      Nan::ForceSet(theObject, Nan::New("stale").ToLocalChecked(),
                    Nan::New(true),
                    (v8::PropertyAttribute)(v8::ReadOnly | v8::DontDelete));
      jsHandle.Reset();
    }
  }
};

#define JSCALLBACKHANDLE_RESOLVE(type, info, object, ...) \
  do {                                                    \
    info = type::Resolve((object));                       \
    if (!info) {                                          \
      return __VA_ARGS__;                                 \
    }                                                     \
  } while (0)

class JSOCDoHandle : public JSHandle<JSOCDoHandle, CallbackInfo<OCDoHandle> *> {
 public:
  static const char *jsClassName() { return "OCDoHandle"; }
};

class JSOCResourceHandle
    : public JSHandle<JSOCResourceHandle, CallbackInfo<OCResourceHandle> *> {
 public:
  static const char *jsClassName() { return "OCResourceHandle"; }
  static std::map<OCResourceHandle, Nan::Persistent<v8::Object> *> handles;
};

v8::Local<v8::Array> jsArrayFromBytes(unsigned char *bytes, uint32_t length);

bool fillCArrayFromJSArray(unsigned char *bytes, uint32_t length,
                           v8::Local<v8::Array> array);

#endif /* __IOTIVITY_NODE_HANDLES_H__ */
