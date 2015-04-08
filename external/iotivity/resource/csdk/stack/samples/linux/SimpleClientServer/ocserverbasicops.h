//******************************************************************
//
// Copyright 2014 Intel Mobile Communications GmbH All Rights Reserved.
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

#ifndef OCSERVER_BASICOPS_H_
#define OCSERVER_BASICOPS_H_

#include "ocstack.h"

//-----------------------------------------------------------------------------
// Defines
//-----------------------------------------------------------------------------
#define TAG "ocserverbasicops"

//-----------------------------------------------------------------------------
// Typedefs
//-----------------------------------------------------------------------------

/* Structure to represent a LED resource */
typedef struct LEDRESOURCE{
    OCResourceHandle handle;
    bool state;
    int power;
} LEDResource;

//-----------------------------------------------------------------------------
// Function prototype
//-----------------------------------------------------------------------------

/* Function that creates a new LED resource by calling the
 * OCCreateResource() method.
 */
int createLEDResource (char *uri, LEDResource *ledResource, bool resourceState, int resourcePower);

/* This method converts the payload to JSON format */
char* constructJsonResponse (OCEntityHandlerRequest *ehRequest);

/* Following methods process the PUT, GET, POST
 * requests
 */
OCEntityHandlerResult ProcessGetRequest (OCEntityHandlerRequest *ehRequest,
                                         char *payload,
                                         uint16_t maxPayloadSize);
OCEntityHandlerResult ProcessPutRequest (OCEntityHandlerRequest *ehRequest,
                                         char *payload,
                                         uint16_t maxPayloadSize);
OCEntityHandlerResult ProcessPostRequest (OCEntityHandlerRequest *ehRequest,
                                          char *payload,
                                          uint16_t maxPayloadSize);

/* call getResult in common.cpp to get the result in string format. */
const char *getResult(OCStackResult result);

//-----------------------------------------------------------------------------
// Callback functions
//-----------------------------------------------------------------------------

/* Entity Handler callback functions */

OCEntityHandlerResult
OCEntityHandlerCb (OCEntityHandlerFlag flag,
        OCEntityHandlerRequest *entityHandlerRequest);

#endif
