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

#include "oc-platform-info.h"
#include <nan.h>
#include "../common.h"

extern "C" {
#include <string.h>
}

using namespace v8;

v8::Local<v8::Object> js_OCPlatformInfo(OCPlatformInfo *info) {
  Local<Object> returnValue = Nan::New<Object>();

  SET_STRING_IF_NOT_NULL(returnValue, info, platformID);
  SET_STRING_IF_NOT_NULL(returnValue, info, manufacturerName);
  SET_STRING_IF_NOT_NULL(returnValue, info, manufacturerUrl);
  SET_STRING_IF_NOT_NULL(returnValue, info, modelNumber);
  SET_STRING_IF_NOT_NULL(returnValue, info, dateOfManufacture);
  SET_STRING_IF_NOT_NULL(returnValue, info, platformVersion);
  SET_STRING_IF_NOT_NULL(returnValue, info, operatingSystemVersion);
  SET_STRING_IF_NOT_NULL(returnValue, info, hardwareVersion);
  SET_STRING_IF_NOT_NULL(returnValue, info, firmwareVersion);
  SET_STRING_IF_NOT_NULL(returnValue, info, supportUrl);
  SET_STRING_IF_NOT_NULL(returnValue, info, systemTime);

  return returnValue;
}

void c_OCPlatformInfoFreeMembers(OCPlatformInfo *info) {
  free(info->platformID);
  free(info->manufacturerName);
  free(info->manufacturerUrl);
  free(info->modelNumber);
  free(info->dateOfManufacture);
  free(info->platformVersion);
  free(info->operatingSystemVersion);
  free(info->hardwareVersion);
  free(info->firmwareVersion);
  free(info->supportUrl);
  free(info->systemTime);
}

bool c_OCPlatformInfo(Local<Object> platformInfo, OCPlatformInfo *info) {
  OCPlatformInfo local = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

  VALIDATE_AND_ASSIGN_STRING(&local, platformID, platformInfo,
                             c_OCPlatformInfoFreeMembers, false);
  VALIDATE_AND_ASSIGN_STRING(&local, manufacturerName, platformInfo,
                             c_OCPlatformInfoFreeMembers, false);
  VALIDATE_AND_ASSIGN_STRING(&local, manufacturerUrl, platformInfo,
                             c_OCPlatformInfoFreeMembers, false);
  VALIDATE_AND_ASSIGN_STRING(&local, modelNumber, platformInfo,
                             c_OCPlatformInfoFreeMembers, false);
  VALIDATE_AND_ASSIGN_STRING(&local, dateOfManufacture, platformInfo,
                             c_OCPlatformInfoFreeMembers, false);
  VALIDATE_AND_ASSIGN_STRING(&local, platformVersion, platformInfo,
                             c_OCPlatformInfoFreeMembers, false);
  VALIDATE_AND_ASSIGN_STRING(&local, operatingSystemVersion, platformInfo,
                             c_OCPlatformInfoFreeMembers, false);
  VALIDATE_AND_ASSIGN_STRING(&local, hardwareVersion, platformInfo,
                             c_OCPlatformInfoFreeMembers, false);
  VALIDATE_AND_ASSIGN_STRING(&local, firmwareVersion, platformInfo,
                             c_OCPlatformInfoFreeMembers, false);
  VALIDATE_AND_ASSIGN_STRING(&local, supportUrl, platformInfo,
                             c_OCPlatformInfoFreeMembers, false);
  VALIDATE_AND_ASSIGN_STRING(&local, systemTime, platformInfo,
                             c_OCPlatformInfoFreeMembers, false);

  *info = local;
  return true;
}
