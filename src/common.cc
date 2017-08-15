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

#include "common.h"

NapiHandleScope::NapiHandleScope(napi_env _env) : scope(nullptr), env(_env) {}

std::string NapiHandleScope::open() {
  NAPI_CALL_RETURN(env, napi_open_handle_scope(env, &scope));
  return std::string();
}

NapiHandleScope::~NapiHandleScope() {
  if (scope) {
    NAPI_CALL(env, napi_close_handle_scope(env, scope), THROW_BODY(env, ));
  }
}

std::string js_StringArray(napi_env env, OCStringLL *source, napi_value *ar) {
  size_t count;
  OCStringLL *item;
  napi_value val;

  for (item = source, count = 0; item; item = item->next, count++)
    ;
  NAPI_CALL_RETURN(env, napi_create_array_with_length(env, count, ar));
  for (item = source, count = 0; item; item = item->next, count++) {
    DECLARE_HANDLE_SCOPE(scope, env, LOCAL_MESSAGE("failed to open scope"));
    NAPI_CALL_RETURN(env, napi_create_string_utf8(env, item->value, -1, &val));
    NAPI_CALL_RETURN(env, napi_set_element(env, *ar, count, val));
  }
  return std::string();
}

std::string js_ArrayFromBytes(napi_env env, unsigned char *bytes,
                              uint32_t length, napi_value *array) {
  uint32_t index;
  napi_value oneValue;
  NAPI_CALL_RETURN(env, napi_create_array_with_length(env, length, array));
  for (index = 0; index < length; index++) {
    NAPI_CALL_RETURN(env, napi_create_uint32(env, (uint32_t)bytes[index],
                                             &oneValue));
    NAPI_CALL_RETURN(env, napi_set_element(env, *array, index, oneValue));
  }
  return std::string();
}

std::string c_ArrayFromBytes(napi_env env, napi_value array,
                             unsigned char *bytes, uint32_t length) {
  uint32_t index, arrayLength;
  napi_value oneValue;
  NAPI_CALL_RETURN(env, napi_get_array_length(env, array, &arrayLength));
  for (index = 0; index < arrayLength; index++) {
    NAPI_CALL_RETURN(env, napi_get_element(env, array, index, &oneValue));
    J2C_ASSIGN_VALUE_JS_RETURN(
        unsigned char, bytes[index], env, oneValue, napi_number,
        "byte array item " + std::to_string(index), uint32, uint32_t);
  }
  return std::string();
}
