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
#include "oc-device-info.h"
#include "string-primitive.h"
#include "../common.h"

extern "C" {
#include <string.h>
}

using namespace v8;

v8::Local<v8::Object> js_OCDeviceInfo(OCDeviceInfo *info) {
  Local<Object> returnValue = Nan::New<Object>();

  SET_STRING_IF_NOT_NULL(returnValue, info, deviceName);
  ADD_STRING_ARRAY(returnValue, info, types);

  return returnValue;
}

void c_OCDeviceInfoFreeMembers(OCDeviceInfo *info) {
  OCStringLL *item, *nextItem;

  free(info->deviceName);
  for (item = info->types; item;) {
    nextItem = item->next;
    free(item->value);
    free(item);
    item = nextItem;
  }
}

bool c_OCDeviceInfo(Local<Object> deviceInfo, OCDeviceInfo *info) {
  OCDeviceInfo local = {0, 0};
  OCStringLL *newType = 0;

  VALIDATE_AND_ASSIGN_STRING(&local, deviceName, deviceInfo,
                             c_OCDeviceInfoFreeMembers, false);

  // Make sure the "types" property is an array
  Local<Value> typesValue =
      Nan::Get(deviceInfo, Nan::New("types").ToLocalChecked()).ToLocalChecked();
  VALIDATE_VALUE_TYPE_OR_FREE(typesValue, IsArray, "device info types list",
                              false, &local, c_OCDeviceInfoFreeMembers);
  Local<Array> jsTypes = Local<Array>::Cast(typesValue);
  size_t index, length = jsTypes->Length();

  for (index = 0; index < length; index++) {
    // Make sure an individual type is a string
    Local<Value> itemValue = Nan::Get(jsTypes, index).ToLocalChecked();
    VALIDATE_VALUE_TYPE_OR_FREE(itemValue, IsString,
                                "device info types list item", false, &local,
                                c_OCDeviceInfoFreeMembers);

    // Copy the string to the C linked list
    newType = new OCStringLL;
    if (newType) {
      newType->next = local.types;
      local.types = newType;
      newType->value = strdup((const char *)*(String::Utf8Value(itemValue)));
      if (newType->value) {
        // If copying succeeds we move on to the next type in the list
        continue;
      }
    }

    // If copying fails we bail
    Nan::ThrowError("Failed to allocate device info types list item");
    c_OCDeviceInfoFreeMembers(&local);
    return false;
  }

  // If we've successfully created the structure, we transfer it to the
  // passed-in structure
  *info = local;
  return true;
}
