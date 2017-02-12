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

#ifndef __IOTIVITY_NODE_FUNCTIONS_H__
#define __IOTIVITY_NODE_FUNCTIONS_H__

#include "common.h"

#define SET_FUNCTION(env, destination, functionName)                     \
  C2J_SET_PROPERTY_RETURN((env), (destination), #functionName, function, \
                          bind_##functionName, 0)

std::string InitFunctions(napi_env env, napi_value exports);

#endif /* __IOTIVITY_NODE_FUNCTIONS_H__ */
