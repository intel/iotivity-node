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

#include "oc-client-response.h"
#include "oc-dev-addr.h"
#include "oc-identity.h"

extern "C" {
#include <string.h>
}

std::string js_OCClientResponse(napi_env env, OCClientResponse *source,
                                napi_value *destination) {
  NAPI_CALL_RETURN(napi_create_object(env, destination));

  C2J_SET_PROPERTY_CALL_RETURN(
      env, *destination, "devAddr",
      HELPER_CALL_RETURN(js_OCDevAddr(env, &(source->devAddr), &jsValue)));

  if (source->addr) {
    C2J_SET_PROPERTY_CALL_RETURN(
        env, *destination, "addr",
        HELPER_CALL_RETURN(js_OCDevAddr(env, source->addr, &jsValue)));
  }

  // response.payload is not set

  C2J_SET_NUMBER_MEMBER_RETURN(env, *destination, source, connType);

  C2J_SET_PROPERTY_CALL_RETURN(
      env, *destination, "identity",
      HELPER_CALL_RETURN(js_OCIdentity(env, &(source->identity), &jsValue)));

  C2J_SET_NUMBER_MEMBER_RETURN(env, *destination, source, result);
  C2J_SET_NUMBER_MEMBER_RETURN(env, *destination, source, sequenceNumber);

  // FIXME - iotivity has a bug whereby these fields are left uninitialized in
  // a presence response
  if (!(source->payload && source->payload->type == PAYLOAD_TYPE_PRESENCE)) {
    C2J_SET_STRING_IF_NOT_NULL_RETURN(env, *destination, source, resourceUri);

    // vendor options are not set
  }
  return std::string();
}
