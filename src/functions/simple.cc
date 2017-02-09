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

extern "C" {
#include <ocstack.h>
}

NAPI_METHOD(bind_OCStop) {
  SET_RETURN_VALUE(env, info, number, ((double)OCStop()));
}

NAPI_METHOD(bind_OCProcess) {
  SET_RETURN_VALUE(env, info, number, ((double)OCProcess()));
}

NAPI_METHOD(bind_OCStartPresence) {
  DECLARE_ARGUMENTS(env, info, 1);
  VALIDATE_VALUE_TYPE_THROW(env, arguments[0], napi_number, false, "arg 1");

  uint32_t timeToLive;
  NAPI_CALL_THROW(env, napi_get_value_uint32(env, arguments[0], &timeToLive));

  SET_RETURN_VALUE(env, info, number, ((double)OCStartPresence(timeToLive)));
}

NAPI_METHOD(bind_OCStopPresence) {
  SET_RETURN_VALUE(env, info, number, ((double)OCStopPresence()));
}

NAPI_METHOD(bind_OCInit) {
  DECLARE_ARGUMENTS(env, info, 3);
  VALIDATE_VALUE_TYPE_THROW(env, arguments[0], napi_string, true, "arg 1");
  VALIDATE_VALUE_TYPE_THROW(env, arguments[1], napi_number, false, "arg 2");
  VALIDATE_VALUE_TYPE_THROW(env, arguments[2], napi_number, false, "arg 3");

  int ipAddressLength;
  NAPI_CALL_THROW(env, napi_get_value_string_utf8_length(env, arguments[0],
                                                         &ipAddressLength));
  char ipAddress[ipAddressLength + 1];
  int ipAddressWritten;
  NAPI_CALL_THROW(
      env, napi_get_value_string_utf8(env, arguments[0], ipAddress,
                                      ipAddressLength, &ipAddressWritten));

  uint32_t port;
  NAPI_CALL_THROW(env, napi_get_value_uint32(env, arguments[1], &port));

  uint32_t mode;
  NAPI_CALL_THROW(env, napi_get_value_uint32(env, arguments[2], &mode));

  SET_RETURN_VALUE(env, info, number,
                   ((double)OCInit(ipAddress, (uint16_t)port, (OCMode)mode)));
}

NAPI_METHOD(bind_OCGetNumberOfResources) {
  DECLARE_ARGUMENTS(env, info, 1);
  VALIDATE_VALUE_TYPE_THROW(env, arguments[0], napi_object, false, "arg 1");

  OCStackResult result;
  uint8_t resourceCount;
  result = OCGetNumberOfResources(&resourceCount);

  if (result == OC_STACK_OK) {
    SET_PROPERTY_THROW(env, arguments[0], "count", number,
                       ((double)resourceCount));
  }

  SET_RETURN_VALUE(env, info, number, ((double)result));
}
