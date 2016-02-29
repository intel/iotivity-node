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

#ifndef __IOTIVITY_NODE_FUNCTIONS_SIMPLE_H__
#define __IOTIVITY_NODE_FUNCTIONS_SIMPLE_H__

#include <v8.h>

NAN_METHOD(bind_OCGetNumberOfResourceInterfaces);
NAN_METHOD(bind_OCGetNumberOfResources);
NAN_METHOD(bind_OCGetNumberOfResourceTypes);
NAN_METHOD(bind_OCGetResourceHandle);
NAN_METHOD(bind_OCGetResourceHandleFromCollection);
NAN_METHOD(bind_OCGetResourceInterfaceName);
NAN_METHOD(bind_OCGetResourceProperties);
NAN_METHOD(bind_OCGetResourceTypeName);
NAN_METHOD(bind_OCGetResourceUri);
NAN_METHOD(bind_OCGetServerInstanceIDString);
NAN_METHOD(bind_OCInit);
NAN_METHOD(bind_OCProcess);
NAN_METHOD(bind_OCSetDeviceInfo);
NAN_METHOD(bind_OCSetPlatformInfo);
NAN_METHOD(bind_OCStartPresence);
NAN_METHOD(bind_OCStop);
NAN_METHOD(bind_OCStopPresence);
NAN_METHOD(bind_OCUnBindResource);

#endif /* __IOTIVITY_NODE_FUNCTIONS_SIMPLE_H__ */
