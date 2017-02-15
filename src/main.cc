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

#include <node_api_helpers.h>
#include <node_jsvmapi.h>
#include "constants.h"
#include "enums.h"
#include "functions.h"
#include "structures/handles.h"

static napi_env global__env = nullptr;

napi_env napi_get_env() { return global__env; }

NAPI_MODULE_INIT(Init) {
  global__env = env;
  HELPER_CALL_THROW(env, InitEnums(env, exports));
  HELPER_CALL_THROW(env, InitConstants(env, exports));
  HELPER_CALL_THROW(env, InitFunctions(env, exports));
  HELPER_CALL_THROW(env, InitHandles(env));
}

NODE_MODULE_ABI(iotivity, Init)
