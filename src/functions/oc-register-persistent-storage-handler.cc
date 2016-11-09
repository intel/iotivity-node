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

// This file makes the assumption that a (Nan::Persistent<Value> *) pointer will
// fit in a (FILE *) pointer

#include <nan.h>

#include "../common.h"
#include "../structures/handles.h"
#include "../structures/oc-payload.h"

extern "C" {
#include <ocpayload.h>
#include <ocstack.h>
}

using namespace v8;
using namespace node;

static Nan::Persistent<Object> *context = 0;
static Nan::Callback *callbackFor_open = 0;
static Nan::Callback *callbackFor_close = 0;
static Nan::Callback *callbackFor_read = 0;
static Nan::Callback *callbackFor_write = 0;
static Nan::Callback *callbackFor_unlink = 0;

// If the JS callback succeeds, create a Nan::Persistent<Value> containing the
// JS file descriptor
static FILE *defaultOpen(const char *path, const char *mode) {
  Nan::Persistent<Value> *fp = 0;
  if (callbackFor_open) {
    Local<Value> arguments[2] = {Nan::New(path).ToLocalChecked(),
                                 Nan::New(mode).ToLocalChecked()};
    Local<Value> returnValue;
    TRY_CALL(callbackFor_open, Nan::New(*context), 2, arguments, returnValue,
             0);
    VALIDATE_VALUE_TYPE(returnValue, IsNumber,
                        "persistent storage open return value", );
    if (Nan::To<double>(returnValue).FromJust() >= 0) {
      fp = new Nan::Persistent<Value>(returnValue);
    }
  }
  return (FILE *)fp;
}

static size_t defaultRead(void *ptr, size_t size, size_t count, FILE *stream) {
  size_t sizeRead = 0;

  if (callbackFor_read) {
    size_t totalSize = size * count;
    Local<Object> buffer = Nan::NewBuffer(totalSize).ToLocalChecked();
    Local<Value> arguments[3] = {
        buffer, Nan::New((double)totalSize),
        Nan::New<Value>(*(Nan::Persistent<Value> *)stream)};
    Local<Value> returnValue;
    TRY_CALL(callbackFor_read, Nan::New(*context), 3, arguments, returnValue,
             0);
    VALIDATE_VALUE_TYPE(returnValue, IsUint32,
                        "persistent storage read return value", );
    sizeRead = Nan::To<uint32_t>(returnValue).FromJust();

    memcpy(ptr, Buffer::Data(buffer), totalSize);
  }

  return sizeRead;
}

static size_t defaultWrite(const void *ptr, size_t size, size_t count,
                           FILE *stream) {
  size_t sizeWritten = 0;

  if (callbackFor_write) {
    size_t totalSize = size * count;
    Local<Object> buffer =
        Nan::CopyBuffer((const char *)ptr, totalSize).ToLocalChecked();
    Local<Value> arguments[3] = {
        buffer, Nan::New((double)totalSize),
        Nan::New<Value>(*(Nan::Persistent<Value> *)stream)};
    Local<Value> returnValue;
    TRY_CALL(callbackFor_write, Nan::New(*context), 3, arguments, returnValue,
             0);
    VALIDATE_VALUE_TYPE(returnValue, IsUint32,
                        "persistent storage write return value", );
    sizeWritten = Nan::To<uint32_t>(returnValue).FromJust();
  }

  return sizeWritten;
}

static int defaultClose(FILE *stream) {
  int returnValue = -1;
  Nan::Persistent<Value> *fp = (Nan::Persistent<Value> *)stream;

  if (callbackFor_close) {
    Local<Value> arguments[1] = {Nan::New<Value>(*fp)};
    Local<Value> jsReturnValue;
    TRY_CALL(callbackFor_close, Nan::New(*context), 1, arguments, jsReturnValue,
             -1);
    VALIDATE_VALUE_TYPE(jsReturnValue, IsUint32,
                        "persistent storage close return value", );
    returnValue = Nan::To<uint32_t>(jsReturnValue).FromJust();
    if (returnValue == 0) {
      delete fp;
    }
  }

  return returnValue;
}

static int defaultUnlink(const char *path) {
  int returnValue = -1;

  if (callbackFor_unlink) {
    Local<Value> arguments[1] = {Nan::New(path).ToLocalChecked()};
    Local<Value> jsReturnValue;
    TRY_CALL(callbackFor_unlink, Nan::New(*context), 1, arguments,
             jsReturnValue, -1);
    VALIDATE_VALUE_TYPE(jsReturnValue, IsUint32,
                        "persistent storage close return value", );
    returnValue = Nan::To<uint32_t>(jsReturnValue).FromJust();
  }

  return returnValue;
}

static OCPersistentStorage psCallbacks = {
    defaultOpen, defaultRead, defaultWrite, defaultClose, defaultUnlink};

#define VALIDATE_MEMBER(jsObject, memberName)                                 \
  if (!((Nan::Get(jsCallbacks, Nan::New("open").ToLocalChecked())             \
             .ToLocalChecked())                                               \
            ->IsFunction())) {                                                \
    return Nan::ThrowTypeError("Persistent storage callback for " #memberName \
                               " must be a function");                        \
  }

#define ASSIGN_CALLBACK(source, name)                           \
  if (callbackFor_##name) {                                     \
    delete callbackFor_##name;                                  \
  }                                                             \
  callbackFor_##name = new Nan::Callback(Local<Function>::Cast( \
      Nan::Get((source), Nan::New(#name).ToLocalChecked()).ToLocalChecked()))

NAN_METHOD(bind_OCRegisterPersistentStorageHandler) {
  VALIDATE_ARGUMENT_COUNT(info, 1);
  VALIDATE_ARGUMENT_TYPE(info, 0, IsObject);

  Local<Object> jsCallbacks = Local<Object>::Cast(info[0]);

  VALIDATE_MEMBER(jsCallbacks, "open");
  VALIDATE_MEMBER(jsCallbacks, "close");
  VALIDATE_MEMBER(jsCallbacks, "read");
  VALIDATE_MEMBER(jsCallbacks, "write");
  VALIDATE_MEMBER(jsCallbacks, "unlink");

  OCStackResult result = OCRegisterPersistentStorageHandler(&psCallbacks);

  if (result == OC_STACK_OK) {
    ASSIGN_CALLBACK(jsCallbacks, open);
    ASSIGN_CALLBACK(jsCallbacks, close);
    ASSIGN_CALLBACK(jsCallbacks, read);
    ASSIGN_CALLBACK(jsCallbacks, write);
    ASSIGN_CALLBACK(jsCallbacks, unlink);
    if (context) {
      delete context;
    }
    context = new Nan::Persistent<Object>(jsCallbacks);
  }

  info.GetReturnValue().Set(Nan::New((int)result));
}
