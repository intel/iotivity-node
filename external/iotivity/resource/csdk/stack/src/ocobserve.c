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

#include <string.h>
#include "ocstack.h"
#include "ocstackconfig.h"
#include "ocstackinternal.h"
#include "ocobserve.h"
#include "ocresourcehandler.h"
#include "occoap.h"
#include "utlist.h"
#include "debug.h"
#include "ocrandom.h"
#include "ocmalloc.h"
#include "ocserverrequest.h"

// Module Name
#define MOD_NAME PCF("ocobserve")

#define TAG  PCF("OCStackObserve")

#define VERIFY_NON_NULL(arg) { if (!arg) {OC_LOG(FATAL, TAG, #arg " is NULL"); goto exit;} }

static struct ResourceObserver * serverObsList = NULL;

// send notifications based on the qos of the request
// The qos passed as a parameter overrides what the client requested
// If we want the client preference taking high priority make:
// qos = resourceObserver->qos;
OCQualityOfService DetermineObserverQoS(OCMethod method, ResourceObserver * resourceObserver,
        OCQualityOfService appQoS)
{
    OCQualityOfService decidedQoS = appQoS;
    if(appQoS == OC_NA_QOS)
    {
        decidedQoS = resourceObserver->qos;
    }

    if(appQoS != OC_HIGH_QOS)
    {
        OC_LOG_V(INFO, TAG, "Current NON count for this observer is %d",
                resourceObserver->lowQosCount);
        #ifdef WITH_PRESENCE
        if((resourceObserver->forceHighQos \
                || resourceObserver->lowQosCount >= MAX_OBSERVER_NON_COUNT) \
                && method != OC_REST_PRESENCE)
        #else
        if(resourceObserver->forceHighQos \
                || resourceObserver->lowQosCount >= MAX_OBSERVER_NON_COUNT)
        #endif
            {
            resourceObserver->lowQosCount = 0;
            // at some point we have to to send CON to check on the
            // availability of observer
            OC_LOG(INFO, TAG, PCF("This time we are sending the  notification as High qos"));
            decidedQoS = OC_HIGH_QOS;
            }
        else
        {
            (resourceObserver->lowQosCount)++;
        }
    }
    return decidedQoS;
}

#ifdef WITH_PRESENCE
OCStackResult SendAllObserverNotification (OCMethod method, OCResource *resPtr, uint32_t maxAge,
        OCResourceType *resourceType, OCQualityOfService qos)
#else
OCStackResult SendAllObserverNotification (OCMethod method, OCResource *resPtr, uint32_t maxAge,
        OCQualityOfService qos)
#endif
{
    OC_LOG(INFO, TAG, PCF("Entering SendObserverNotification"));
    OCStackResult result = OC_STACK_ERROR;
    ResourceObserver * resourceObserver = serverObsList;
    uint8_t numObs = 0;
    OCServerRequest * request = NULL;
    OCEntityHandlerRequest ehRequest = {0};
    OCEntityHandlerResult ehResult = OC_EH_ERROR;

    // Find clients that are observing this resource
    while (resourceObserver)
    {
        if (resourceObserver->resource == resPtr)
        {
            numObs++;
            #ifdef WITH_PRESENCE
            if(method != OC_REST_PRESENCE)
            {
            #endif
                qos = DetermineObserverQoS(method, resourceObserver, qos);
                result = AddServerRequest(&request, 0, 0, 0, 1, OC_REST_GET,
                        0, resPtr->sequenceNum, qos, resourceObserver->query,
                        NULL, NULL,
                        &resourceObserver->token, resourceObserver->addr,
                        resourceObserver->resUri, 0);
                request->observeResult = OC_STACK_OK;
                if(request && result == OC_STACK_OK)
                {
                    result = FormOCEntityHandlerRequest(&ehRequest, (OCRequestHandle) request,
                                request->method, (OCResourceHandle) resPtr, request->query,
                                request->reqJSONPayload, request->numRcvdVendorSpecificHeaderOptions,
                                request->rcvdVendorSpecificHeaderOptions, OC_OBSERVE_NO_OPTION, 0);
                    if(result == OC_STACK_OK)
                    {
                        ehResult = resPtr->entityHandler(OC_REQUEST_FLAG, &ehRequest);
                        if(ehResult == OC_EH_ERROR)
                        {
                            FindAndDeleteServerRequest(request);
                        }
                    }
                }
            #ifdef WITH_PRESENCE
            }
            else
            {
                OCEntityHandlerResponse ehResponse = {0};
                unsigned char presenceResBuf[MAX_RESPONSE_LENGTH] = {0};
                //This is effectively the implementation for the presence entity handler.
                OC_LOG(DEBUG, TAG, PCF("This notification is for Presence"));
                result = AddServerRequest(&request, 0, 0, 0, 1, OC_REST_GET,
                        0, OC_OBSERVE_NO_OPTION, OC_LOW_QOS,
                        NULL, NULL, NULL, &resourceObserver->token,
                        resourceObserver->addr, resourceObserver->resUri, 0);
                if(result == OC_STACK_OK)
                {
                    // we create the payload here
                    if(resourceType && resourceType->resourcetypename)
                    {
                        snprintf((char *)presenceResBuf, sizeof(presenceResBuf), "%u:%u:%s",
                                resPtr->sequenceNum, maxAge, resourceType->resourcetypename);
                    }
                    else
                    {
                        snprintf((char *)presenceResBuf, sizeof(presenceResBuf), "%u:%u",
                                resPtr->sequenceNum, maxAge);
                    }
                    ehResponse.ehResult = OC_EH_OK;
                    ehResponse.payload = presenceResBuf;
                    ehResponse.payloadSize = strlen((const char *)presenceResBuf) + 1;
                    ehResponse.persistentBufferFlag = 0;
                    ehResponse.requestHandle = (OCRequestHandle) request;
                    ehResponse.resourceHandle = (OCResourceHandle) resPtr;
                    strcpy((char *)ehResponse.resourceUri, (const char *)resourceObserver->resUri);
                    result = OCDoResponse(&ehResponse);
                }
            }
            #endif
        }
        resourceObserver = resourceObserver->next;
    }
    if (numObs == 0)
    {
        OC_LOG(INFO, TAG, PCF("Resource has no observers"));
        result = OC_STACK_NO_OBSERVERS;
    }
    return result;
}

OCStackResult SendListObserverNotification (OCResource * resource,
        OCObservationId  *obsIdList, uint8_t numberOfIds,
        unsigned char *notificationJSONPayload, uint32_t maxAge,
        OCQualityOfService qos)
{
    uint8_t numIds = numberOfIds;
    ResourceObserver *observation = NULL;
    uint8_t numSentNotification = 0;
    OCServerRequest * request = NULL;
    OCStackResult result = OC_STACK_ERROR;
    OCEntityHandlerResponse ehResponse = {0};

    OC_LOG(INFO, TAG, PCF("Entering SendListObserverNotification"));
    while(numIds)
    {
        OC_LOG_V(INFO, TAG, "Need to notify observation id %d", *obsIdList);
        observation = NULL;
        observation = GetObserverUsingId (*obsIdList);
        if(observation)
        {
            // Found observation - verify if it matches the resource handle
            if (observation->resource == resource)
            {
                qos = DetermineObserverQoS(OC_REST_GET, observation, qos);

                result = AddServerRequest(&request, 0, 0, 0, 1, OC_REST_GET,
                        0, resource->sequenceNum, qos, observation->query,
                        NULL, NULL, &observation->token,
                        observation->addr, observation->resUri, 0);
                request->observeResult = OC_STACK_OK;
                if(request && result == OC_STACK_OK)
                {
                    memset(&ehResponse, 0, sizeof(OCEntityHandlerResponse));
                    ehResponse.ehResult = OC_EH_OK;
                    ehResponse.payload = (unsigned char *) OCMalloc(MAX_RESPONSE_LENGTH);
                    if(!ehResponse.payload)
                    {
                        FindAndDeleteServerRequest(request);
                        continue;
                    }
                    strcpy((char *)ehResponse.payload, (const char *)notificationJSONPayload);
                    ehResponse.payloadSize = strlen((const char *)ehResponse.payload) + 1;
                    ehResponse.persistentBufferFlag = 0;
                    ehResponse.requestHandle = (OCRequestHandle) request;
                    ehResponse.resourceHandle = (OCResourceHandle) resource;
                    result = OCDoResponse(&ehResponse);
                    if(result == OC_STACK_OK)
                    {
                        OCFree(ehResponse.payload);
                        FindAndDeleteServerRequest(request);
                    }
                }
                else
                {
                    FindAndDeleteServerRequest(request);
                }

                numSentNotification++;
            }
        }
        obsIdList++;
        numIds--;
    }
    if(numSentNotification == numberOfIds)
    {
        return OC_STACK_OK;
    }
    else if(numSentNotification == 0)
    {
        return OC_STACK_NO_OBSERVERS;
    }
    else
    {
        //TODO: we need to signal that not every one in the
        // list got an update, should we also indicate who did not receive on?
        return OC_STACK_OK;
    }
}

OCStackResult GenerateObserverId (OCObservationId *observationId)
{
    ResourceObserver *resObs = NULL;

    OC_LOG(INFO, TAG, PCF("Entering GenerateObserverId"));
    VERIFY_NON_NULL (observationId);

    do
    {
        *observationId = OCGetRandomByte();
        // Check if observation Id already exists
        resObs = GetObserverUsingId (*observationId);
    } while (NULL != resObs);

    OC_LOG_V(INFO, TAG, "Observation ID is %u", *observationId);

    return OC_STACK_OK;
exit:
    return OC_STACK_ERROR;
}

OCStackResult AddObserver (const char         *resUri,
                           const char         *query,
                           OCObservationId    obsId,
                           OCCoAPToken        *token,
                           OCDevAddr          *addr,
                           OCResource         *resHandle,
                           OCQualityOfService qos)
{
    ResourceObserver *obsNode = NULL;

    obsNode = (ResourceObserver *) OCCalloc(1, sizeof(ResourceObserver));
    if (obsNode)
    {
        obsNode->observeId = obsId;

        obsNode->resUri = (unsigned char *)OCMalloc(strlen(resUri)+1);
        VERIFY_NON_NULL (obsNode->resUri);
        memcpy (obsNode->resUri, resUri, strlen(resUri)+1);

        obsNode->qos = qos;
        if(query)
        {
            obsNode->query = (unsigned char *)OCMalloc(strlen(query)+1);
            VERIFY_NON_NULL (obsNode->query);
            memcpy (obsNode->query, query, strlen(query)+1);
        }

        obsNode->token.tokenLength = token->tokenLength;
        memcpy (obsNode->token.token, token->token, token->tokenLength);

        obsNode->addr = (OCDevAddr *)OCMalloc(sizeof(OCDevAddr));
        VERIFY_NON_NULL (obsNode->addr);
        memcpy (obsNode->addr, addr, sizeof(OCDevAddr));

        obsNode->resource = resHandle;

        LL_APPEND (serverObsList, obsNode);
        return OC_STACK_OK;
    }

exit:
    if (obsNode)
    {
        OCFree(obsNode->resUri);
        OCFree(obsNode->query);
        OCFree(obsNode->addr);
        OCFree(obsNode);
    }
    return OC_STACK_NO_MEMORY;
}

ResourceObserver* GetObserverUsingId (const OCObservationId observeId)
{
    ResourceObserver *out = NULL;

    if (observeId)
    {
        LL_FOREACH (serverObsList, out)
        {
            if (out->observeId == observeId)
            {
                return out;
            }
        }
    }
    OC_LOG(INFO, TAG, PCF("Observer node not found!!"));
    return NULL;
}

ResourceObserver* GetObserverUsingToken (const OCCoAPToken * token)
{
    ResourceObserver *out = NULL;

    if(token)
    {
        LL_FOREACH (serverObsList, out)
        {
            OC_LOG(INFO, TAG,PCF("comparing tokens"));
            OC_LOG_BUFFER(INFO, TAG, token->token, token->tokenLength);
            OC_LOG_BUFFER(INFO, TAG, out->token.token, out->token.tokenLength);
            if((out->token.tokenLength == token->tokenLength) &&
               (memcmp(out->token.token, token->token, token->tokenLength) == 0))
            {
                return out;
            }
        }
    }
    OC_LOG(INFO, TAG, PCF("Observer node not found!!"));
    return NULL;
}

OCStackResult DeleteObserverUsingToken (OCCoAPToken * token)
{
    ResourceObserver *obsNode = NULL;

    obsNode = GetObserverUsingToken (token);
    if (obsNode)
    {
        OC_LOG_V(INFO, TAG, PCF("deleting tokens"));
        OC_LOG_BUFFER(INFO, TAG, obsNode->token.token, obsNode->token.tokenLength);
        LL_DELETE (serverObsList, obsNode);
        OCFree(obsNode->resUri);
        OCFree(obsNode->query);
        OCFree(obsNode->addr);
        OCFree(obsNode);
    }
    // it is ok if we did not find the observer...
    return OC_STACK_OK;
}

void DeleteObserverList()
{
    ResourceObserver *out = NULL;
    ResourceObserver *tmp = NULL;
    LL_FOREACH_SAFE (serverObsList, out, tmp)
    {
        DeleteObserverUsingToken (&(out->token));
    }
    serverObsList = NULL;
}
