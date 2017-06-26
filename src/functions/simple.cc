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
#include <string>
#include "../common.h"

extern "C" {
#include <ocstack.h>
}

using namespace v8;

// From
// http://stackoverflow.com/questions/34158318/are-there-some-v8-functions-to-create-a-c-v8date-object-from-a-string-like#answer-36200373
Local<Date> dateFromString(const char *str) {
  static Nan::Callback dateConstructor;
  if (dateConstructor.IsEmpty()) {
    Local<Date> date = Nan::New<Date>(0).ToLocalChecked();
    dateConstructor.Reset(Local<Function>::Cast(
        Nan::Get(date, Nan::New("constructor").ToLocalChecked())
            .ToLocalChecked()));
  }
  Local<Value> jsString = Nan::New(str).ToLocalChecked();
  return Local<Date>::Cast(
      Nan::NewInstance(*dateConstructor, 1, &jsString).ToLocalChecked());
}

NAN_METHOD(bind_OCInit) {
  VALIDATE_ARGUMENT_COUNT(info, 3);
  VALIDATE_ARGUMENT_TYPE_OR_NULL(info, 0, IsString);
  VALIDATE_ARGUMENT_TYPE(info, 1, IsUint32);
  VALIDATE_ARGUMENT_TYPE(info, 2, IsUint32);

  info.GetReturnValue().Set(Nan::New(OCInit(
      (const char *)(info[0]->IsString() ? (*String::Utf8Value(info[0])) : 0),
      (uint16_t)Nan::To<uint32_t>(info[1]).FromJust(),
      (OCMode)Nan::To<uint32_t>(info[2]).FromJust())));
}

NAN_METHOD(bind_OCStop) { info.GetReturnValue().Set(Nan::New(OCStop())); }

NAN_METHOD(bind_OCProcess) { info.GetReturnValue().Set(Nan::New(OCProcess())); }

NAN_METHOD(bind_OCStartPresence) {
  VALIDATE_ARGUMENT_COUNT(info, 1);
  VALIDATE_ARGUMENT_TYPE(info, 0, IsUint32);

  info.GetReturnValue().Set(Nan::New(
      OCStartPresence((uint32_t)Nan::To<uint32_t>(info[0]).FromJust())));
}

NAN_METHOD(bind_OCStopPresence) {
  info.GetReturnValue().Set(Nan::New(OCStopPresence()));
}

NAN_METHOD(bind_OCGetNumberOfResources) {
  VALIDATE_ARGUMENT_COUNT(info, 1);
  VALIDATE_ARGUMENT_TYPE(info, 0, IsObject);

  OCStackResult result;
  uint8_t resourceCount = 0;

  result = OCGetNumberOfResources(&resourceCount);

  if (result == OC_STACK_OK) {
    Nan::Set(Nan::To<Object>(info[0]).ToLocalChecked(),
             Nan::New("count").ToLocalChecked(), Nan::New(resourceCount));
  }

  info.GetReturnValue().Set(Nan::New(result));
}

NAN_METHOD(bind_OCGetServerInstanceIDString) {
  VALIDATE_ARGUMENT_COUNT(info, 0);

  const char *idString = OCGetServerInstanceIDString();

  info.GetReturnValue().Set(idString ? (Nan::New(idString).ToLocalChecked())
                                     : Nan::EmptyString());
}

NAN_METHOD(bind_OCGetPropertyValue) {
  VALIDATE_ARGUMENT_COUNT(info, 3);
  VALIDATE_ARGUMENT_TYPE(info, 0, IsUint32);
  VALIDATE_ARGUMENT_TYPE(info, 1, IsString);
  VALIDATE_ARGUMENT_TYPE(info, 2, IsObject);

  String::Utf8Value propName(info[1]);
  OCPayloadType payloadType =
      (OCPayloadType)Nan::To<uint32_t>(info[0]).FromJust();
  OCStackResult returnValue;
  void *result = nullptr;

  Local<Value> jsResult;

  returnValue =
      OCGetPropertyValue(payloadType, (const char *)*propName, &result);

  if (returnValue == OC_STACK_OK) {
    // string conditions
    if ((payloadType == PAYLOAD_TYPE_DEVICE &&
         !strcmp(*propName, OC_RSRVD_SPEC_VERSION)) ||
        (payloadType == PAYLOAD_TYPE_DEVICE &&
         !strcmp(*propName, OC_RSRVD_DEVICE_NAME)) ||
        (payloadType == PAYLOAD_TYPE_PLATFORM &&
         !strcmp(*propName, OC_RSRVD_MFG_URL)) ||
        (payloadType == PAYLOAD_TYPE_PLATFORM &&
         !strcmp(*propName, OC_RSRVD_MFG_NAME))) {
      jsResult = Nan::New((char *)result).ToLocalChecked();

      // string list conditions
    } else if (payloadType == PAYLOAD_TYPE_DEVICE &&
               !strcmp(*propName, OC_RSRVD_DATA_MODEL_VERSION)) {
      jsResult = js_StringArray((OCStringLL *)result);

      // date conditions
    } else if ((payloadType == PAYLOAD_TYPE_PLATFORM &&
                !strcmp(*propName, OC_RSRVD_MFG_DATE)) ||
               (payloadType == PAYLOAD_TYPE_PLATFORM &&
                !strcmp(*propName, OC_RSRVD_SYSTEM_TIME))) {
      jsResult = dateFromString((const char *)result);
    }

    Nan::Set(Local<Object>::Cast(info[2]), Nan::New("value").ToLocalChecked(),
             jsResult);
  }
  info.GetReturnValue().Set(Nan::New(returnValue));
}

NAN_METHOD(bind_OCSetPropertyValue) {
  VALIDATE_ARGUMENT_COUNT(info, 3);
  VALIDATE_ARGUMENT_TYPE(info, 0, IsUint32);
  VALIDATE_ARGUMENT_TYPE(info, 1, IsString);
  if (!(info[2]->IsString() || info[2]->IsArray() || info[2]->IsDate())) {
    return Nan::ThrowTypeError(
        (std::string("Property value ") +
         std::string((const char *)*String::Utf8Value(info[1])) +
         std::string(" must be a string, an array, or a date"))
            .c_str());
  }

  OCStackResult returnValue;

  if (info[2]->IsString()) {
    returnValue =
        OCSetPropertyValue((OCPayloadType)Nan::To<uint32_t>(info[0]).FromJust(),
                           (const char *)*String::Utf8Value(info[1]),
                           (const void *)*String::Utf8Value(info[2]));
  } else if (info[2]->IsArray()) {
    OCStringLL *start = 0, *current = 0, *previous = 0;
    Local<Array> jsList = Local<Array>::Cast(info[2]);
    size_t index, length = jsList->Length();
    for (index = 0; index < length; index++) {
      current = new OCStringLL;
      current->value = strdup((const char *)*String::Utf8Value(
          Nan::Get(jsList, index).ToLocalChecked()));
      current->next = 0;
      if (previous) {
        previous->next = current;
      }
      previous = current;
      if (!start) {
        start = previous;
      }
    }
    returnValue = OCSetPropertyValue(
        (OCPayloadType)Nan::To<uint32_t>(info[0]).FromJust(),
        (const char *)*String::Utf8Value(info[1]), (const void *)start);

    for (current = start; start; current = start) {
      free(current->value);
      start = current->next;
      delete current;
    }
  } else {
    returnValue =
        OCSetPropertyValue((OCPayloadType)Nan::To<uint32_t>(info[0]).FromJust(),
                           (const char *)*String::Utf8Value(info[1]),
                           (const char *)*String::Utf8Value(
                               Nan::To<String>(info[2]).ToLocalChecked()));
  }
  info.GetReturnValue().Set(Nan::New(returnValue));
}
