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
#include "ocstack.h"
#include "logger.h"
#include "cJSON.h"
#include "ocserverbasicops.h"
#include "common.h"

int gQuitFlag = 0;

static LEDResource LED;
// This variable determines instance number of the LED resource.
// Used by POST method to create a new instance of LED resource.
static int gCurrLedInstance = 0;
#define SAMPLE_MAX_NUM_POST_INSTANCE  2
static LEDResource gLedInstance[SAMPLE_MAX_NUM_POST_INSTANCE];

char *gResourceUri= (char *)"/a/led";

static uint16_t OC_WELL_KNOWN_PORT = 5683;

//File containing Server's Identity and the PSK credentials
//of other devices which the server trusts
//This can be generated using 'gen_sec_bin' application
static char CRED_FILE[] = "server_cred.bin";

//This function takes the request as an input and returns the response
//in JSON format.
char* constructJsonResponse (OCEntityHandlerRequest *ehRequest)
{
    cJSON *json = cJSON_CreateObject();
    cJSON *format;
    char *jsonResponse;
    LEDResource *currLEDResource = &LED;

    if (ehRequest->resource == gLedInstance[0].handle)
    {
        currLEDResource = &gLedInstance[0];
        gResourceUri = (char *) "a/led/0";
    }
    else if (ehRequest->resource == gLedInstance[1].handle)
    {
        currLEDResource = &gLedInstance[1];
        gResourceUri = (char *) "a/led/1";
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

    jsonResponse = cJSON_Print(json);
    cJSON_Delete(json);
    return jsonResponse;
}

OCEntityHandlerResult ProcessGetRequest (OCEntityHandlerRequest *ehRequest,
                        char *payload, size_t maxPayloadSize)
{
    OCEntityHandlerResult ehResult;

    char *getResp = constructJsonResponse(ehRequest);
    if (maxPayloadSize > strlen (getResp))
    {
        strcpy(payload, getResp);
        ehResult = OC_EH_OK;
    }
    else
    {
        OC_LOG_V (INFO, TAG, "Response buffer: %d bytes is too small",
                maxPayloadSize);
        ehResult = OC_EH_ERROR;
    }

    free(getResp);

    return ehResult;
}

OCEntityHandlerResult ProcessPutRequest (OCEntityHandlerRequest *ehRequest,
                        char *payload, size_t maxPayloadSize)
{
    OCEntityHandlerResult ehResult;

    char *putResp = constructJsonResponse(ehRequest);
    if (maxPayloadSize > strlen (putResp))
    {
        strcpy(payload, putResp);
        ehResult = OC_EH_OK;
    }
    else
    {
        OC_LOG_V (INFO, TAG, "Response buffer: %d bytes is too small",
                maxPayloadSize);
        ehResult = OC_EH_ERROR;
    }

    free(putResp);

    return ehResult;
}

OCEntityHandlerResult ProcessPostRequest (OCEntityHandlerRequest *ehRequest,
                        char *payload, size_t maxPayloadSize)
{
    char *respPLPost_led = NULL;
    cJSON *json;
    cJSON *format;
    OCEntityHandlerResult ehResult;

    /*
     * The entity handler determines how to process a POST request.
     * Per the REST paradigm, POST can also be used to update representation of existing
     * resource or create a new resource.
     * In the sample below, if the POST is for /a/led then a new instance of the LED
     * resource is created with default representation (if representation is included in
     * POST payload it can be used as initial values) as long as the instance is
     * lesser than max new instance count. Once max instance count is reached, POST on
     * /a/led updated the representation of /a/led (just like PUT)
     */

    if (ehRequest->resource == LED.handle)
    {
        if (gCurrLedInstance < SAMPLE_MAX_NUM_POST_INSTANCE)
        {
            // Create new LED instance
            char newLedUri[15] = "/a/led/";
            int newLedUriLength = strlen(newLedUri);
            snprintf (newLedUri + newLedUriLength, sizeof(newLedUri)-newLedUriLength, "%d", gCurrLedInstance);

            json = cJSON_CreateObject();

            cJSON_AddStringToObject(json,"href",gResourceUri);
            cJSON_AddItemToObject(json, "rep", format=cJSON_CreateObject());
            cJSON_AddStringToObject(format, "createduri", (char *) newLedUri);

            if (0 == createLEDResource (newLedUri, &gLedInstance[gCurrLedInstance], false, 0))
            {
                OC_LOG (INFO, TAG, "Created new LED instance");
                gLedInstance[gCurrLedInstance].state = 0;
                gLedInstance[gCurrLedInstance].power = 0;
                gCurrLedInstance++;
                respPLPost_led = cJSON_Print(json);
            }

            cJSON_Delete(json);
        }
        else
        {
            respPLPost_led = constructJsonResponse(ehRequest);
        }
    }
    else
    {
        for (int i = 0; i < SAMPLE_MAX_NUM_POST_INSTANCE; i++)
        {
            if (ehRequest->resource == gLedInstance[i].handle)
            {
                if (i == 0)
                {
                    respPLPost_led = constructJsonResponse(ehRequest);
                    break;
                }
                else if (i == 1)
                {
                    respPLPost_led = constructJsonResponse(ehRequest);
                }
            }
        }
    }

    if ((respPLPost_led != NULL) && (maxPayloadSize > strlen (respPLPost_led)))
    {
        strcpy(payload, respPLPost_led);
        ehResult = OC_EH_OK;
    }
    else
    {
        OC_LOG_V (INFO, TAG, "Response buffer: %d bytes is too small",
                maxPayloadSize);
        ehResult = OC_EH_ERROR;
    }

    free(respPLPost_led);

    return ehResult;
}

OCEntityHandlerResult
OCEntityHandlerCb (OCEntityHandlerFlag flag,
        OCEntityHandlerRequest *entityHandlerRequest)
{
    OC_LOG_V (INFO, TAG, "Inside entity handler - flags: 0x%x", flag);

    OCEntityHandlerResult ehResult = OC_EH_ERROR;
    OCEntityHandlerResponse response;
    char payload[MAX_RESPONSE_LENGTH];

    if (flag & OC_INIT_FLAG)
    {
        OC_LOG (INFO, TAG, "Flag includes OC_INIT_FLAG");
        ehResult = OC_EH_OK;
    }
    if (flag & OC_REQUEST_FLAG)
    {
        OC_LOG (INFO, TAG, "Flag includes OC_REQUEST_FLAG");
        if (entityHandlerRequest)
        {
            if (OC_REST_GET == entityHandlerRequest->method)
            {
                OC_LOG (INFO, TAG, "Received OC_REST_GET from client");
                ehResult = ProcessGetRequest (entityHandlerRequest, payload, sizeof(payload));
            }
            else if (OC_REST_PUT == entityHandlerRequest->method)
            {
                OC_LOG (INFO, TAG, "Received OC_REST_PUT from client");
                ehResult = ProcessPutRequest (entityHandlerRequest, payload, sizeof(payload));
            }
            else if (OC_REST_POST == entityHandlerRequest->method)
            {
                OC_LOG (INFO, TAG, "Received OC_REST_POST from client");
                ehResult = ProcessPostRequest (entityHandlerRequest, payload, sizeof(payload));
            }
            else
            {
                OC_LOG_V (INFO, TAG, "Received unsupported method %d from client",
                        entityHandlerRequest->method);
            }

            if (ehResult == OC_EH_OK)
            {
                // Format the response.  Note this requires some info about the request
                response.requestHandle = entityHandlerRequest->requestHandle;
                response.resourceHandle = entityHandlerRequest->resource;
                response.ehResult = ehResult;
                response.payload = (unsigned char *)payload;
                response.payloadSize = strlen(payload);
                response.numSendVendorSpecificHeaderOptions = 0;
                memset(response.sendVendorSpecificHeaderOptions, 0, sizeof response.sendVendorSpecificHeaderOptions);
                memset(response.resourceUri, 0, sizeof(response.resourceUri));
                // Indicate that response is NOT in a persistent buffer
                response.persistentBufferFlag = 0;

                // Send the response
                if (OCDoResponse(&response) != OC_STACK_OK)
                {
                    OC_LOG(ERROR, TAG, "Error sending response");
                    ehResult = OC_EH_ERROR;
                }
            }
        }
    }
    return ehResult;
}

/* SIGINT handler: set gQuitFlag to 1 for graceful termination */
void handleSigInt(int signum)
{
    if (signum == SIGINT)
    {
        gQuitFlag = 1;
    }
}

int main(int argc, char* argv[])
{
    uint8_t addr[20] = {0};
    uint8_t* paddr = NULL;
    uint16_t port = OC_WELL_KNOWN_PORT;
    uint8_t ifname[] = "eth0";
    struct timespec timeout;

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
     * Read DTLS PSK credentials from persistent storage and
     * set in the OC stack.
     */
    if (SetCredentials(CRED_FILE) != OC_STACK_OK)
    {
        OC_LOG(ERROR, TAG, "SetCredentials failed");
        return 0;
    }
    /*
     * Declare and create the example resource: LED
     */
    createLEDResource(gResourceUri, &LED, false, 0);

    timeout.tv_sec  = 0;
    timeout.tv_nsec = 100000000L;

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
        nanosleep(&timeout, NULL);
    }

    OC_LOG(INFO, TAG, "Exiting ocserver main loop...");

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
            OC_DISCOVERABLE|OC_OBSERVABLE | OC_SECURE);
    OC_LOG_V(INFO, TAG, "Created LED resource with result: %s", getResult(res));

    return 0;
}
