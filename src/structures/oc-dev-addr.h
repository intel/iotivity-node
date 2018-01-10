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

#ifndef __IOTIVITY_NODE_OCDEVADDR_H__
#define __IOTIVITY_NODE_OCDEVADDR_H__

#include "../common.h"

extern "C" {
#include <ocstack.h>
}

std::string c_OCDevAddr(napi_env env, napi_value source,
                        OCDevAddr *destination);
std::string js_OCDevAddr(napi_env env, OCDevAddr *source,
                         napi_value *destination);

#endif /* __IOTIVITY_NODE_OCDEVADDR_H__ */
