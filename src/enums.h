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

#ifndef __IOTIVITY_NODE_ENUMS_H__
#define __IOTIVITY_NODE_ENUMS_H__

#include <v8.h>

void InitEnums(v8::Handle<v8::Object> exports);

#define SET_CONSTANT_NUMBER(destination, name)                   \
  Nan::ForceSet((destination), Nan::New(#name).ToLocalChecked(), \
                Nan::New((name)),                                \
                (v8::PropertyAttribute)(v8::ReadOnly | v8::DontDelete));
#define SET_CONSTANT_STRING(destination, name)                   \
  Nan::ForceSet((destination), Nan::New(#name).ToLocalChecked(), \
                Nan::New((name)).ToLocalChecked(),               \
                (v8::PropertyAttribute)(v8::ReadOnly | v8::DontDelete));
#endif /* __IOTIVITY_NODE_ENUMS_H__ */
