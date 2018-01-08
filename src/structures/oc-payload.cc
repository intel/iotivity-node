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
#include "../common.h"
#include "handles.h"
#include "oc-payload-macros.h"
#include "oc-rep-payload/to-c.h"
#include "oc-rep-payload/to-js.h"

extern "C" {
#include <ocpayload.h>
#include <string.h>
}

static std::string js_OCEndpointPayload(napi_env env,
                                        OCEndpointPayload* item,
                                        napi_value* js_item) {
  NAPI_CALL_RETURN(env, napi_create_object(env, js_item));

  C2J_SET_STRING_IF_NOT_NULL_RETURN(env, *js_item, item, tps);
  C2J_SET_STRING_IF_NOT_NULL_RETURN(env, *js_item, item, addr);
  C2J_SET_NUMBER_MEMBER_RETURN(env, *js_item, item, family);
  C2J_SET_NUMBER_MEMBER_RETURN(env, *js_item, item, port);
  C2J_SET_NUMBER_MEMBER_RETURN(env, *js_item, item, pri);

  return std::string();
}

static std::string js_OCResourcePayload(napi_env env,
                                        OCResourcePayload *payload,
                                        napi_value *destination) {
  NAPI_CALL_RETURN(env, napi_create_object(env, destination));

  C2J_SET_STRING_IF_NOT_NULL_RETURN(env, *destination, payload, uri);
  C2J_SET_STRING_IF_NOT_NULL_RETURN(env, *destination, payload, rel);
  C2J_SET_STRING_IF_NOT_NULL_RETURN(env, *destination, payload, anchor);

  SET_TYPES_INTERFACES(env, *destination, payload, types, interfaces);

  C2J_SET_NUMBER_MEMBER_RETURN(env, *destination, payload, bitmap);

  C2J_SET_PROPERTY_CALL_RETURN(
      env, *destination, "secure",
      NAPI_CALL_RETURN(env, napi_get_boolean(env, payload->secure, &jsValue)));

  C2J_SET_NUMBER_MEMBER_RETURN(env, *destination, payload, port);

  C2J_SET_LL_PROPERTY(env, *destination, payload, eps, OCEndpointPayload*,
      HELPER_CALL_RETURN(js_OCEndpointPayload(env, current, &item)));
  return std::string();
}

static std::string js_OCDiscoveryPayload(napi_env env,
                                         OCDiscoveryPayload *payload,
                                         napi_value destination) {
  C2J_SET_STRING_IF_NOT_NULL_RETURN(env, destination, payload, sid);
  C2J_SET_STRING_IF_NOT_NULL_RETURN(env, destination, payload, name);

  SET_TYPES_INTERFACES(env, destination, payload, type, iface);

  C2J_SET_LL_PROPERTY(
      env, destination, payload, resources, OCResourcePayload *,
      HELPER_CALL_RETURN(js_OCResourcePayload(env, current, &item)));

  // ignore "next"
  return std::string();
}

std::string js_OCPresencePayload(napi_env env, OCPresencePayload *payload,
                                 napi_value destination) {
  C2J_SET_NUMBER_MEMBER_RETURN(env, destination, payload, sequenceNumber);
  C2J_SET_NUMBER_MEMBER_RETURN(env, destination, payload, maxAge);
  C2J_SET_NUMBER_MEMBER_RETURN(env, destination, payload, trigger);
  C2J_SET_STRING_IF_NOT_NULL_RETURN(env, destination, payload, resourceType);

  return std::string();
}

std::string js_OCSecurityPayload(napi_env env, OCSecurityPayload *payload,
                                 napi_value destination) {
  if (payload->securityData) {
    napi_value securityData;
    HELPER_CALL_RETURN(
        js_ArrayFromBytes(env, ((unsigned char *)(payload->securityData)),
                          payload->payloadSize, &securityData));
    NAPI_CALL_RETURN(env, napi_set_named_property(
                              env, destination, "securityData", securityData));
  }

  return std::string();
}

std::string js_OCPayload(napi_env env, OCPayload *payload, napi_value *result) {
  NAPI_CALL_RETURN(env, napi_create_object(env, result));
  C2J_SET_PROPERTY_RETURN(env, *result, "type", double,
                          ((double)(payload->type)));
  switch (payload->type) {
    case PAYLOAD_TYPE_DISCOVERY:
      HELPER_CALL_RETURN(
          js_OCDiscoveryPayload(env, (OCDiscoveryPayload *)payload, *result));
      break;

    case PAYLOAD_TYPE_PRESENCE:
      HELPER_CALL_RETURN(
          js_OCPresencePayload(env, (OCPresencePayload *)payload, *result));
      break;

    case PAYLOAD_TYPE_REPRESENTATION:
      HELPER_CALL_RETURN(
          js_OCRepPayload(env, (OCRepPayload *)payload, *result));
      break;

    case PAYLOAD_TYPE_SECURITY:
      HELPER_CALL_RETURN(
          js_OCSecurityPayload(env, (OCSecurityPayload *)payload, *result));
      break;

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

  if (payloadType == PAYLOAD_TYPE_REPRESENTATION) {
    HELPER_CALL_RETURN(
        c_OCRepPayload(env, source, (OCRepPayload **)destination));
  } else {
    return LOCAL_MESSAGE("Support for this payload type not implemented");
  }
  return std::string();
}
