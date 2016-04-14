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

  return returnValue;
}

void c_OCDeviceInfoFreeMembers(OCDeviceInfo *info) { free(info->deviceName); }

bool c_OCDeviceInfo(Local<Object> deviceInfo, OCDeviceInfo *info) {
  OCDeviceInfo local = {0};

  VALIDATE_AND_ASSIGN_STRING(&local, deviceName, deviceInfo,
                             c_OCDeviceInfoFreeMembers, false);

  *info = local;
  return true;
}
