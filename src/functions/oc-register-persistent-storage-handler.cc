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

#include <limits.h>

#define THROW_IF_EXCEEDS_UINT32_MAX(prefix, sizeAsSizeT, returnValue) \
  if ((sizeAsSizeT) > UINT32_MAX) {                                   \
    Nan::ThrowRangeError(prefix ": request exceeds UINT32_MAX");      \
    return (returnValue);                                             \
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
  double result;
  if (callbackFor_open) {
    CALL_JS(callbackFor_open, Nan::New(*context), 2, 0, IsNumber,
            "persistent storage open return value",
            result = Nan::To<double>(jsReturn).FromJust(),
            Nan::New(path).ToLocalChecked(), Nan::New(mode).ToLocalChecked());
    if (result >= 0) {
      return ((FILE *)(new double(result)));
    }
  }
  return 0;
}

static size_t defaultRead(void *ptr, size_t size, size_t count, FILE *stream) {
  size_t sizeRead = 0;

  if (callbackFor_read) {
    size_t totalSize = size * count;
    THROW_IF_EXCEEDS_UINT32_MAX("defaultRead", totalSize, -1);
    Local<Object> buffer = Nan::NewBuffer((uint32_t)totalSize).ToLocalChecked();

    CALL_JS(callbackFor_read, Nan::New(*context), 3, 0, IsUint32,
            "persistent storage read return value",
            sizeRead = Nan::To<uint32_t>(jsReturn).FromJust(), buffer,
            Nan::New((double)totalSize), Nan::New(*((double *)stream)));

    memcpy(ptr, Buffer::Data(buffer), sizeRead);
  }

  return sizeRead;
}

static size_t defaultWrite(const void *ptr, size_t size, size_t count,
                           FILE *stream) {
  size_t sizeWritten = 0;

  if (callbackFor_write) {
    size_t totalSize = size * count;
    THROW_IF_EXCEEDS_UINT32_MAX("defaultWrite", totalSize, -1);
    Local<Object> buffer =
        Nan::CopyBuffer((const char *)ptr, (uint32_t)totalSize)
            .ToLocalChecked();

    CALL_JS(callbackFor_write, Nan::New(*context), 3, 0, IsUint32,
            "persistent storage write return value",
            sizeWritten = Nan::To<uint32_t>(jsReturn).FromJust(), buffer,
            Nan::New((double)totalSize), Nan::New(*((double *)stream)));
  }

  return sizeWritten;
}

static int defaultClose(FILE *stream) {
  int returnValue = -1;

  if (callbackFor_close) {
    CALL_JS(callbackFor_close, Nan::New(*context), 1, -1, IsInt32,
            "persistent storage close return value",
            returnValue = Nan::To<int>(jsReturn).FromJust(),
            Nan::New(*((double *)stream)));
    if (returnValue == 0) {
      delete ((double *)stream);
    }
  }

  return returnValue;
}

static int defaultUnlink(const char *path) {
  int returnValue = -1;

  if (callbackFor_unlink) {
    CALL_JS(callbackFor_unlink, Nan::New(*context), 1, -1, IsInt32,
            "persistent storage close return value",
            returnValue = Nan::To<int>(jsReturn).FromJust(),
            Nan::New(path).ToLocalChecked());
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
