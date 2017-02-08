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

#define CONVEY_SIMPLE_CALL(api)                                    \
  napi_value result;                                               \
  BINDING_NAPI_CALL(env, napi_create_number(env, api(), &result)); \
  BINDING_NAPI_CALL(env, napi_set_return_value(env, info, result));

NAPI_METHOD(bind_OCStop) { CONVEY_SIMPLE_CALL(OCStop); }

NAPI_METHOD(bind_OCProcess) { CONVEY_SIMPLE_CALL(OCProcess); }
