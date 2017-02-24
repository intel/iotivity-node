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

#include "entity-handler.h"

std::string entityHandler(napi_env env, OCEntityHandlerFlag flag,
                          OCEntityHandlerRequest *request, char *uri,
                          napi_ref callback, OCEntityHandlerResult *result) {
  *result = OC_EH_ERROR;
  napi_value jsContext, jsCallback, jsReturnValue;
  NAPI_CALL_RETURN(napi_get_null(env, &jsContext));
  NAPI_CALL_RETURN(napi_get_reference_value(env, callback, &jsCallback));

  napi_value arguments[3];
  NAPI_CALL_RETURN(napi_create_number(env, (double)flag, &arguments[0]));
  NAPI_CALL_RETURN(napi_get_null(env, &arguments[1]));
  if (uri) {
    NAPI_CALL_RETURN(
        napi_create_string_utf8(env, uri, strlen(uri), &arguments[2]));
  }

  NAPI_CALL_RETURN(napi_call_function(env, jsContext, jsCallback, (uri ? 3 : 2),
                                      arguments, &jsReturnValue));

  J2C_ASSIGN_VALUE_JS_RETURN(OCEntityHandlerResult, *result, env, jsReturnValue,
                             napi_number, "entity handler return value", uint32,
                             uint32_t);
  return std::string();
}
