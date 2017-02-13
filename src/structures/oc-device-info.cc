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

extern "C" {
#include <string.h>
}

void delete_OCStringLL(OCStringLL *list) {
  OCStringLL *item, *nextItem;
  for (item = list; item;) {
    nextItem = item->next;
    delete item->value;
    delete item;
    item = nextItem;
  }
}

OCDeviceInfo *new_OCDeviceInfo() {
  OCDeviceInfo *info = new OCDeviceInfo;
  info->deviceName = 0;
  info->specVersion = 0;
  info->types = 0;
  info->dataModelVersions = 0;
  return info;
}

void delete_OCDeviceInfo(OCDeviceInfo *info) {
  delete info->deviceName;
  delete info->specVersion;
  delete_OCStringLL(info->types);
  delete_OCStringLL(info->dataModelVersions);
}

static std::string c_StringArrayFromProperty(napi_env env, napi_value source,
                                             const char *propertyName,
                                             OCStringLL **destination) {
  J2C_GET_PROPERTY_JS_RETURN(sourceValue, env, source, propertyName);
  NAPI_IS_ARRAY_RETURN(env, sourceValue,
                       std::string("device info ") + propertyName);

  uint32_t index, length;
  NAPI_CALL_RETURN(napi_get_array_length(env, sourceValue, &length));

  auto local = std::unique_ptr<OCStringLL *, void (*)(OCStringLL **)>(
      new OCStringLL *(0), [](OCStringLL **ll) {
        delete_OCStringLL(*ll);
        delete ll;
      });
  OCStringLL **previous = local.get();

  for (index = 0; index < length; index++, previous = &((*previous)->next)) {
    napi_value jsItemValue;

    (*previous) = new OCStringLL;
    if (!(*previous)) {
      return LOCAL_MESSAGE("Failed to allocate " + propertyName + " list item");
    }

    NAPI_CALL_RETURN(napi_get_element(env, sourceValue, index, &jsItemValue));
    J2C_GET_STRING_JS_RETURN(env, ((*previous)->value), jsItemValue, false,
                             "device info " + propertyName + " list item");
    (*previous)->next = 0;
  }

  *destination = *(local.get());
  return std::string();
}

std::string c_OCDeviceInfo(
    napi_env env, napi_value deviceInfo,
    std::unique_ptr<OCDeviceInfo, void (*)(OCDeviceInfo *)> &info) {
  J2C_ASSIGN_MEMBER_STRING_RETURN(env, info, deviceInfo, deviceName);
  J2C_ASSIGN_MEMBER_STRING_RETURN(env, info, deviceInfo, specVersion);

  HELPER_CALL_RETURN(
      c_StringArrayFromProperty(env, deviceInfo, "types", &(info->types)));
  HELPER_CALL_RETURN(c_StringArrayFromProperty(
      env, deviceInfo, "dataModelVersions", &(info->dataModelVersions)));

  return std::string();
}
