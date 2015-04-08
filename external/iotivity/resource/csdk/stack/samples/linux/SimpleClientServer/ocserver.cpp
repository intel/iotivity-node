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
#include <array>
#include "ocstack.h"
#include "logger.h"
#include "cJSON.h"
#include "ocserver.h"

//string length of "/a/light/" + std::numeric_limits<int>::digits10 + '\0'"
// 9 + 9 + 1 = 19
const int URI_MAXSIZE = 19;

static int gObserveNotifyType = 3;

int gQuitFlag = 0;
int gLightUnderObservation = 0;

static LightResource Light;
// This variable determines instance number of the Light resource.
// Used by POST method to create a new instance of Light resource.
static int gCurrLightInstance = 0;

static LightResource gLightInstance[SAMPLE_MAX_NUM_POST_INSTANCE];

Observers interestedObservers[SAMPLE_MAX_NUM_OBSERVATIONS];

#ifdef WITH_PRESENCE
static int stopPresenceCount = 10;
#define numPresenceResources (2)
#endif

//TODO: Follow the pattern used in constructJsonResponse() when the payload is decided.
const char responsePayloadDeleteOk[] = "{App determines payload: Delete Resource operation succeeded.}";
const char responsePayloadDeleteNotOK[] = "{App determines payload: Delete Resource operation failed.}";
const char responsePayloadResourceDoesNotExist[] = "{App determines payload: The resource does not exist.}";
const char responsePayloadDeleteResourceNotSupported[] =
        "{App determines payload: The request is received for a non-support resource.}";


char *gResourceUri= (char *)"/a/light";
const char *contentType = "myContentType";
const char *dateOfManufacture = "myDateOfManufacture";
const char *deviceName = "myDeviceName";
const char *deviceUUID = "myDeviceUUID";
const char *firmwareVersion = "myFirmwareVersion";
const char *hostName = "myHostName";
const char *manufacturerName = "myManufacturerNa";
const char *manufacturerUrl = "myManufacturerUrl";
const char *modelNumber = "myModelNumber";
const char *platformVersion = "myPlatformVersion";
const char *supportUrl = "mySupportUrl";
const char *version = "myVersion";

OCDeviceInfo deviceInfo;

static uint16_t OC_WELL_KNOWN_PORT = 5683;

//This function takes the request as an input and returns the response
//in JSON format.
char* constructJsonResponse (OCEntityHandlerRequest *ehRequest)
{
    cJSON *json = cJSON_CreateObject();
    cJSON *format;
    char *jsonResponse;
    LightResource *currLightResource = &Light;

    if (ehRequest->resource == gLightInstance[0].handle)
    {
        currLightResource = &gLightInstance[0];
        gResourceUri = (char *) "a/light/0";
    }
    else if (ehRequest->resource == gLightInstance[1].handle)
    {
        currLightResource = &gLightInstance[1];
        gResourceUri = (char *) "a/light/1";
    }

    if(OC_REST_PUT == ehRequest->method)
    {
        cJSON *putJson = cJSON_Parse((char *)ehRequest->reqJSONPayload);
        currLightResource->state = ( !strcmp(cJSON_GetObjectItem(putJson,"state")->valuestring,
                "on") ? true:false);
        currLightResource->power = cJSON_GetObjectItem(putJson,"power")->valuedouble;
        cJSON_Delete(putJson);
    }

    cJSON_AddStringToObject(json,"href",gResourceUri);
    cJSON_AddItemToObject(json, "rep", format=cJSON_CreateObject());
    cJSON_AddStringToObject(format, "state", (char *) (currLightResource->state ? "on":"off"));
    cJSON_AddNumberToObject(format, "power", currLightResource->power);

    jsonResponse = cJSON_Print(json);
    cJSON_Delete(json);

    return jsonResponse;
}

OCEntityHandlerResult ProcessGetRequest (OCEntityHandlerRequest *ehRequest, char *payload, uint16_t maxPayloadSize)
{
    OCEntityHandlerResult ehResult;
    char *getResp = constructJsonResponse(ehRequest);

    if (maxPayloadSize > strlen ((char *)getResp))
    {
        strncpy(payload, getResp, strlen((char *)getResp));
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

OCEntityHandlerResult ProcessPutRequest (OCEntityHandlerRequest *ehRequest, char *payload, uint16_t maxPayloadSize)
{
    OCEntityHandlerResult ehResult;
    char *putResp = constructJsonResponse(ehRequest);

    if (maxPayloadSize > strlen ((char *)putResp))
    {
        strncpy(payload, putResp, strlen((char *)putResp));
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

OCEntityHandlerResult ProcessPostRequest (OCEntityHandlerRequest *ehRequest, OCEntityHandlerResponse *response, char *payload, uint16_t maxPayloadSize)
{
    OCEntityHandlerResult ehResult = OC_EH_OK;
    char *respPLPost_light = NULL;
    cJSON *json;
    cJSON *format;

    /*
     * The entity handler determines how to process a POST request.
     * Per the REST paradigm, POST can also be used to update representation of existing
     * resource or create a new resource.
     * In the sample below, if the POST is for /a/light then a new instance of the Light
     * resource is created with default representation (if representation is included in
     * POST payload it can be used as initial values) as long as the instance is
     * lesser than max new instance count. Once max instance count is reached, POST on
     * /a/light updated the representation of /a/light (just like PUT)
     */

    if (ehRequest->resource == Light.handle)
    {
        if (gCurrLightInstance < SAMPLE_MAX_NUM_POST_INSTANCE)
        {
            // Create new Light instance
            char newLightUri[URI_MAXSIZE];
            snprintf(newLightUri, URI_MAXSIZE, "/a/light/%d", gCurrLightInstance);

            json = cJSON_CreateObject();
            cJSON_AddStringToObject(json,"href",gResourceUri);
            cJSON_AddItemToObject(json, "rep", format=cJSON_CreateObject());
            cJSON_AddStringToObject(format, "createduri", (char *) newLightUri);

            if (0 == createLightResource (newLightUri, &gLightInstance[gCurrLightInstance]))
            {
                OC_LOG (INFO, TAG, "Created new Light instance\n");
                gLightInstance[gCurrLightInstance].state = 0;
                gLightInstance[gCurrLightInstance].power = 0;
                gCurrLightInstance++;
                respPLPost_light = cJSON_Print(json);
                strncpy ((char *)response->resourceUri, newLightUri, MAX_URI_LENGTH);
                ehResult = OC_EH_RESOURCE_CREATED;
            }

            cJSON_Delete(json);
        }
        else
        {
            // Update repesentation of /a/light
            Light.state = true;
            Light.power = 11;
            respPLPost_light = constructJsonResponse(ehRequest);
        }
    }
    else
    {
        for (int i = 0; i < SAMPLE_MAX_NUM_POST_INSTANCE; i++)
        {
            if (ehRequest->resource == gLightInstance[i].handle)
            {
                gLightInstance[i].state = true;
                gLightInstance[i].power = 22;
                if (i == 0)
                {
                    respPLPost_light = constructJsonResponse(ehRequest);
                    break;
                }
                else if (i == 1)
                {
                    respPLPost_light = constructJsonResponse(ehRequest);
                }
            }
        }
    }

    if ((respPLPost_light != NULL) && (maxPayloadSize > strlen ((char *)respPLPost_light)))
    {
        strncpy(payload, respPLPost_light, strlen((char *)respPLPost_light));
    }
    else
    {
        OC_LOG_V (INFO, TAG, "Response buffer: %d bytes is too small",
                maxPayloadSize);
        ehResult = OC_EH_ERROR;
    }

    free(respPLPost_light);
    return ehResult;
}

OCEntityHandlerResult ProcessDeleteRequest (OCEntityHandlerRequest *ehRequest, char *payload, uint16_t maxPayloadSize)
{
    if(ehRequest == NULL)
    {
        OC_LOG(INFO, TAG, "The ehRequest is NULL");
        return OC_EH_ERROR;
    }
    OCEntityHandlerResult ehResult = OC_EH_OK;

    OC_LOG_V(INFO, TAG, "\n\nExecuting %s for resource %d ", __func__, ehRequest->resource);

    /*
     * In the sample below, the application will:
     * 1a. pass the delete request to the c stack
     * 1b. internally, the c stack figures out what needs to be done and does it accordingly
     *    (e.g. send observers notification, remove observers...)
     * 1c. the c stack returns with the result whether the request is fullfilled.
     * 2. optionally, app removes observers out of its array 'interestedObservers'
     */

    const char* deleteResponse = NULL;

    if ((ehRequest != NULL) && (ehRequest->resource == Light.handle))
    {
        //Step 1: Ask stack to do the work.
        OCStackResult result = OCDeleteResource(ehRequest->resource);

        if (result == OC_STACK_OK)
        {
            OC_LOG (INFO, TAG, "\n\nDelete Resource operation succeeded.");
            ehResult = OC_EH_OK;
            deleteResponse = responsePayloadDeleteOk;

            //Step 2: clear observers who wanted to observe this resource at the app level.
            for (uint8_t i = 0; i < SAMPLE_MAX_NUM_OBSERVATIONS; i++)
            {
                if (interestedObservers[i].resourceHandle == ehRequest->resource)
                {
                    interestedObservers[i].valid = false;
                    interestedObservers[i].observationId = 0;
                    interestedObservers[i].resourceHandle = NULL;
                }
            }
        }
        else if (result == OC_STACK_NO_RESOURCE)
        {
            OC_LOG(INFO, TAG, "\n\nThe resource doesn't exist or it might have been deleted.");
            deleteResponse = responsePayloadResourceDoesNotExist;
            ehResult = OC_EH_RESOURCE_DELETED;
        }
        else
        {
            OC_LOG(INFO, TAG, "\n\nEncountered error from OCDeleteResource().");
            deleteResponse = responsePayloadDeleteNotOK;
            ehResult = OC_EH_ERROR;
        }
    }
    else if (ehRequest->resource != Light.handle)
    {
        //Let's this app not supporting DELETE on some resources so
        //consider the DELETE request is received for a non-support resource.
        OC_LOG_V(INFO, TAG, "\n\nThe request is received for a non-support resource.");
        deleteResponse = responsePayloadDeleteResourceNotSupported;
        ehResult = OC_EH_FORBIDDEN;
    }

    if (maxPayloadSize > strlen ((char *)deleteResponse))
    {
        strncpy(payload, deleteResponse, strlen((char *)deleteResponse));
    }
    else
    {
        OC_LOG_V (INFO, TAG, "Response buffer: %d bytes is too small",
                maxPayloadSize);
        ehResult = OC_EH_ERROR;
    }

    return ehResult;
}

OCEntityHandlerResult ProcessNonExistingResourceRequest(OCEntityHandlerRequest *ehRequest, char *payload, uint16_t maxPayloadSize)
{
    OC_LOG_V(INFO, TAG, "\n\nExecuting %s ", __func__);

    const char* response = NULL;
    response = responsePayloadResourceDoesNotExist;

    if ( (ehRequest != NULL) &&
         (maxPayloadSize > strlen ((char *)response)) )
    {
        strncpy((char *)payload, response, strlen((char *)response));
    }
    else
    {
        OC_LOG_V (INFO, TAG, "Response buffer: %d bytes is too small",
                maxPayloadSize);
    }

    return OC_EH_RESOURCE_DELETED;
}

void ProcessObserveRegister (OCEntityHandlerRequest *ehRequest)
{
    OC_LOG_V (INFO, TAG, "Received observation registration request with observation Id %d",
            ehRequest->obsInfo.obsId);
    for (uint8_t i = 0; i < SAMPLE_MAX_NUM_OBSERVATIONS; i++)
    {
        if (interestedObservers[i].valid == false)
        {
            interestedObservers[i].observationId = ehRequest->obsInfo.obsId;
            interestedObservers[i].valid = true;
            gLightUnderObservation = 1;
            break;
        }
    }
}

void ProcessObserveDeregister (OCEntityHandlerRequest *ehRequest)
{
    bool clientStillObserving = false;

    OC_LOG_V (INFO, TAG, "Received observation deregistration request for observation Id %d",
            ehRequest->obsInfo.obsId);
    for (uint8_t i = 0; i < SAMPLE_MAX_NUM_OBSERVATIONS; i++)
    {
        if (interestedObservers[i].observationId == ehRequest->obsInfo.obsId)
        {
            interestedObservers[i].valid = false;
        }
        if (interestedObservers[i].valid == true)
        {
            // Even if there is one single client observing we continue notifying entity handler
            clientStillObserving = true;
        }
    }
    if (clientStillObserving == false)
        gLightUnderObservation = 0;
}

OCEntityHandlerResult
OCDeviceEntityHandlerCb (OCEntityHandlerFlag flag,
        OCEntityHandlerRequest *entityHandlerRequest, char* uri)
{
    OC_LOG_V (INFO, TAG, "Inside device default entity handler - flags: 0x%x, uri: %s", flag, uri);

    OCEntityHandlerResult ehResult = OC_EH_OK;
    OCEntityHandlerResponse response;
    char payload[MAX_RESPONSE_LENGTH] = {0};

    // Validate pointer
    if (!entityHandlerRequest)
    {
        OC_LOG (ERROR, TAG, "Invalid request pointer");
        return OC_EH_ERROR;
    }

    // Initialize certain response fields
    response.numSendVendorSpecificHeaderOptions = 0;
    memset(response.sendVendorSpecificHeaderOptions, 0, sizeof response.sendVendorSpecificHeaderOptions);
    memset(response.resourceUri, 0, sizeof response.resourceUri);

    if (flag & OC_INIT_FLAG)
    {
        OC_LOG (INFO, TAG, "Flag includes OC_INIT_FLAG");
    }
    if (flag & OC_REQUEST_FLAG)
    {
        OC_LOG (INFO, TAG, "Flag includes OC_REQUEST_FLAG");
        if (entityHandlerRequest->resource == NULL) {
            OC_LOG (INFO, TAG, "Received request from client to a non-existing resource");
            ehResult = ProcessNonExistingResourceRequest(entityHandlerRequest, payload, sizeof(payload) - 1);
        }
        else if (OC_REST_GET == entityHandlerRequest->method)
        {
            OC_LOG (INFO, TAG, "Received OC_REST_GET from client");
            ehResult = ProcessGetRequest (entityHandlerRequest, payload, sizeof(payload) - 1);
        }
        else if (OC_REST_PUT == entityHandlerRequest->method)
        {
            OC_LOG (INFO, TAG, "Received OC_REST_PUT from client");
            ehResult = ProcessPutRequest (entityHandlerRequest, payload, sizeof(payload) - 1);
        }
        else if (OC_REST_DELETE == entityHandlerRequest->method)
        {
            OC_LOG (INFO, TAG, "Received OC_REST_DELETE from client");
            ehResult = ProcessDeleteRequest (entityHandlerRequest, payload, sizeof(payload) - 1);
        }
        else
        {
            OC_LOG_V (INFO, TAG, "Received unsupported method %d from client",
                    entityHandlerRequest->method);
            ehResult = OC_EH_ERROR;
        }

        // If the result isn't an error or forbidden, send response
        if (!((ehResult == OC_EH_ERROR) || (ehResult == OC_EH_FORBIDDEN)))
        {
            // Format the response.  Note this requires some info about the request
            response.requestHandle = entityHandlerRequest->requestHandle;
            response.resourceHandle = entityHandlerRequest->resource;
            response.ehResult = ehResult;
            response.payload = (unsigned char *)payload;
            response.payloadSize = strlen(payload);
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
    if (flag & OC_OBSERVE_FLAG)
    {
        OC_LOG(INFO, TAG, "Flag includes OC_OBSERVE_FLAG");
        if (OC_OBSERVE_REGISTER == entityHandlerRequest->obsInfo.action)
        {
            OC_LOG (INFO, TAG, "Received OC_OBSERVE_REGISTER from client");
        }
        else if (OC_OBSERVE_DEREGISTER == entityHandlerRequest->obsInfo.action)
        {
            OC_LOG (INFO, TAG, "Received OC_OBSERVE_DEREGISTER from client");
        }
    }

    return ehResult;
}

OCEntityHandlerResult
OCNOPEntityHandlerCb (OCEntityHandlerFlag flag,
        OCEntityHandlerRequest *entityHandlerRequest)
{
    // This is callback is associated with the 2 presence notification
    // resources. They are non-operational.
    return OC_EH_OK;
}

OCEntityHandlerResult
OCEntityHandlerCb (OCEntityHandlerFlag flag,
        OCEntityHandlerRequest *entityHandlerRequest)
{
    OC_LOG_V (INFO, TAG, "Inside entity handler - flags: 0x%x", flag);

    OCEntityHandlerResult ehResult = OC_EH_OK;
    OCEntityHandlerResponse response;
    char payload[MAX_RESPONSE_LENGTH] = {0};

    // Validate pointer
    if (!entityHandlerRequest)
    {
        OC_LOG (ERROR, TAG, "Invalid request pointer");
        return OC_EH_ERROR;
    }

    // Initialize certain response fields
    response.numSendVendorSpecificHeaderOptions = 0;
    memset(response.sendVendorSpecificHeaderOptions, 0, sizeof response.sendVendorSpecificHeaderOptions);
    memset(response.resourceUri, 0, sizeof response.resourceUri);

    if (flag & OC_INIT_FLAG)
    {
        OC_LOG (INFO, TAG, "Flag includes OC_INIT_FLAG");
    }
    if (flag & OC_REQUEST_FLAG)
    {
        OC_LOG (INFO, TAG, "Flag includes OC_REQUEST_FLAG");
        if (OC_REST_GET == entityHandlerRequest->method)
        {
            OC_LOG (INFO, TAG, "Received OC_REST_GET from client");
            ehResult = ProcessGetRequest (entityHandlerRequest, payload, sizeof(payload) - 1);
        }
        else if (OC_REST_PUT == entityHandlerRequest->method)
        {
            OC_LOG (INFO, TAG, "Received OC_REST_PUT from client");
            ehResult = ProcessPutRequest (entityHandlerRequest, payload, sizeof(payload) - 1);
        }
        else if (OC_REST_POST == entityHandlerRequest->method)
        {
            OC_LOG (INFO, TAG, "Received OC_REST_POST from client");
            ehResult = ProcessPostRequest (entityHandlerRequest, &response, payload, sizeof(payload) - 1);
        }
        else if (OC_REST_DELETE == entityHandlerRequest->method)
        {
            OC_LOG (INFO, TAG, "Received OC_REST_DELETE from client");
            ehResult = ProcessDeleteRequest (entityHandlerRequest, payload, sizeof(payload) - 1);
        }
        else
        {
            OC_LOG_V (INFO, TAG, "Received unsupported method %d from client",
                    entityHandlerRequest->method);
        }

        // If the result isn't an error or forbidden, send response
        if (!((ehResult == OC_EH_ERROR) || (ehResult == OC_EH_FORBIDDEN)))
        {
            // Format the response.  Note this requires some info about the request
            response.requestHandle = entityHandlerRequest->requestHandle;
            response.resourceHandle = entityHandlerRequest->resource;
            response.ehResult = ehResult;
            response.payload = (unsigned char *)payload;
            response.payloadSize = strlen(payload);
            // Indicate that response is NOT in a persistent buffer
            response.persistentBufferFlag = 0;

            // Handle vendor specific options
            if(entityHandlerRequest->rcvdVendorSpecificHeaderOptions &&
                    entityHandlerRequest->numRcvdVendorSpecificHeaderOptions)
            {
                OC_LOG (INFO, TAG, "Received vendor specific options");
                uint8_t i = 0;
                OCHeaderOption * rcvdOptions = entityHandlerRequest->rcvdVendorSpecificHeaderOptions;
                for( i = 0; i < entityHandlerRequest->numRcvdVendorSpecificHeaderOptions; i++)
                {
                    if(((OCHeaderOption)rcvdOptions[i]).protocolID == OC_COAP_ID)
                    {
                        OC_LOG_V(INFO, TAG, "Received option with OC_COAP_ID and ID %u with",
                                ((OCHeaderOption)rcvdOptions[i]).optionID );
                        OC_LOG_BUFFER(INFO, TAG, ((OCHeaderOption)rcvdOptions[i]).optionData,
                                ((OCHeaderOption)rcvdOptions[i]).optionLength);
                    }
                }
                OCHeaderOption * sendOptions = response.sendVendorSpecificHeaderOptions;
                uint8_t option2[] = {21,22,23,24,25,26,27,28,29,30};
                uint8_t option3[] = {31,32,33,34,35,36,37,38,39,40};
                sendOptions[0].protocolID = OC_COAP_ID;
                sendOptions[0].optionID = 2248;
                memcpy(sendOptions[0].optionData, option2, sizeof(option2));
                sendOptions[0].optionLength = 10;
                sendOptions[1].protocolID = OC_COAP_ID;
                sendOptions[1].optionID = 2600;
                memcpy(sendOptions[1].optionData, option3, sizeof(option3));
                sendOptions[1].optionLength = 10;
                response.numSendVendorSpecificHeaderOptions = 2;
            }

            // Send the response
            if (OCDoResponse(&response) != OC_STACK_OK)
            {
                OC_LOG(ERROR, TAG, "Error sending response");
                ehResult = OC_EH_ERROR;
            }
        }
    }
    if (flag & OC_OBSERVE_FLAG)
    {
        OC_LOG(INFO, TAG, "Flag includes OC_OBSERVE_FLAG");

        if (OC_OBSERVE_REGISTER == entityHandlerRequest->obsInfo.action)
        {
            OC_LOG (INFO, TAG, "Received OC_OBSERVE_REGISTER from client");
            ProcessObserveRegister (entityHandlerRequest);
        }
        else if (OC_OBSERVE_DEREGISTER == entityHandlerRequest->obsInfo.action)
        {
            OC_LOG (INFO, TAG, "Received OC_OBSERVE_DEREGISTER from client");
            ProcessObserveDeregister (entityHandlerRequest);
        }
    }

    return ehResult;
}

/* SIGINT handler: set gQuitFlag to 1 for graceful termination */
void handleSigInt(int signum) {
    if (signum == SIGINT) {
        gQuitFlag = 1;
    }
}

void *ChangeLightRepresentation (void *param)
{
    (void)param;
    OCStackResult result = OC_STACK_ERROR;

    uint8_t j = 0;
    uint8_t numNotifies = (SAMPLE_MAX_NUM_OBSERVATIONS)/2;
    OCObservationId obsNotify[numNotifies];

    while (!gQuitFlag)
    {
        sleep(10);
        Light.power += 5;
        if (gLightUnderObservation)
        {
            OC_LOG_V(INFO, TAG, " =====> Notifying stack of new power level %d\n", Light.power);
            if (gObserveNotifyType == 1)
            {
                // Notify list of observers. Alternate observers on the list will be notified.
                j = 0;
                for (uint8_t i = 0; i < SAMPLE_MAX_NUM_OBSERVATIONS; (i=i+2))
                {
                    if (interestedObservers[i].valid == true)
                    {
                        obsNotify[j] = interestedObservers[i].observationId;
                        j++;
                    }
                }

                cJSON *json = cJSON_CreateObject();
                cJSON *format;
                cJSON_AddStringToObject(json,"href",gResourceUri);
                cJSON_AddItemToObject(json, "rep", format=cJSON_CreateObject());
                cJSON_AddStringToObject(format, "state", (char *) (Light.state ? "on":"off"));
                cJSON_AddNumberToObject(format, "power", Light.power);
                char * obsResp = cJSON_Print(json);
                cJSON_Delete(json);
                result = OCNotifyListOfObservers (Light.handle, obsNotify, j,
                        (unsigned char *)obsResp, OC_NA_QOS);
                free(obsResp);
            }
            else if (gObserveNotifyType == 0)
            {
                // Notifying all observers
                result = OCNotifyAllObservers (Light.handle, OC_NA_QOS);
                if (OC_STACK_NO_OBSERVERS == result)
                {
                    OC_LOG (INFO, TAG,
                            "=======> No more observers exist, stop sending observations");
                    gLightUnderObservation = 0;
                }
            }
            else
            {
                OC_LOG (ERROR, TAG, "Incorrect notification type selected");
            }
        }
#ifdef WITH_PRESENCE
        if(stopPresenceCount > 0)
        {
            OC_LOG_V(INFO, TAG, "================ presence count %d", stopPresenceCount);
        }
        if(!stopPresenceCount--)
        {
            OC_LOG(INFO, TAG, "================ stopping presence");
            OCStopPresence();
        }
#endif
    }
    return NULL;
}

void *presenceNotificationGenerator(void *param)
{
    sleep(5);
    (void)param;
    OCDoHandle presenceNotificationHandles[numPresenceResources];
    OCStackResult res = OC_STACK_OK;

    std::array<std::string, numPresenceResources> presenceNotificationResources { {
        std::string("core.fan"),
        std::string("core.led") } };
    std::array<std::string, numPresenceResources> presenceNotificationUris { {
        std::string("/a/fan"),
        std::string("/a/led") } };

    for(int i=0; i<numPresenceResources; i++)
    {
        if(res == OC_STACK_OK)
        {
            sleep(1);
            res = OCCreateResource(&presenceNotificationHandles[i],
                    presenceNotificationResources.at(i).c_str(),
                    "oc.mi.def",
                    presenceNotificationUris.at(i).c_str(),
                    OCNOPEntityHandlerCb,
                    OC_DISCOVERABLE|OC_OBSERVABLE);
        }
        if(res != OC_STACK_OK)
        {
            OC_LOG_V(ERROR, TAG, "\"Presence Notification Generator\" failed to create resource "
                    "%s with result %s.", presenceNotificationResources.at(i).c_str(),
                    getResult(res));
            break;
        }
    }
    sleep(5);
    for(int i=0; i<numPresenceResources; i++)
    {
        if(res == OC_STACK_OK)
        {
            res = OCDeleteResource(presenceNotificationHandles[i]);
        }
        if(res != OC_STACK_OK)
        {
            OC_LOG_V(ERROR, TAG, "\"Presence Notification Generator\" failed to delete "\
                    "resource %s.", presenceNotificationResources.at(i).c_str());
            break;
        }
    }
    return NULL;
}

static void PrintUsage()
{
    OC_LOG(INFO, TAG, "Usage : ocserver -o <0|1>");
    OC_LOG(INFO, TAG, "-o 0 : Notify all observers");
    OC_LOG(INFO, TAG, "-o 1 : Notify list of observers");
}

int main(int argc, char* argv[])
{
    uint8_t addr[20] = {0};
    uint8_t* paddr = NULL;
    uint16_t port = OC_WELL_KNOWN_PORT;
    uint8_t ifname[] = "eth0";
    pthread_t threadId;
    pthread_t threadId_presence;
    int opt;

    while ((opt = getopt(argc, argv, "o:")) != -1)
    {
        switch(opt)
        {
            case 'o':
                gObserveNotifyType = atoi(optarg);
                break;
            default:
                PrintUsage();
                return -1;
        }
    }

    if ((gObserveNotifyType != 0) && (gObserveNotifyType != 1))
    {
        PrintUsage();
        return -1;
    }

    OC_LOG(DEBUG, TAG, "OCServer is starting...");
    /*Get Ip address on defined interface and initialize coap on it with random port number
     * this port number will be used as a source port in all coap communications*/
    if ( OCGetInterfaceAddress(ifname, sizeof(ifname), AF_INET, addr,
                sizeof(addr)) == ERR_SUCCESS)
    {
        OC_LOG_V(INFO, TAG, "Starting ocserver on address %s:%d",addr,port);
        paddr = addr;
    }

    if (OCInit((char *) paddr, port, OC_SERVER) != OC_STACK_OK) {
        OC_LOG(ERROR, TAG, "OCStack init error");
        return 0;
    }
#ifdef WITH_PRESENCE
    if (OCStartPresence(0) != OC_STACK_OK) {
        OC_LOG(ERROR, TAG, "OCStack presence/discovery error");
        return 0;
    }
#endif

    OCSetDefaultDeviceEntityHandler(OCDeviceEntityHandlerCb);

    OCStackResult deviceResult = SetDeviceInfo(contentType, dateOfManufacture, deviceName,
            deviceUUID, firmwareVersion, hostName, manufacturerName,
            manufacturerUrl, modelNumber, platformVersion, supportUrl, version);

    if (deviceResult != OC_STACK_OK)
    {
        OC_LOG(INFO, TAG, "Device Registration failed!");
        exit (EXIT_FAILURE);
    }

    deviceResult = OCSetDeviceInfo(deviceInfo);

    if (deviceResult != OC_STACK_OK)
    {
        OC_LOG(INFO, TAG, "Device Registration failed!");
        exit (EXIT_FAILURE);
    }

    /*
     * Declare and create the example resource: Light
     */
    createLightResource(gResourceUri, &Light);

    // Initialize observations data structure for the resource
    for (uint8_t i = 0; i < SAMPLE_MAX_NUM_OBSERVATIONS; i++)
    {
        interestedObservers[i].valid = false;
    }

    /*
     * Create a thread for changing the representation of the Light
     */
    pthread_create (&threadId, NULL, ChangeLightRepresentation, (void *)NULL);

    /*
     * Create a thread for generating changes that cause presence notifications
     * to be sent to clients
     */
    pthread_create(&threadId_presence, NULL, presenceNotificationGenerator, (void *)NULL);

    // Break from loop with Ctrl-C
    OC_LOG(INFO, TAG, "Entering ocserver main loop...");
    DeleteDeviceInfo();
    signal(SIGINT, handleSigInt);
    while (!gQuitFlag) {
        if (OCProcess() != OC_STACK_OK) {
            OC_LOG(ERROR, TAG, "OCStack process error");
            return 0;
        }

        sleep(2);
    }

    /*
     * Cancel the Light thread and wait for it to terminate
     */
    pthread_cancel(threadId);
    pthread_join(threadId, NULL);
    pthread_cancel(threadId_presence);
    pthread_join(threadId_presence, NULL);

    OC_LOG(INFO, TAG, "Exiting ocserver main loop...");

    if (OCStop() != OC_STACK_OK) {
        OC_LOG(ERROR, TAG, "OCStack process error");
    }

    return 0;
}

int createLightResource (char *uri, LightResource *lightResource)
{
    if (!uri)
    {
        OC_LOG(ERROR, TAG, "Resource URI cannot be NULL");
        return -1;
    }

    lightResource->state = false;
    lightResource->power= 0;
    OCStackResult res = OCCreateResource(&(lightResource->handle),
            "core.light",
            "oc.mi.def",
            uri,
            OCEntityHandlerCb,
            OC_DISCOVERABLE|OC_OBSERVABLE);
    OC_LOG_V(INFO, TAG, "Created Light resource with result: %s", getResult(res));

    return 0;
}

void DeleteDeviceInfo()
{
    free(deviceInfo.contentType);
    free(deviceInfo.dateOfManufacture);
    free(deviceInfo.deviceName);
    free(deviceInfo.deviceUUID);
    free(deviceInfo.firmwareVersion);
    free(deviceInfo.hostName);
    free(deviceInfo.manufacturerName);
    free(deviceInfo.manufacturerUrl);
    free(deviceInfo.modelNumber);
    free(deviceInfo.platformVersion);
    free(deviceInfo.supportUrl);
    free(deviceInfo.version);
}

bool DuplicateString(char** targetString, const char* sourceString)
{
    if(!sourceString)
    {
        return false;
    }
    else
    {
        *targetString = (char *) malloc(strlen(sourceString) + 1);

        if(targetString)
        {
            strncpy(*targetString, sourceString, (strlen(sourceString) + 1));
            return true;
        }
    }
    return false;
}

OCStackResult SetDeviceInfo(const char *contentType, const char *dateOfManufacture,
        const char *deviceName, const char *deviceUUID, const char *firmwareVersion,
        const char *hostName, const char *manufacturerName, const char *manufacturerUrl,
        const char *modelNumber, const char *platformVersion, const char *supportUrl,
        const char *version)
{

    bool success = true;

    if(manufacturerName != NULL && (strlen(manufacturerName) > MAX_MANUFACTURER_NAME_LENGTH))
    {
        return OC_STACK_INVALID_PARAM;
    }

    if(manufacturerUrl != NULL && (strlen(manufacturerUrl) > MAX_MANUFACTURER_URL_LENGTH))
    {
        return OC_STACK_INVALID_PARAM;
    }

    if(!DuplicateString(&deviceInfo.contentType, contentType))
    {
        success = false;
    }

    if(!DuplicateString(&deviceInfo.dateOfManufacture, dateOfManufacture))
    {
        success = false;
    }

    if(!DuplicateString(&deviceInfo.deviceName, deviceName))
    {
        success = false;
    }

    if(!DuplicateString(&deviceInfo.deviceUUID, deviceUUID))
    {
        success = false;
    }

    if(!DuplicateString(&deviceInfo.firmwareVersion, firmwareVersion))
    {
        success = false;
    }

    if(!DuplicateString(&deviceInfo.hostName, hostName))
    {
        success = false;
    }

    if(!DuplicateString(&deviceInfo.manufacturerName, manufacturerName))
    {
        success = false;
    }

    if(!DuplicateString(&deviceInfo.manufacturerUrl, manufacturerUrl))
    {
        success = false;
    }

    if(!DuplicateString(&deviceInfo.modelNumber, modelNumber))
    {
        success = false;
    }

    if(!DuplicateString(&deviceInfo.platformVersion, platformVersion))
    {
        success = false;
    }

    if(!DuplicateString(&deviceInfo.supportUrl, supportUrl))
    {
        success = false;
    }

    if(!DuplicateString(&deviceInfo.version, version))
    {
        success = false;
    }

    if(success)
    {
        return OC_STACK_OK;
    }

    DeleteDeviceInfo();
    return OC_STACK_ERROR;
}
