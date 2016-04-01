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

#ifndef __IOTIVITY_NODE_OCHEADEROPTION_ARRAY__
#define __IOTIVITY_NODE_OCHEADEROPTION_ARRAY__

#include <v8.h>
extern "C" {
#include <ocstack.h>
}

v8::Local<v8::Array> js_OCHeaderOption(OCHeaderOption *options,
                                       uint8_t optionCount);
bool c_OCHeaderOption(v8::Local<v8::Array> jsOptions, OCHeaderOption *p_options,
                      uint8_t *p_optionCount);

#endif /* __IOTIVITY_NODE_OCHEADEROPTION_ARRAY__ */
