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

#include <node_api.h>
#include <stdio.h>
#include "constants.h"
#include "enums.h"
#include "functions.h"
#include "structures/handles.h"

void Init(napi_env env, napi_value exports, napi_value module, void *) {
  HELPER_CALL(InitEnums(env, exports), THROW_BODY(env, ));
  HELPER_CALL(InitConstants(env, exports), THROW_BODY(env, ));
  HELPER_CALL(InitFunctions(env, exports), THROW_BODY(env, ));
}

NAPI_MODULE(iotivity, Init)
