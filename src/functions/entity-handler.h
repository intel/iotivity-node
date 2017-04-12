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

#ifndef _ENTITY_HANDLER_H_
#define _ENTITI_HANDLER_H_

extern "C" {
#include <ocstack.h>
}

#include "../common.h"

std::string entityHandler(napi_env env, OCEntityHandlerFlag flag,
                          OCEntityHandlerRequest *request, char *uri,
                          napi_ref callback, OCEntityHandlerResult *result);

#define EH_BODY(env, flag, request, uri, callback)                         \
  OCEntityHandlerResult result = OC_EH_ERROR;                              \
  DECLARE_HANDLE_SCOPE(scope, ((env)), result);                            \
  HELPER_CALL(                                                             \
      entityHandler((env), (flag), (request), (uri), (callback), &result), \
      THROW_BODY((env), result));                                          \
  return result;

#endif /* ndef _ENTITI_HANDLER_H_ */
