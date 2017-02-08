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

NAPI_MODULE_INIT(Init) {
  BINDING_HELPER_CALL(env, InitEnums(env, exports));
  BINDING_HELPER_CALL(env, InitConstants(env, exports));
  BINDING_HELPER_CALL(env, InitFunctions(env, exports));
}

NODE_MODULE_ABI(iotivity, Init)
