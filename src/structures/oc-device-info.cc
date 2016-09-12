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

#include "oc-device-info.h"
#include <nan.h>
#include "../common.h"
#include "string-primitive.h"

extern "C" {
#include <string.h>
}

using namespace v8;

v8::Local<v8::Object> js_OCDeviceInfo(OCDeviceInfo *info) {
  Local<Object> returnValue = Nan::New<Object>();

  SET_STRING_IF_NOT_NULL(returnValue, info, deviceName);
  SET_STRING_IF_NOT_NULL(returnValue, info, specVersion);
  ADD_STRING_ARRAY(returnValue, info, types);
  ADD_STRING_ARRAY(returnValue, info, dataModelVersions);

  return returnValue;
}

void c_freeLinkedList(OCStringLL *list) {
  OCStringLL *item, *nextItem;
  for (item = list; item;) {
    nextItem = item->next;
    free(item->value);
    free(item);
    item = nextItem;
  }
}

void c_OCDeviceInfoFreeMembers(OCDeviceInfo *info) {
  free(info->deviceName);
  free(info->specVersion);
  c_freeLinkedList(info->types);
  c_freeLinkedList(info->dataModelVersions);
}

bool c_StringArrayFromProperty(Local<Object> source, const char *propertyName,
                               OCStringLL **destination) {
  Local<Value> sourceValue =
      Nan::Get(source, Nan::New(propertyName).ToLocalChecked())
          .ToLocalChecked();
  VALIDATE_VALUE_TYPE(sourceValue, IsArray,
                      (std::string("device info ") + propertyName).c_str(),
                      false);
  Local<Array> jsArray = Local<Array>::Cast(sourceValue);

  size_t index, length = jsArray->Length();
  OCStringLL *local = 0, **previous = &local;

  for (index = 0; index < length; index++, previous = &((*previous)->next)) {
    Local<Value> itemValue = Nan::Get(jsArray, index).ToLocalChecked();
    VALIDATE_VALUE_TYPE_OR_FREE(
        itemValue, IsString,
        (std::string("device info ") + propertyName + " list item").c_str(),
        false, local, c_freeLinkedList);

    (*previous) = new OCStringLL;
    if (!(*previous)) {
      goto freeAndQuit;
    }
    (*previous)->next = 0;
    (*previous)->value = strdup((const char *)*(String::Utf8Value(itemValue)));
    if (!(*previous)->value) {
      goto freeAndQuit;
    }
  }

  *destination = local;
  return true;
freeAndQuit:
  c_freeLinkedList(local);
  return false;
}

bool c_OCDeviceInfo(Local<Object> deviceInfo, OCDeviceInfo *info) {
  OCDeviceInfo local = {
      .deviceName = 0, .types = 0, .specVersion = 0, .dataModelVersions = 0};

  VALIDATE_AND_ASSIGN_STRING(&local, deviceName, deviceInfo,
                             c_OCDeviceInfoFreeMembers, false);

  VALIDATE_AND_ASSIGN_STRING(&local, specVersion, deviceInfo,
                             c_OCDeviceInfoFreeMembers, false);

  // Make sure the "types" property is an array and copy it to the C structure
  if (!c_StringArrayFromProperty(deviceInfo, "types", &(local.types))) {
    c_OCDeviceInfoFreeMembers(&local);
    return false;
  }

  // Make sure the "dataModelVersion" property is an array and copy it to the C
  // structure
  if (!c_StringArrayFromProperty(deviceInfo, "dataModelVersions",
                                 &(local.dataModelVersions))) {
    c_OCDeviceInfoFreeMembers(&local);
    return false;
  }

  // If we've successfully created the structure, we transfer it to the
  // passed-in structure
  *info = local;
  return true;
}
