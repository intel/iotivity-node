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

#include <node.h>
#include <nan.h>

#include "functions/oc-cancel.h"
#include "functions/oc-create-delete-resource.h"
#include "functions/oc-do-resource.h"
#include "functions/oc-do-response.h"
#include "functions/oc-notify.h"
#include "functions/oc-random.h"
#include "functions/oc-register-persistent-storage-handler.h"
#include "functions/oc-set-default-device-entity-handler.h"
#include "functions/simple.h"

using namespace v8;

#define SET_FUNCTION(destination, functionName)                              \
  Nan::ForceSet((destination), Nan::New(#functionName).ToLocalChecked(),     \
                Nan::GetFunction(Nan::New<FunctionTemplate>(                 \
                                     bind_##functionName)).ToLocalChecked(), \
                (v8::PropertyAttribute)(v8::DontDelete));

void InitFunctions(Handle<Object> exports, Handle<Object> module) {
  SET_FUNCTION(exports, OCBindResource);
  SET_FUNCTION(exports, OCBindResourceHandler);
  SET_FUNCTION(exports, OCBindResourceInterfaceToResource);
  SET_FUNCTION(exports, OCBindResourceTypeToResource);
  SET_FUNCTION(exports, OCCancel);
  SET_FUNCTION(exports, OCConvertUuidToString);
  SET_FUNCTION(exports, OCCreateResource);
  SET_FUNCTION(exports, OCDeleteResource);
  SET_FUNCTION(exports, OCDoResource);
  SET_FUNCTION(exports, OCDoResponse);
  SET_FUNCTION(exports, OCFillRandomMem);
  SET_FUNCTION(exports, OCGenerateUuid);
  SET_FUNCTION(exports, OCGenerateUuidString);
  SET_FUNCTION(exports, OCGetNumberOfResourceInterfaces);
  SET_FUNCTION(exports, OCGetNumberOfResources);
  SET_FUNCTION(exports, OCGetNumberOfResourceTypes);
  SET_FUNCTION(exports, OCGetRandom);
  SET_FUNCTION(exports, OCGetRandomByte);
  SET_FUNCTION(exports, OCGetRandomRange);
  SET_FUNCTION(exports, OCGetResourceHandle);
  SET_FUNCTION(exports, OCGetResourceHandleFromCollection);
  SET_FUNCTION(exports, OCGetResourceHandler);
  SET_FUNCTION(exports, OCGetResourceInterfaceName);
  SET_FUNCTION(exports, OCGetResourceProperties);
  SET_FUNCTION(exports, OCGetResourceTypeName);
  SET_FUNCTION(exports, OCGetResourceUri);
  SET_FUNCTION(exports, OCGetServerInstanceIDString);
  SET_FUNCTION(exports, OCInit);
  SET_FUNCTION(exports, OCNotifyAllObservers);
  SET_FUNCTION(exports, OCNotifyListOfObservers);
  SET_FUNCTION(exports, OCProcess);
  SET_FUNCTION(exports, OCRegisterPersistentStorageHandler);
  SET_FUNCTION(exports, OCSeedRandom);
  SET_FUNCTION(exports, OCSetDefaultDeviceEntityHandler);
  SET_FUNCTION(exports, OCSetDeviceInfo);
  SET_FUNCTION(exports, OCSetPlatformInfo);
  SET_FUNCTION(exports, OCStartPresence);
  SET_FUNCTION(exports, OCStop);
  SET_FUNCTION(exports, OCStopPresence);
  SET_FUNCTION(exports, OCUnBindResource);
}
