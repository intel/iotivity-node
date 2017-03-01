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

#include "oc-payload.h"
#include "oc-payload-macros.h"
#include "oc-platform-info.h"
#include "oc-rep-payload/to-c.h"
#include "oc-rep-payload/to-js.h"

extern "C" {
#include <ocpayload.h>
#include <string.h>
}

std::string js_OCPresencePayload(napi_env env, OCPresencePayload *payload,
                                 napi_value destination) {
  C2J_SET_NUMBER_MEMBER_RETURN(env, destination, payload, sequenceNumber);
  C2J_SET_NUMBER_MEMBER_RETURN(env, destination, payload, maxAge);
  C2J_SET_NUMBER_MEMBER_RETURN(env, destination, payload, trigger);
  C2J_SET_STRING_IF_NOT_NULL_RETURN(env, destination, payload, resourceType);
  return std::string();
}

static std::string js_OCResourcePayload(napi_env env,
                                        OCResourcePayload *payload,
                                        napi_value *destination) {
  NAPI_CALL_RETURN(napi_create_object(env, destination));
  C2J_SET_STRING_IF_NOT_NULL_RETURN(env, *destination, payload, uri);

  SET_TYPES_INTERFACES(env, *destination, payload, types, interfaces);

  C2J_SET_NUMBER_MEMBER_RETURN(env, *destination, payload, bitmap);

  C2J_SET_PROPERTY_RETURN(env, *destination, "secure", boolean,
                          payload->secure);

  C2J_SET_NUMBER_MEMBER_RETURN(env, *destination, payload, port);
  return std::string();
}

static std::string js_OCDiscoveryPayload(napi_env env,
                                         OCDiscoveryPayload *payload,
                                         napi_value destination) {
  C2J_SET_STRING_IF_NOT_NULL_RETURN(env, destination, payload, sid);
  C2J_SET_STRING_IF_NOT_NULL_RETURN(env, destination, payload, baseURI);
  C2J_SET_STRING_IF_NOT_NULL_RETURN(env, destination, payload, name);
  C2J_SET_STRING_IF_NOT_NULL_RETURN(env, destination, payload, uri);

  SET_TYPES_INTERFACES(env, destination, payload, type, iface);

  C2J_SET_LL_PROPERTY(
      env, destination, payload, resources, OCResourcePayload *,
      HELPER_CALL_RETURN(js_OCResourcePayload(env, current, &item)));

  // ignore "next"
  return std::string();
}

static std::string js_OCDevicePayload(napi_env env, OCDevicePayload *payload,
                                      napi_value destination) {
  C2J_SET_STRING_IF_NOT_NULL_RETURN(env, destination, payload, sid);
  C2J_SET_STRING_IF_NOT_NULL_RETURN(env, destination, payload, deviceName);
  C2J_SET_STRING_IF_NOT_NULL_RETURN(env, destination, payload, specVersion);

  C2J_SET_STRING_LL_PROPERTY(env, destination, payload, dataModelVersions);

  SET_TYPES_INTERFACES(env, destination, payload, types, interfaces);

  return std::string();
}

static std::string js_OCPlatformPayload(napi_env env,
                                        OCPlatformPayload *payload,
                                        napi_value destination) {
  C2J_SET_STRING_IF_NOT_NULL_RETURN(env, destination, payload, uri);
  C2J_SET_PROPERTY_CALL_RETURN(
      env, destination, "info",
      HELPER_CALL_RETURN(js_OCPlatformInfo(env, &(payload->info), &jsValue)));
  C2J_SET_STRING_LL_PROPERTY(env, destination, payload, rt);
  C2J_SET_STRING_LL_PROPERTY(env, destination, payload, interfaces);
  return std::string();
}

std::string js_OCPayload(napi_env env, OCPayload *payload, napi_value *result) {
  NAPI_CALL_RETURN(napi_create_object(env, result));
  C2J_SET_PROPERTY_RETURN(env, *result, "type", number,
                          ((double)(payload->type)));
  switch (payload->type) {
    case PAYLOAD_TYPE_DISCOVERY:
      HELPER_CALL_RETURN(
          js_OCDiscoveryPayload(env, (OCDiscoveryPayload *)payload, *result));
      break;

    case PAYLOAD_TYPE_DEVICE:
      HELPER_CALL_RETURN(
          js_OCDevicePayload(env, (OCDevicePayload *)payload, *result));
      break;

    case PAYLOAD_TYPE_PLATFORM:
      HELPER_CALL_RETURN(
          js_OCPlatformPayload(env, (OCPlatformPayload *)payload, *result));
      break;

    case PAYLOAD_TYPE_PRESENCE:
      HELPER_CALL_RETURN(
          js_OCPresencePayload(env, (OCPresencePayload *)payload, *result));
      break;

    case PAYLOAD_TYPE_REPRESENTATION:
      HELPER_CALL_RETURN(
          js_OCRepPayload(env, (OCRepPayload *)payload, *result));
      break;

    /*
        case PAYLOAD_TYPE_SECURITY:
          return js_OCSecurityPayload((OCSecurityPayload *)payload);
    */

    case PAYLOAD_TYPE_INVALID:
    default:
      break;
  }
  return std::string();
}

std::string c_OCPayload(napi_env env, napi_value source,
                        OCPayload **destination) {
  DECLARE_VALUE_TYPE_RETURN(jsType, env, source);
  if (jsType == napi_null || jsType == napi_undefined) {
    *destination = nullptr;
    return std::string();
  }

  J2C_DECLARE_PROPERTY_JS_RETURN(jsPayloadType, env, source, "type");
  J2C_DECLARE_VALUE_JS_RETURN(OCPayloadType, payloadType, env, jsPayloadType,
                              napi_number, "payload.type", uint32, uint32_t);

  if (payloadType != PAYLOAD_TYPE_REPRESENTATION) {
    return LOCAL_MESSAGE("Payload other than representation not supported");
  }

  HELPER_CALL_RETURN(c_OCRepPayload(env, source, (OCRepPayload **)destination));

  return std::string();
}
