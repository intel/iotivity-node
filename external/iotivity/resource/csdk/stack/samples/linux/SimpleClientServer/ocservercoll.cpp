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
#include <string>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <ocstack.h>
#include <logger.h>

const char *getResult(OCStackResult result);

#define TAG PCF("ocservercontainer")

volatile sig_atomic_t gQuitFlag = 0;
int gLightUnderObservation = 0;
void createResources();
typedef struct LIGHTRESOURCE{
    OCResourceHandle handle;
    bool state;
    int power;
} LightResource;

static LightResource light;

// TODO : hard coded for now, change after Sprint10
const char rspGetRoomDefault[] = "{\"href\":\"/a/room\",\"rep\":{\"name\":\"John's Room\"}}";
const char rspGetRoomCollection[] = "{\"href\":\"/a/room\"}";
// TODO : Needs to be changed to retrieve current status of room and return that in response
const char rspPutRoomDefault[] = "{\"href\":\"/a/room\",\"rep\":{\"name\":\"John's Room\"}}";
const char rspPutRoomCollection[] = "{\"href\":\"/a/room\"}";
const char rspFailureRoom[] = "{\"href\":\"/a/room\",\"rep\":{\"error\":\"ROOM_OP_FAIL\"}}";

// TODO : hard coded for now, change after Sprint4
const char rspGetLightDefault[] = "{\"href\":\"/a/light\",\"rep\":{\"state\":\"false\",\"color\":\"0\"}}";
const char rspGetLightCollection[] = "{\"href\":\"/a/light\"}";
// TODO : Needs to be changed to retrieve current status of light and return that in response
const char rspPutLightDefault[] = "{\"href\":\"/a/light\",\"rep\":{\"state\":\"true\",\"color\":\"0\"}}";
const char rspPutLightCollection[] = "{\"href\":\"/a/light\"}";
const char rspFailureLight[] = "{\"href\":\"/a/light\",\"rep\":{\"error\":\"LIGHT_OP_FAIL\"}}";


// TODO : hard coded for now, change after Sprint4
const char rspGetFanDefault[] = "{\"href\":\"/a/fan\",\"rep\":{\"state\":\"true\",\"speed\":10}}";
const char rspGetFanCollection[] = "{\"href\":\"/a/fan\"}";
// TODO : Needs to be changed to retrieve current status of fan and return that in response
const char rspPutFanDefault[] = "{\"href\":\"/a/fan\",\"rep\":{\"state\":\"false\",\"speed\":0}}";
const char rspPutFanCollection[] = "{\"href\":\"/a/fan\"}";
const char rspFailureFan[] = "{\"href\":\"/a/fan\",\"rep\":{\"error\":\"FAN_OP_FAIL\"}}";

typedef enum {
    TEST_INVALID = 0,
    TEST_DEFAULT_COLL_EH,
    TEST_APP_COLL_EH,
    MAX_TESTS
} SERVER_TEST;

void PrintUsage()
{
    OC_LOG(INFO, TAG, "Usage : ocservercoll -t <Test Case>");
    OC_LOG(INFO, TAG, "Test Case 1 : Create room resource with default collection entity handler.");
    OC_LOG(INFO, TAG, "Test Case 2 : Create room resource with application collection entity handler.");
}

unsigned static int TEST = TEST_INVALID;

static OCEntityHandlerResult
HandleCallback(OCEntityHandlerRequest * ehRequest,
               const char* opStr,
               const char* errStr,
               char *payload,
               uint16_t maxPayloadSize)
{
    OCEntityHandlerResult ret = OC_EH_OK;

    // Append opStr or errStr, after making sure there is
    // enough room in the payload
    if (strlen(opStr) < (maxPayloadSize - strlen(payload)))
    {
        strncat((char*)payload, opStr, strlen(opStr));
    }
    else if (strlen(errStr) < (maxPayloadSize - strlen(payload)))
    {
        strncat((char*)payload, errStr, strlen(errStr));
        ret = OC_EH_ERROR;
    }
    else
    {
        ret = OC_EH_ERROR;
    }

    return ret;
}

static void
PrintReceivedMsgInfo(OCEntityHandlerFlag flag, OCEntityHandlerRequest * ehRequest)
{
    const char* typeOfMessage;

    switch (flag) {
        case OC_INIT_FLAG:
            typeOfMessage = "OC_INIT_FLAG";
            break;
        case OC_REQUEST_FLAG:
            typeOfMessage = "OC_REQUEST_FLAG";
            break;
        case OC_OBSERVE_FLAG:
            typeOfMessage = "OC_OBSERVE_FLAG";
            break;
        default:
            typeOfMessage = "UNKNOWN";
    }

    OC_LOG_V(INFO, TAG, "Receiving message type: %s, method %s",
             typeOfMessage,
             (ehRequest->method == OC_REST_GET) ? "OC_REST_GET" : "OC_REST_PUT" );
}

OCEntityHandlerResult OCEntityHandlerRoomCb(OCEntityHandlerFlag flag,
                                            OCEntityHandlerRequest * ehRequest)
{
    OCEntityHandlerResult ret = OC_EH_OK;
    OCEntityHandlerResponse response;
    char payload[MAX_RESPONSE_LENGTH] = {0};

    OC_LOG_V(INFO, TAG, "Callback for Room");
    PrintReceivedMsgInfo(flag, ehRequest );

    if(ehRequest && flag == OC_REQUEST_FLAG )
    {
        std::string query = (const char*)ehRequest->query;

        if(OC_REST_GET == ehRequest->method)
        {
            if(query.find("oc.mi.def") != std::string::npos)
            {
                ret = HandleCallback(ehRequest, rspGetRoomDefault, rspFailureRoom, payload, sizeof(payload));
                if(ret != OC_EH_ERROR)
                {
                    ret = HandleCallback(ehRequest, ",", ",", payload, sizeof(payload));
                    ret = HandleCallback(ehRequest, rspGetLightCollection, rspFailureLight, payload, sizeof(payload));
                }
                if(ret != OC_EH_ERROR)
                {
                    ret = HandleCallback(ehRequest, ",", ",", payload, sizeof(payload));
                    ret = HandleCallback(ehRequest, rspGetFanCollection, rspFailureFan, payload, sizeof(payload));
                }
            }
            else if(query.find("oc.mi.ll") != std::string::npos)
            {
                ret = HandleCallback(ehRequest, rspGetRoomCollection, rspFailureRoom, payload, sizeof(payload));
                if(ret != OC_EH_ERROR)
                {
                    ret = HandleCallback(ehRequest, ",", ",", payload, sizeof(payload));
                    ret = HandleCallback(ehRequest, rspGetLightCollection, rspFailureLight, payload, sizeof(payload));
                }
                if(ret != OC_EH_ERROR)
                {
                    ret = HandleCallback(ehRequest, ",", ",", payload, sizeof(payload));
                    ret = HandleCallback(ehRequest, rspGetFanCollection, rspFailureFan, payload, sizeof(payload));
                }
            }
            else if(query.find("oc.mi.b") != std::string::npos)
            {
                ret = HandleCallback(ehRequest, rspGetRoomCollection, rspFailureRoom, payload, sizeof(payload));
                if(ret != OC_EH_ERROR)
                {
                    ret = HandleCallback(ehRequest, ",", ",", payload, sizeof(payload));
                    ret = HandleCallback(ehRequest, rspGetLightDefault, rspFailureLight, payload, sizeof(payload));
                }
                if(ret != OC_EH_ERROR)
                {
                    ret = HandleCallback(ehRequest, ",", ",", payload, sizeof(payload));
                    ret = HandleCallback(ehRequest, rspGetFanDefault, rspFailureFan, payload, sizeof(payload));
                }
            }
            if (ret == OC_EH_OK)
            {
                // Format the response.  Note this requires some info about the request
                response.requestHandle = ehRequest->requestHandle;
                response.resourceHandle = ehRequest->resource;
                response.ehResult = ret;
                response.payload = (unsigned char *)payload;
                response.payloadSize = strlen(payload);
                response.numSendVendorSpecificHeaderOptions = 0;
                memset(response.sendVendorSpecificHeaderOptions, 0, sizeof response.sendVendorSpecificHeaderOptions);
                memset(response.resourceUri, 0, sizeof response.resourceUri);
                // Indicate that response is NOT in a persistent buffer
                response.persistentBufferFlag = 0;
                // Send the response
                if (OCDoResponse(&response) != OC_STACK_OK)
                {
                    OC_LOG(ERROR, TAG, "Error sending response");
                    ret = OC_EH_ERROR;
                }
            }
        }
        else if(OC_REST_PUT == ehRequest->method)
        {
            if(query.find("oc.mi.def") != std::string::npos)
            {
                if(ret != OC_EH_ERROR)
                {
                    ret = HandleCallback(ehRequest, rspPutRoomDefault, rspFailureRoom, payload, sizeof(payload));
                }
            }
            if(query.find("oc.mi.ll") != std::string::npos)
            {
                if(ret != OC_EH_ERROR)
                {
                    ret = HandleCallback(ehRequest, rspPutRoomCollection, rspFailureRoom, payload, sizeof(payload));
                }
                if(ret != OC_EH_ERROR)
                {
                    ret = HandleCallback(ehRequest, ",", ",", payload, sizeof(payload));
                    ret = HandleCallback(ehRequest, rspPutLightCollection, rspFailureLight, payload, sizeof(payload));
                }
                if(ret != OC_EH_ERROR)
                {
                    ret = HandleCallback(ehRequest, ",", ",", payload, sizeof(payload));
                    ret = HandleCallback(ehRequest, rspPutFanCollection, rspFailureFan, payload, sizeof(payload));
                }
            }
            if(query.find("oc.mi.b") != std::string::npos)
            {
                if(ret != OC_EH_ERROR)
                {
                    ret = HandleCallback(ehRequest, rspPutRoomCollection, rspFailureRoom, payload, sizeof(payload));
                }
                if(ret != OC_EH_ERROR)
                {
                    ret = HandleCallback(ehRequest, ",", ",", payload, sizeof(payload));
                    ret = HandleCallback(ehRequest, rspPutLightDefault, rspFailureLight, payload, sizeof(payload));
                }
                if(ret != OC_EH_ERROR)
                {
                    ret = HandleCallback(ehRequest, ",", ",", payload, sizeof(payload));
                    ret = HandleCallback(ehRequest, rspPutFanDefault, rspFailureFan, payload, sizeof(payload));
                }
            }
            if (ret == OC_EH_OK)
            {
                // Format the response.  Note this requires some info about the request
                response.requestHandle = ehRequest->requestHandle;
                response.resourceHandle = ehRequest->resource;
                response.ehResult = ret;
                response.payload = (unsigned char *)payload;
                response.payloadSize = strlen(payload);
                response.numSendVendorSpecificHeaderOptions = 0;
                memset(response.sendVendorSpecificHeaderOptions, 0, sizeof response.sendVendorSpecificHeaderOptions);
                memset(response.resourceUri, 0, sizeof response.resourceUri);
                // Indicate that response is NOT in a persistent buffer
                response.persistentBufferFlag = 0;
                // Send the response
                if (OCDoResponse(&response) != OC_STACK_OK)
                {
                    OC_LOG(ERROR, TAG, "Error sending response");
                    ret = OC_EH_ERROR;
                }
            }
        }
        else
        {
            OC_LOG_V (INFO, TAG, "Received unsupported method %d from client",
                    ehRequest->method);
            ret = OC_EH_ERROR;
        }
    }
    else if (ehRequest && flag == OC_OBSERVE_FLAG)
    {
        gLightUnderObservation = 1;
    }
    return ret;
}

OCEntityHandlerResult OCEntityHandlerLightCb(OCEntityHandlerFlag flag, OCEntityHandlerRequest * ehRequest)
{
    OCEntityHandlerResult ret = OC_EH_OK;
    OCEntityHandlerResponse response;
    char payload[MAX_RESPONSE_LENGTH] = {0};

    OC_LOG_V(INFO, TAG, "Callback for Light");
    PrintReceivedMsgInfo(flag, ehRequest );

    if(ehRequest && flag == OC_REQUEST_FLAG)
    {
        if(OC_REST_GET == ehRequest->method)
        {
            ret = HandleCallback(ehRequest, rspGetLightDefault, rspFailureLight, payload, sizeof(payload));
        }
        else if(OC_REST_PUT == ehRequest->method)
        {
            ret = HandleCallback(ehRequest, rspPutLightDefault, rspFailureLight, payload, sizeof(payload));
        }
        else
        {
            OC_LOG_V (INFO, TAG, "Received unsupported method %d from client",
                    ehRequest->method);
            ret = OC_EH_ERROR;
        }

        if (ret == OC_EH_OK)
        {
            // Format the response.  Note this requires some info about the request
            response.requestHandle = ehRequest->requestHandle;
            response.resourceHandle = ehRequest->resource;
            response.ehResult = ret;
            response.payload = (unsigned char *)payload;
            response.payloadSize = strlen(payload);
            response.numSendVendorSpecificHeaderOptions = 0;
            memset(response.sendVendorSpecificHeaderOptions, 0, sizeof response.sendVendorSpecificHeaderOptions);
            memset(response.resourceUri, 0, sizeof response.resourceUri);
            // Indicate that response is NOT in a persistent buffer
            response.persistentBufferFlag = 0;

            // Send the response
            if (OCDoResponse(&response) != OC_STACK_OK)
            {
                OC_LOG(ERROR, TAG, "Error sending response");
                ret = OC_EH_ERROR;
            }
        }
    }
    else if (ehRequest && flag == OC_OBSERVE_FLAG)
    {
        gLightUnderObservation = 1;
    }

    return ret;
}

OCEntityHandlerResult OCEntityHandlerFanCb(OCEntityHandlerFlag flag, OCEntityHandlerRequest * ehRequest)
{
    OCEntityHandlerResult ret = OC_EH_OK;
    OCEntityHandlerResponse response;
    char payload[MAX_RESPONSE_LENGTH] = {0};

    OC_LOG_V(INFO, TAG, "Callback for Fan");
    PrintReceivedMsgInfo(flag, ehRequest );

    if(ehRequest && flag == OC_REQUEST_FLAG)
    {
        if(OC_REST_GET == ehRequest->method)
        {
            ret = HandleCallback(ehRequest, rspGetFanDefault, rspFailureFan, payload, sizeof(payload));
        }
        else if(OC_REST_PUT == ehRequest->method)
        {
            ret = HandleCallback(ehRequest, rspPutFanDefault, rspFailureFan, payload, sizeof(payload));
        }
        else
        {
            OC_LOG_V (INFO, TAG, "Received unsupported method %d from client",
                    ehRequest->method);
            ret = OC_EH_ERROR;
        }

        if (ret == OC_EH_OK)
        {
            // Format the response.  Note this requires some info about the request
            response.requestHandle = ehRequest->requestHandle;
            response.resourceHandle = ehRequest->resource;
            response.ehResult = ret;
            response.payload = (unsigned char *)payload;
            response.payloadSize = strlen(payload);
            response.numSendVendorSpecificHeaderOptions = 0;
            memset(response.sendVendorSpecificHeaderOptions, 0, sizeof response.sendVendorSpecificHeaderOptions);
            memset(response.resourceUri, 0, sizeof response.resourceUri);
            // Indicate that response is NOT in a persistent buffer
            response.persistentBufferFlag = 0;

            // Send the response
            if (OCDoResponse(&response) != OC_STACK_OK)
            {
                OC_LOG(ERROR, TAG, "Error sending response");
                ret = OC_EH_ERROR;
            }
        }

    }
    else if (ehRequest && flag == OC_OBSERVE_FLAG)
    {
        gLightUnderObservation = 1;
    }

    return ret;
}

/* SIGINT handler: set gQuitFlag to 1 for graceful termination */
void handleSigInt(int signum)
{
    if (signum == SIGINT)
    {
        gQuitFlag = 1;
    }
}

void *ChangeLightRepresentation (void *param)
{
    (void)param;
    OCStackResult result = OC_STACK_ERROR;

    while (!gQuitFlag)
    {
        sleep(10);
        light.power += 5;
        if (gLightUnderObservation)
        {
            OC_LOG_V(INFO, TAG,
                " =====> Notifying stack of new power level %d\n", light.power);
            result = OCNotifyAllObservers (light.handle, OC_NA_QOS);
            if (OC_STACK_NO_OBSERVERS == result)
            {
                gLightUnderObservation = 0;
            }
        }
    }
    return NULL;
}

int main(int argc, char* argv[])
{
    uint8_t addr[20] = {0};
    uint8_t* paddr = NULL;
    uint16_t port = 0;
    uint8_t ifname[] = "eth0";
    pthread_t threadId;
    int opt;

    while ((opt = getopt(argc, argv, "t:")) != -1)
    {
        switch(opt)
        {
        case 't':
            TEST = atoi(optarg);
            break;
        default:
            PrintUsage();
            return -1;
        }
    }
    if(TEST <= TEST_INVALID || TEST >= MAX_TESTS){
        PrintUsage();
        return -1;
    }

    OC_LOG(DEBUG, TAG, "OCServer is starting...");
    /*Get Ip address on defined interface and initialize coap on it with random port number
     * this port number will be used as a source port in all coap communications*/
    if (OCGetInterfaceAddress(ifname, sizeof(ifname), AF_INET, addr,
                               sizeof(addr)) == ERR_SUCCESS)
    {
        OC_LOG_V(INFO, TAG, "Starting ocserver on address %s:%d",addr,port);
        paddr = addr;
    }

    if (OCInit((char *) paddr, port, OC_SERVER) != OC_STACK_OK) {
        OC_LOG(ERROR, TAG, "OCStack init error");
        return 0;
    }

    /*
     * Declare and create the example resource: light
     */
    createResources();

    /*
     * Create a thread for changing the representation of the light
     */
    pthread_create (&threadId, NULL, ChangeLightRepresentation, (void *)NULL);

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

    /*
     * Cancel the light thread and wait for it to terminate
     */
    pthread_cancel(threadId);
    pthread_join(threadId, NULL);

    OC_LOG(INFO, TAG, "Exiting ocserver main loop...");

    if (OCStop() != OC_STACK_OK)
    {
        OC_LOG(ERROR, TAG, "OCStack process error");
    }

    return 0;
}
void createResources()
{
    light.state = false;

    OCResourceHandle fan;
    OCStackResult res = OCCreateResource(&fan,
            "core.fan",
            "oc.mi.def",
            "/a/fan",
            OCEntityHandlerFanCb,
            OC_DISCOVERABLE|OC_OBSERVABLE);
    OC_LOG_V(INFO, TAG, "Created fan resource with result: %s", getResult(res));

    OCResourceHandle light;
    res = OCCreateResource(&light,
            "core.light",
            "oc.mi.def",
            "/a/light",
            OCEntityHandlerLightCb,
            OC_DISCOVERABLE|OC_OBSERVABLE);
    OC_LOG_V(INFO, TAG, "Created light resource with result: %s", getResult(res));

    OCResourceHandle room;

    if(TEST == TEST_APP_COLL_EH)
    {
        res = OCCreateResource(&room,
                "core.room",
                "oc.mi.b",
                "/a/room",
                OCEntityHandlerRoomCb,
                OC_DISCOVERABLE);
    }
    else
    {
        res = OCCreateResource(&room,
                "core.room",
                "oc.mi.b",
                "/a/room",
                NULL,
                OC_DISCOVERABLE);
    }

    OC_LOG_V(INFO, TAG, "Created room resource with result: %s", getResult(res));
    OCBindResourceInterfaceToResource(room, "oc.mi.ll");
    OCBindResourceInterfaceToResource(room, "oc.mi.def");

    res = OCBindResource(room, light);
    OC_LOG_V(INFO, TAG, "OC Bind Contained Resource to resource: %s", getResult(res));

    res = OCBindResource(room, fan);
    OC_LOG_V(INFO, TAG, "OC Bind Contained Resource to resource: %s", getResult(res));
}
