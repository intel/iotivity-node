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

#ifndef __IOTIVITY_NODE_FUNCTIONS_OC_RANDOM_H__
#define __IOTIVITY_NODE_FUNCTIONS_OC_RANDOM_H__

#include <v8.h>

NAN_METHOD(bind_OCConvertUuidToString);
NAN_METHOD(bind_OCFillRandomMem);
NAN_METHOD(bind_OCGenerateUuid);
NAN_METHOD(bind_OCGenerateUuidString);
NAN_METHOD(bind_OCGetRandom);
NAN_METHOD(bind_OCGetRandomByte);
NAN_METHOD(bind_OCGetRandomRange);
NAN_METHOD(bind_OCSeedRandom);

#endif /* __IOTIVITY_NODE_FUNCTIONS_OC_RANDOM_H__ */
