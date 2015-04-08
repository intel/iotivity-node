//******************************************************************
//
// Copyright 2014 Samsung Electronics All Rights Reserved.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#ifndef OIC_GROUP_H
#define OIC_GROUP_H

#include "ocstack.h"
#include "ocstackinternal.h"

void AddCapability(OCCapability** head, OCCapability* node);

void AddAction(OCAction** head, OCAction* node);

void AddActionSet(OCActionSet **head, OCActionSet* node);

void DeleteCapability(OCCapability *del);

void DeleteAction(OCAction** action);

void DeleteActionSet(OCActionSet** actionset);

OCStackResult DeleteActionSets(OCResource** resource);

OCStackResult FindAndDeleteActionSet(OCResource **resource, const char * actionsetName);

OCStackResult GetActionSetFromString(OCResource **resource, unsigned char *request, char** method,
        char **actionsetName);

OCStackResult GetStringFromActionSet(OCActionSet* actionset, char** desc);

OCStackApplicationResult ActionSetCB(void* context, OCDoHandle handle,
        OCClientResponse* clientResponse);

void ActionSetCD(void *context);

OCStackResult
BuildCollectionGroupActionJSONResponse(OCMethod method/*OCEntityHandlerFlag flag*/,
        OCResource *resource, OCEntityHandlerRequest *ehRequest);

#endif // OIC_GROUP_H
