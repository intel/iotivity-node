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

#include <nan.h>
#include <node_jsvmapi.h>
#include <stdio.h>
#include "constants.h"
#include "enums.h"
#include "functions.h"
#include "structures/handles.h"

void Init(napi_env env, napi_value exports, napi_value module) {
  HELPER_CALL_THROW(env, InitEnums(env, exports));
  HELPER_CALL_THROW(env, InitConstants(env, exports));
  HELPER_CALL_THROW(env, InitFunctions(env, exports));
}

NODE_MODULE_ABI(iotivity, Init)
