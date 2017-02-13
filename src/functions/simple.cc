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

#include "../common.h"
#include "../structures/oc-device-info.h"

extern "C" {
#include <ocstack.h>
}

NAPI_METHOD(bind_OCStop) {
  C2J_SET_RETURN_VALUE(env, info, number, ((double)OCStop()));
}

NAPI_METHOD(bind_OCProcess) {
  C2J_SET_RETURN_VALUE(env, info, number, ((double)OCProcess()));
}

NAPI_METHOD(bind_OCStartPresence) {
  J2C_GET_ARGUMENTS(env, info, 1);

  J2C_GET_VALUE_JS_THROW(uint32_t, ttl, env, arguments[0], napi_number, "ttl", uint32, uint32_t);

  C2J_SET_RETURN_VALUE(env, info, number, ((double)OCStartPresence(ttl)));
}

NAPI_METHOD(bind_OCStopPresence) {
  C2J_SET_RETURN_VALUE(env, info, number, ((double)OCStopPresence()));
}

NAPI_METHOD(bind_OCSetDeviceInfo) {
  J2C_GET_ARGUMENTS(env, info, 1);
  J2C_VALIDATE_VALUE_TYPE_THROW(env, arguments[0], napi_object, "arg 1");

  auto devInfo = std::unique_ptr<OCDeviceInfo, void (*)(OCDeviceInfo *)>(
      new_OCDeviceInfo(), delete_OCDeviceInfo);
  HELPER_CALL_THROW(env, c_OCDeviceInfo(env, arguments[0], devInfo));

  C2J_SET_RETURN_VALUE(env, info, number,
                       ((double)OCSetDeviceInfo(*(devInfo.get()))));
}

NAPI_METHOD(bind_OCInit) {
  J2C_GET_ARGUMENTS(env, info, 3);

  std::unique_ptr<char> ipAddress_tracker;
  char *ipAddress;
  J2C_GET_STRING_JS_THROW(env, ipAddress, arguments[0], true, "address");
  ipAddress_tracker.reset(ipAddress);

  J2C_GET_VALUE_JS_THROW(uint16_t, port, env, arguments[1], napi_number, "port", uint32, uint32_t);

  J2C_GET_VALUE_JS_THROW(OCMode, mode, env, arguments[2], napi_number, "mode", uint32, uint32_t);

  C2J_SET_RETURN_VALUE(env, info, number, ((double)OCInit(ipAddress, port, mode)));
}

NAPI_METHOD(bind_OCGetNumberOfResources) {
  J2C_GET_ARGUMENTS(env, info, 1);
  J2C_VALIDATE_VALUE_TYPE_THROW(env, arguments[0], napi_object, "arg 1");

  OCStackResult result;
  uint8_t resourceCount;
  result = OCGetNumberOfResources(&resourceCount);

  if (result == OC_STACK_OK) {
    C2J_SET_PROPERTY_THROW(env, arguments[0], "count", number,
                           ((double)resourceCount));
  }

  C2J_SET_RETURN_VALUE(env, info, number, ((double)result));
}
