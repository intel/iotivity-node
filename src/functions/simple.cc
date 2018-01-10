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

#include <node_api.h>
#include <string>
#include "../common.h"

extern "C" {
#include <ocstack.h>
}

std::string getDateCtor(napi_env env, napi_value *result) {
  napi_value global;
  NAPI_CALL_RETURN(env, napi_get_global(env, &global));
  NAPI_CALL_RETURN(env, napi_get_named_property(env, global, "Date", result));

  return std::string();
}

// From
// http://stackoverflow.com/questions/34158318/are-there-some-v8-functions-to-create-a-c-v8date-object-from-a-string-like#answer-36200373
std::string dateFromString(napi_env env, const char *str, napi_value *result) {
  napi_value dateCtor, string;
  HELPER_CALL_RETURN(getDateCtor(env, &dateCtor));
  NAPI_CALL_RETURN(env, napi_create_string_utf8(env, str, -1, &string));
  NAPI_CALL_RETURN(env, napi_new_instance(env, dateCtor, 1, &string, result));

  return std::string();
}

napi_value bind_OCInit(napi_env env, napi_callback_info info) {
  J2C_DECLARE_ARGUMENTS(env, info, 3);
  J2C_GET_STRING_TRACKED_JS_THROW(ip, env, arguments[0], true, "address");
  J2C_DECLARE_VALUE_JS_THROW(uint16_t, port, env, arguments[1], napi_number,
                             "port", uint32, uint32_t);
  J2C_DECLARE_VALUE_JS_THROW(OCMode, mode, env, arguments[2], napi_number,
                             "mode", uint32, uint32_t);
  C2J_SET_RETURN_VALUE(env, info, double, ((double)OCInit(ip, port, mode)));
}

napi_value bind_OCStop(napi_env env, napi_callback_info info) {
  C2J_SET_RETURN_VALUE(env, info, double, ((double)OCStop()));
}

napi_value bind_OCProcess(napi_env env, napi_callback_info info) {
  C2J_SET_RETURN_VALUE(env, info, double, ((double)OCProcess()));
}

napi_value bind_OCStartPresence(napi_env env, napi_callback_info info) {
  J2C_DECLARE_ARGUMENTS(env, info, 1);

  J2C_DECLARE_VALUE_JS_THROW(uint32_t, ttl, env, arguments[0], napi_number,
                             "ttl", uint32, uint32_t);

  C2J_SET_RETURN_VALUE(env, info, double, ((double)OCStartPresence(ttl)));
}

napi_value bind_OCStopPresence(napi_env env, napi_callback_info info) {
  C2J_SET_RETURN_VALUE(env, info, double, ((double)OCStopPresence()));
}

napi_value bind_OCGetNumberOfResources(napi_env env, napi_callback_info info) {
  J2C_DECLARE_ARGUMENTS(env, info, 1);
  J2C_VALIDATE_VALUE_TYPE_THROW(env, arguments[0], napi_object, "receptacle");

  uint8_t resourceCount;
  OCStackResult result = OCGetNumberOfResources(&resourceCount);

  if (result == OC_STACK_OK) {
    C2J_SET_PROPERTY_THROW(env, arguments[0], "count", double,
                           ((double)resourceCount));
  }

  C2J_SET_RETURN_VALUE(env, info, double, ((double)result));
}

napi_value bind_OCGetServerInstanceIDString(napi_env env,
                                            napi_callback_info info) {
  const char *uuid = OCGetServerInstanceIDString();
  C2J_SET_RETURN_VALUE(env, info, string_utf8, uuid, strlen(uuid));
}

napi_value bind_OCGetPropertyValue(napi_env env, napi_callback_info info) {
  J2C_DECLARE_ARGUMENTS(env, info, 3);
  J2C_DECLARE_VALUE_JS_THROW(OCPayloadType, payloadType, env, arguments[0],
                             napi_number, "payload type", uint32, uint32_t);
  J2C_GET_STRING_TRACKED_JS_THROW(prop_name, env, arguments[1], true,
                                  "property name");
  J2C_VALIDATE_VALUE_TYPE_THROW(env, arguments[2], napi_object, "receptacle");

  OCStackResult returnValue;
  void *result = nullptr;
  napi_value jsResult = nullptr;

  returnValue = OCGetPropertyValue(payloadType, prop_name, &result);

  if (returnValue == OC_STACK_OK) {
    // string conditions
    if ((payloadType == PAYLOAD_TYPE_DEVICE &&
         !strcmp(prop_name, OC_RSRVD_SPEC_VERSION)) ||
        (payloadType == PAYLOAD_TYPE_DEVICE &&
         !strcmp(prop_name, OC_RSRVD_DEVICE_NAME)) ||
        (payloadType == PAYLOAD_TYPE_PLATFORM &&
         !strcmp(prop_name, OC_RSRVD_MFG_URL)) ||
        (payloadType == PAYLOAD_TYPE_PLATFORM &&
         !strcmp(prop_name, OC_RSRVD_MFG_NAME))) {
      NAPI_CALL_THROW(env, napi_create_string_utf8(env, (const char *)result,
                                                   -1, &jsResult));

      // string list conditions
    } else if (payloadType == PAYLOAD_TYPE_DEVICE &&
               !strcmp(prop_name, OC_RSRVD_DATA_MODEL_VERSION)) {
      HELPER_CALL_THROW(env,
                        js_StringArray(env, (OCStringLL *)result, &jsResult));

      // date conditions
    } else if ((payloadType == PAYLOAD_TYPE_PLATFORM &&
                !strcmp(prop_name, OC_RSRVD_MFG_DATE)) ||
               (payloadType == PAYLOAD_TYPE_PLATFORM &&
                !strcmp(prop_name, OC_RSRVD_SYSTEM_TIME))) {
      HELPER_CALL_THROW(env,
                        dateFromString(env, (const char *)result, &jsResult));
    }

    NAPI_CALL_THROW(
        env, napi_set_named_property(env, arguments[2], "value", jsResult));
  }

  C2J_SET_RETURN_VALUE(env, info, double, ((double)returnValue));
}

napi_value bind_OCSetPropertyValue(napi_env env, napi_callback_info info) {
  J2C_DECLARE_ARGUMENTS(env, info, 3);
  J2C_DECLARE_VALUE_JS_THROW(OCPayloadType, payloadType, env, arguments[0],
                             napi_number, "payload type", uint32, uint32_t);
  J2C_GET_STRING_TRACKED_JS_THROW(prop_name, env, arguments[1], true,
                                  "property name");

  napi_valuetype prop_type = napi_undefined;
  NAPI_CALL_THROW(env, napi_typeof(env, arguments[2], &prop_type));
  if (prop_type != napi_string) {
    bool is_date = false;
    napi_value dateCtor;
    HELPER_CALL_THROW(env, getDateCtor(env, &dateCtor));
    NAPI_CALL_THROW(env,
                    napi_instanceof(env, arguments[2], dateCtor, &is_date));
    JS_ASSERT(is_date == true,
              std::string("Property value ") + std::string(prop_name) +
                  std::string(" must be a string or a date"),
              THROW_BODY(env, nullptr));
    NAPI_CALL_THROW(env,
                    napi_coerce_to_string(env, arguments[2], &arguments[2]));
  }

  J2C_GET_STRING_TRACKED_JS_THROW(prop_value, env, arguments[2], true,
                                  "property value");
  OCStackResult returnValue =
      OCSetPropertyValue(payloadType, prop_name, (const void *)prop_value);

  C2J_SET_RETURN_VALUE(env, info, double, ((double)returnValue));
}
