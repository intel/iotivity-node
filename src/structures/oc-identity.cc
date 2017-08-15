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

#include "oc-identity.h"

extern "C" {
#include <string.h>
}

std::string js_OCIdentity(napi_env env, OCIdentity *source,
                          napi_value *destination) {
  HELPER_CALL_RETURN(js_ArrayFromBytes(
      env, source->id, (uint32_t)(source->id_length), destination));
  return std::string();
}

std::string c_OCIdentity(napi_env env, napi_value source,
                         OCIdentity *destination) {
  uint32_t length;
  NAPI_CALL_RETURN(env, napi_get_array_length(env, source, &length));
  if (length > MAX_IDENTITY_SIZE) {
    return LOCAL_MESSAGE("array length " + std::to_string(length) +
                         " exceeds MAX_IDENTITY_SIZE(" +
                         std::to_string(MAX_IDENTITY_SIZE) + ")");
  }
  HELPER_CALL_RETURN(c_ArrayFromBytes(env, source, destination->id, length));
  destination->id_length = (uint16_t)length;
  return std::string();
}
