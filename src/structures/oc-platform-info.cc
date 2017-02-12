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

extern "C" {
#include <string.h>
}

std::string js_OCPlatformInfo(napi_env env, OCPlatformInfo *info,
                              napi_value *destination) {
  napi_value jsInfo;
  NAPI_CALL_RETURN(napi_create_object(env, &jsInfo));
  C2J_SET_STRING_IF_NOT_NULL_RETURN(env, jsInfo, info, platformID);
  C2J_SET_STRING_IF_NOT_NULL_RETURN(env, jsInfo, info, manufacturerName);
  C2J_SET_STRING_IF_NOT_NULL_RETURN(env, jsInfo, info, manufacturerUrl);
  C2J_SET_STRING_IF_NOT_NULL_RETURN(env, jsInfo, info, modelNumber);
  C2J_SET_STRING_IF_NOT_NULL_RETURN(env, jsInfo, info, dateOfManufacture);
  C2J_SET_STRING_IF_NOT_NULL_RETURN(env, jsInfo, info, platformVersion);
  C2J_SET_STRING_IF_NOT_NULL_RETURN(env, jsInfo, info, operatingSystemVersion);
  C2J_SET_STRING_IF_NOT_NULL_RETURN(env, jsInfo, info, hardwareVersion);
  C2J_SET_STRING_IF_NOT_NULL_RETURN(env, jsInfo, info, firmwareVersion);
  C2J_SET_STRING_IF_NOT_NULL_RETURN(env, jsInfo, info, supportUrl);
  C2J_SET_STRING_IF_NOT_NULL_RETURN(env, jsInfo, info, systemTime);
  *destination = jsInfo;
  return std::string();
}

OCPlatformInfo *new_OCPlatformInfo() {
  OCPlatformInfo *info = new OCPlatformInfo;
  info->platformID = 0;
  info->manufacturerName = 0;
  info->manufacturerUrl = 0;
  info->modelNumber = 0;
  info->dateOfManufacture = 0;
  info->platformVersion = 0;
  info->operatingSystemVersion = 0;
  info->hardwareVersion = 0;
  info->firmwareVersion = 0;
  info->supportUrl = 0;
  info->systemTime = 0;
  return info;
}

void delete_OCPlatformInfo(OCPlatformInfo *info) {
  delete info->platformID;
  delete info->manufacturerName;
  delete info->manufacturerUrl;
  delete info->modelNumber;
  delete info->dateOfManufacture;
  delete info->platformVersion;
  delete info->operatingSystemVersion;
  delete info->hardwareVersion;
  delete info->firmwareVersion;
  delete info->supportUrl;
  delete info->systemTime;
  delete info;
}

std::string c_OCPlatformInfo(
    napi_env env, napi_value source,
    std::unique_ptr<OCPlatformInfo, void (*)(OCPlatformInfo *)> &destination) {
  J2C_ASSIGN_MEMBER_RETURN(env, destination, source, platformID);
  J2C_ASSIGN_MEMBER_RETURN(env, destination, source, manufacturerName);
  J2C_ASSIGN_MEMBER_RETURN(env, destination, source, manufacturerUrl);
  J2C_ASSIGN_MEMBER_RETURN(env, destination, source, modelNumber);
  J2C_ASSIGN_MEMBER_RETURN(env, destination, source, dateOfManufacture);
  J2C_ASSIGN_MEMBER_RETURN(env, destination, source, platformVersion);
  J2C_ASSIGN_MEMBER_RETURN(env, destination, source, operatingSystemVersion);
  J2C_ASSIGN_MEMBER_RETURN(env, destination, source, hardwareVersion);
  J2C_ASSIGN_MEMBER_RETURN(env, destination, source, firmwareVersion);
  J2C_ASSIGN_MEMBER_RETURN(env, destination, source, supportUrl);
  J2C_ASSIGN_MEMBER_RETURN(env, destination, source, systemTime);
  return std::string();
}
