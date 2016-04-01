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

#ifndef __IOTIVITY_NODE_OC_CREATE_DELETE_RESOURCE_H__
#define __IOTIVITY_NODE_OC_CREATE_DELETE_RESOURCE_H__

#include <nan.h>

NAN_METHOD(bind_OCCreateResource);
NAN_METHOD(bind_OCDeleteResource);
NAN_METHOD(bind_OCBindResourceHandler);
NAN_METHOD(bind_OCBindResource);
NAN_METHOD(bind_OCBindResourceInterfaceToResource);
NAN_METHOD(bind_OCBindResourceTypeToResource);
NAN_METHOD(bind_OCGetResourceHandler);

#endif /* __IOTIVITY_NODE_OC_CREATE_DELETE_RESOURCE_H__ */
