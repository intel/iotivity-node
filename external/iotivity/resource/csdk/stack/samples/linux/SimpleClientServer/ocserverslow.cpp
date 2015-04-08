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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <sys/time.h>
#include <list>
#include "ocstack.h"
#include "ocmalloc.h"
#include "logger.h"
#include "cJSON.h"
#include "ocserverslow.h"

volatile sig_atomic_t gQuitFlag = 0;

static std::list<OCEntityHandlerRequest *> gRequestList;
static constexpr unsigned int SLOW_RESPONSE_DELAY_SEC = 5;

static LEDResource LED;
// This variable determines instance number of the LED resource.
// Used by POST method to create a new instance of LED resource.
static unsigned int gCurrLedInstance = 0;
static constexpr unsigned int SAMPLE_MAX_NUM_POST_INSTANCE = 2;
static LEDResource gLedInstance[SAMPLE_MAX_NUM_POST_INSTANCE];

//char *gResourceUri= const_cast<char *>("/a/led");
char *gResourceUri= (char *)"/a/led";

static constexpr uint16_t OC_WELL_KNOWN_PORT = 5683;

//This function takes the request as an input and returns the response
//in JSON format.
char* constructJsonResponse (OCEntityHandlerRequest *ehRequest)
{
    cJSON *json = cJSON_CreateObject();
    cJSON *format;
    char *jsonResponse;
    LEDResource *currLEDResource = &LED;

    OC_LOG(INFO, TAG, "Entering constructJsonResponse");

    if (ehRequest->resource == gLedInstance[0].handle)
    {
        OC_LOG(INFO, TAG, "handle 0");
        currLEDResource = &gLedInstance[0];
        gResourceUri = const_cast<char *>("a/led/0");
    }
    else if (ehRequest->resource == gLedInstance[1].handle)
    {
        OC_LOG(INFO, TAG, "handle 1");
        currLEDResource = &gLedInstance[1];
        gResourceUri = const_cast<char *>("a/led/1");
    }

    if(OC_REST_PUT == ehRequest->method)
    {
        cJSON *putJson = cJSON_Parse((char *)ehRequest->reqJSONPayload);
        currLEDResource->state = ( !strcmp(cJSON_GetObjectItem(putJson,"state")->valuestring ,
                "on") ? true:false);
        currLEDResource->power = cJSON_GetObjectItem(putJson,"power")->valuedouble;
        cJSON_Delete(putJson);
    }

    cJSON_AddStringToObject(json,"href",gResourceUri);
    cJSON_AddItemToObject(json, "rep", format=cJSON_CreateObject());
    cJSON_AddStringToObject(format, "state", (char *) (currLEDResource->state ? "on":"off"));
    cJSON_AddNumberToObject(format, "power", currLEDResource->power);

    OC_LOG(INFO, TAG, "Before constructJsonResponse print");
    jsonResponse = cJSON_Print(json);
    OC_LOG(INFO, TAG, "Before constructJsonResponse delete");
    cJSON_Delete(json);

    OC_LOG(INFO, TAG, "Before constructJsonResponse return");
    return jsonResponse;
}

void ProcessGetRequest (OCEntityHandlerRequest *ehRequest)
{
    OC_LOG(INFO, TAG, "Entering ProcessGetRequest");
    char *getResp = constructJsonResponse(ehRequest);
    OC_LOG(INFO, TAG, "After constructJsonResponse");
    OCEntityHandlerResponse response;

    // Format the response.  Note this requires some info about the request
    response.requestHandle = ehRequest->requestHandle;
    response.resourceHandle = ehRequest->resource;
    response.ehResult = OC_EH_OK;
    response.payload = (unsigned char *)getResp;
    response.payloadSize = strlen(getResp) + 1;
    response.numSendVendorSpecificHeaderOptions = 0;
    memset(response.sendVendorSpecificHeaderOptions, 0, sizeof response.sendVendorSpecificHeaderOptions);
    memset(response.resourceUri, 0, sizeof(response.resourceUri));
    // Indicate that response is NOT in a persistent buffer
    response.persistentBufferFlag = 0;

    // Send the response
    if (OCDoResponse(&response) != OC_STACK_OK)
    {
        OC_LOG(ERROR, TAG, "Error sending response");
    }

    free(getResp);
}

OCEntityHandlerRequest *CopyRequest(OCEntityHandlerRequest *entityHandlerRequest)
{
    OC_LOG(INFO, TAG, "Copying received request for slow response");
    OCEntityHandlerRequest *request = (OCEntityHandlerRequest *)OCMalloc(sizeof(OCEntityHandlerRequest));
    if (request)
    {
        // Do shallow copy
        memcpy(request, entityHandlerRequest, sizeof(OCEntityHandlerRequest));
        // Do deep copy of query
        request->query = (unsigned char * )OCMalloc(strlen((const char *)entityHandlerRequest->query) + 1);
        if (request->query)
        {
            strcpy((char *)request->query, (const char *)entityHandlerRequest->query);

            // Copy the request payload
            request->reqJSONPayload = (unsigned char * )OCMalloc(strlen((const char *)entityHandlerRequest->reqJSONPayload) + 1);
            if (request->reqJSONPayload)
            {
                strcpy((char *)request->reqJSONPayload, (const char *)entityHandlerRequest->reqJSONPayload);

                // Ignore vendor specific header options for example
                request->numRcvdVendorSpecificHeaderOptions = 0;
                request->rcvdVendorSpecificHeaderOptions = NULL;
            }
            else
            {
                OCFree(request->query);
                OCFree(request);
                request = NULL;
            }
        }
        else
        {
            OCFree(request);
            request = NULL;
        }
    }

    if (request)
    {
        OC_LOG(INFO, TAG, "Copied client request");
    }
    else
    {
        OC_LOG(ERROR, TAG, "Error copying client request");
    }
    return request;
}

OCEntityHandlerResult
OCEntityHandlerCb (OCEntityHandlerFlag flag,
        OCEntityHandlerRequest *entityHandlerRequest)
{
    OCEntityHandlerResult result = OC_EH_ERROR;
    OCEntityHandlerRequest *request = NULL;

    OC_LOG_V (INFO, TAG, "Inside entity handler - flags: 0x%x", flag);
    if (flag & OC_INIT_FLAG)
    {
        OC_LOG(INFO, TAG, "Flag includes OC_INIT_FLAG");
        result = OC_EH_OK;
    }
    if (flag & OC_REQUEST_FLAG)
    {
        OC_LOG(INFO, TAG, "Flag includes OC_REQUEST_FLAG");
        if (entityHandlerRequest)
        {
            OC_LOG_V (INFO, TAG, "request query %s from client",
                                        entityHandlerRequest->query);
            OC_LOG_V (INFO, TAG, "request payload %s from client",
                                        entityHandlerRequest->reqJSONPayload);
            // Make deep copy of received request and queue it for slow processing
            request = CopyRequest(entityHandlerRequest);
            if (request)
            {

                OC_LOG(INFO, TAG, "Scheduling slow response for received request");
                gRequestList.push_back(request);
                // Indicate to the stack that this is a slow response
                result = OC_EH_SLOW;
                // Start the slow response alarm
                alarm(SLOW_RESPONSE_DELAY_SEC);
            }
            else
            {
                OC_LOG(ERROR, TAG, "Error queuing request for slow response");
                // Indicate to the stack that this is a slow response
                result = OC_EH_ERROR;
            }
        }
        else
        {
            OC_LOG(ERROR, TAG, "Invalid request");
            result = OC_EH_ERROR;
        }
    }
    return result;
}

/* SIGINT handler: set gQuitFlag to 1 for graceful termination */
void handleSigInt(int signum)
{
    if (signum == SIGINT)
    {
        gQuitFlag = 1;
    }
}

// SIGINT alarm handler:  alarm set by entity handler.  Does
// slow response when fired
void AlarmHandler(int sig)
{
    if (sig == SIGALRM)
    {
        OC_LOG (INFO, TAG, "Server starting slow response");
        if (gRequestList.empty())
        {
            OC_LOG (INFO, TAG, "No requests to service");
            return;
        }

        // Get the request from the list
        OCEntityHandlerRequest *entityHandlerRequest = gRequestList.front();
        gRequestList.pop_front();
        if (entityHandlerRequest->method == OC_REST_GET)
        {
            OC_LOG (INFO, TAG, "Received OC_REST_GET from client");
            ProcessGetRequest (entityHandlerRequest);
        }
        else
        {
            OC_LOG_V (INFO, TAG, "Received unsupported method %d from client",
                    entityHandlerRequest->method);
        }
        // Free the request
        OCFree(entityHandlerRequest->query);
        OCFree(entityHandlerRequest->reqJSONPayload);
        OCFree(entityHandlerRequest);

        // If there are more requests in list, re-arm the alarm signal
        if (gRequestList.empty())
        {
            alarm(SLOW_RESPONSE_DELAY_SEC);
        }
    }
}

int main(int argc, char* argv[])
{
    uint8_t addr[20] = {0};
    uint8_t* paddr = NULL;
    uint16_t port = OC_WELL_KNOWN_PORT;
    uint8_t ifname[] = "eth0";


    OC_LOG(DEBUG, TAG, "OCServer is starting...");
    /*Get Ip address on defined interface and initialize coap on it with random port number
     * this port number will be used as a source port in all coap communications*/
    if ( OCGetInterfaceAddress(ifname, sizeof(ifname), AF_INET, addr,
                sizeof(addr)) == ERR_SUCCESS)
    {
        OC_LOG_V(INFO, TAG, "Starting ocserver on address %s:%d",addr,port);
        paddr = addr;
    }

    if (OCInit((char *) paddr, port, OC_SERVER) != OC_STACK_OK)
    {
        OC_LOG(ERROR, TAG, "OCStack init error");
        return 0;
    }

    /*
     * Declare and create the example resource: LED
     */
    createLEDResource(gResourceUri, &LED, false, 0);

    // Initialize slow response alarm
    signal(SIGALRM, AlarmHandler);

    // Break from loop with Ctrl-C
    OC_LOG(INFO, TAG, "Entering ocserver main loop...");
    signal(SIGINT, handleSigInt);

    while (!gQuitFlag)
    {
        if (OCProcess() != OC_STACK_OK)
        {
            OC_LOG(ERROR, TAG, "OCStack process error");
            return 0;
        }

        sleep(2);
    }

    OC_LOG(INFO, TAG, "Exiting ocserver main loop...");

    // Free requests
    if (!gRequestList.empty())
    {
        for (auto iter = gRequestList.begin(); iter != gRequestList.end(); ++iter)
        {
            OCFree((*iter)->query);
            OCFree((*iter)->reqJSONPayload);
            OCFree(*iter);
        }
        gRequestList.clear();
    }

    if (OCStop() != OC_STACK_OK)
    {
        OC_LOG(ERROR, TAG, "OCStack process error");
    }

    return 0;
}

int createLEDResource (char *uri, LEDResource *ledResource, bool resourceState, int resourcePower)
{
    if (!uri)
    {
        OC_LOG(ERROR, TAG, "Resource URI cannot be NULL");
        return -1;
    }

    ledResource->state = resourceState;
    ledResource->power= resourcePower;
    OCStackResult res = OCCreateResource(&(ledResource->handle),
            "core.led",
            "oc.mi.def",
            uri,
            OCEntityHandlerCb,
            OC_DISCOVERABLE|OC_OBSERVABLE);
    OC_LOG_V(INFO, TAG, "Created LED resource with result: %s", getResult(res));

    return 0;
}
