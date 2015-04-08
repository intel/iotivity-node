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

#include "ocstack.h"
#include "ocserverrequest.h"
#include "ocresourcehandler.h"

// Module Name
#define VERIFY_NON_NULL(arg) { if (!arg) {OC_LOG(FATAL, TAG, #arg " is NULL"); goto exit;} }

#define TAG  PCF("ocserverrequest")

static struct OCServerRequest * serverRequestList = NULL;
static struct OCServerResponse * serverResponseList = NULL;

OCServerRequest * GetServerRequestUsingToken (const OCCoAPToken token)
{
    OCServerRequest * out = NULL;
    LL_FOREACH (serverRequestList, out)
    {
        OC_LOG(INFO, TAG,PCF("comparing tokens"));
        OC_LOG_BUFFER(INFO, TAG, token.token, token.tokenLength);
        OC_LOG_BUFFER(INFO, TAG, out->requestToken.token, out->requestToken.tokenLength);
        if((out->requestToken.tokenLength == token.tokenLength) &&
                (memcmp(out->requestToken.token, token.token, token.tokenLength) == 0))
        {
            return out;
        }
    }
    OC_LOG(INFO, TAG, PCF("Server Request not found!!"));
    return NULL;
}

OCServerRequest * GetServerRequestUsingHandle (const OCServerRequest * handle)
{
    OCServerRequest * out = NULL;
    LL_FOREACH (serverRequestList, out)
    {
        if(out == handle)
        {
            return out;
        }
    }
    OC_LOG(INFO, TAG, PCF("Server Request not found!!"));
    return NULL;
}

OCServerResponse * GetServerResponseUsingHandle (const OCServerRequest * handle)
{
    OCServerResponse * out = NULL;
    LL_FOREACH (serverResponseList, out)
    {
        if(out->requestHandle == handle)
        {
            return out;
        }
    }
    OC_LOG(INFO, TAG, PCF("Server Response not found!!"));
    return NULL;
}

OCStackResult AddServerRequest (OCServerRequest ** request, uint16_t coapID,
        uint8_t delayedResNeeded, uint8_t secured, uint8_t notificationFlag, OCMethod method,
        uint8_t numRcvdVendorSpecificHeaderOptions, uint32_t observationOption,
        OCQualityOfService qos, unsigned char * query,
        OCHeaderOption * rcvdVendorSpecificHeaderOptions,
        unsigned char * reqJSONPayload, OCCoAPToken * requestToken,
        OCDevAddr * requesterAddr, unsigned char * resourceUrl, uint32_t reqTotalSize)
{
    OCServerRequest * serverRequest = NULL;

    serverRequest = (OCServerRequest *) OCCalloc(1, sizeof(OCServerRequest) + reqTotalSize - 1);
    VERIFY_NON_NULL(serverRequest);

    serverRequest->coapID = coapID;
    serverRequest->delayedResNeeded = delayedResNeeded;
    serverRequest->secured = secured;
    serverRequest->notificationFlag = notificationFlag;

    serverRequest->method = method;
    serverRequest->numRcvdVendorSpecificHeaderOptions = numRcvdVendorSpecificHeaderOptions;
    serverRequest->observationOption = observationOption;
    serverRequest->observeResult = OC_STACK_ERROR;
    serverRequest->qos = qos;
    serverRequest->ehResponseHandler = HandleSingleResponse;
    serverRequest->numResponses = 1;
    if(query)
    {
        memcpy(serverRequest->query, query, strlen((const char *)query) + 1);
    }
    if(rcvdVendorSpecificHeaderOptions)
    {
        memcpy(serverRequest->rcvdVendorSpecificHeaderOptions, rcvdVendorSpecificHeaderOptions,
            MAX_HEADER_OPTIONS * sizeof(OCHeaderOption));
    }
    if(reqJSONPayload)
    {
        memcpy((void *)serverRequest->reqJSONPayload, (void *)reqJSONPayload,
            strlen((const char *)reqJSONPayload) + 1);
    }
    serverRequest->requestComplete = 0;
    if(requestToken)
    {
        memcpy(&serverRequest->requestToken, requestToken, sizeof(OCCoAPToken));
    }
    if(requesterAddr)
    {
        memcpy(&serverRequest->requesterAddr, requesterAddr, sizeof(OCDevAddr));
    }
    if(resourceUrl)
    {
        memcpy(serverRequest->resourceUrl, resourceUrl, strlen((const char *)resourceUrl) + 1);
    }
    *request = serverRequest;
    OC_LOG(INFO, TAG, PCF("Server Request Added!!"));
    LL_APPEND (serverRequestList, serverRequest);
    return OC_STACK_OK;

exit:
    if (serverRequest)
    {
        OCFree(serverRequest);
        serverRequest = NULL;
    }
    *request = NULL;
    return OC_STACK_NO_MEMORY;
}

OCStackResult AddServerResponse (OCServerResponse ** response, OCRequestHandle requestHandle)
{
    OCServerResponse * serverResponse = NULL;

    serverResponse = (OCServerResponse *) OCCalloc(1, sizeof(OCServerResponse));
    VERIFY_NON_NULL(serverResponse);

    serverResponse->payload = (unsigned char *) OCMalloc(MAX_RESPONSE_LENGTH);
    VERIFY_NON_NULL(serverResponse->payload);
    memset(serverResponse->payload, 0, sizeof(MAX_RESPONSE_LENGTH));

    serverResponse->remainingPayloadSize = MAX_RESPONSE_LENGTH;
    serverResponse->requestHandle = requestHandle;

    *response = serverResponse;
    OC_LOG(INFO, TAG, PCF("Server Response Added!!"));
    LL_APPEND (serverResponseList, serverResponse);
    return OC_STACK_OK;

exit:
    if (serverResponse)
    {
        OCFree(serverResponse);
        serverResponse = NULL;
    }
    *response = NULL;
    return OC_STACK_NO_MEMORY;
}

// Form the OCEntityHandlerRequest struct
OCStackResult FormOCEntityHandlerRequest(OCEntityHandlerRequest * entityHandlerRequest, OCRequestHandle request,
        OCMethod method, OCResourceHandle resource, unsigned char * queryBuf, unsigned char * bufReqPayload,
        uint8_t numVendorOptions, OCHeaderOption * vendorOptions, OCObserveAction observeAction,
        OCObservationId observeID)
{
    if (entityHandlerRequest)
    {
        memset(entityHandlerRequest, 0, sizeof(OCEntityHandlerRequest));
        entityHandlerRequest->requestHandle = request;
        entityHandlerRequest->method = method;
        entityHandlerRequest->resource = (OCResourceHandle) resource;
        entityHandlerRequest->query = queryBuf;
        entityHandlerRequest->reqJSONPayload = bufReqPayload;
        entityHandlerRequest->numRcvdVendorSpecificHeaderOptions = numVendorOptions;
        entityHandlerRequest->rcvdVendorSpecificHeaderOptions = vendorOptions;

        entityHandlerRequest->obsInfo.action = observeAction;
        entityHandlerRequest->obsInfo.obsId = observeID;
        return OC_STACK_OK;
    }

    return OC_STACK_INVALID_PARAM;
}

void FindAndDeleteServerResponse(OCServerResponse * serverResponse)
{
    OCServerResponse* tmp;
    if(serverResponse)
    {
        LL_FOREACH(serverResponseList, tmp)
        {
            if (serverResponse == tmp)
            {
                DeleteServerResponse(tmp);
                return;
            }
        }
    }
}

void DeleteServerResponse(OCServerResponse * serverResponse)
{
    if(serverResponse) {
        LL_DELETE(serverResponseList, serverResponse);
        OCFree(serverResponse->payload);
        OCFree(serverResponse);
        OC_LOG(INFO, TAG, PCF("Server Response Removed!!"));
    }
}

void FindAndDeleteServerRequest(OCServerRequest * serverRequest)
{
    OCServerRequest* tmp;
    if(serverRequest)
    {
        LL_FOREACH(serverRequestList, tmp)
        {
            if (serverRequest == tmp)
            {
                DeleteServerRequest(tmp);
                return;
            }
        }
    }
}

void DeleteServerRequest(OCServerRequest * serverRequest)
{
    if(serverRequest) {
        LL_DELETE(serverRequestList, serverRequest);
        OCFree(serverRequest);
        serverRequest = NULL;
        OC_LOG(INFO, TAG, PCF("Server Request Removed!!"));
    }
}

OCStackResult HandleSingleResponse(OCEntityHandlerResponse * ehResponse)
{
    OCStackResult result = OC_STACK_ERROR;
    OCServerProtocolResponse protocolResponse = {0};

    OC_LOG_V(INFO, TAG, "Inside HandleSingleResponse: %s", ehResponse->payload);

    OCServerRequest *serverRequest = (OCServerRequest *)ehResponse->requestHandle;
    // Format protocol response structure with data needed for
    // sending the response
    protocolResponse.qos = serverRequest->qos;

    if((OCResource *)ehResponse->resourceHandle &&
            ((OCResource *)ehResponse->resourceHandle)->resourceProperties == (OCResourceProperty) 0)
    {
        ehResponse->ehResult = OC_EH_RESOURCE_DELETED;
    }
    protocolResponse.result = EntityHandlerCodeToOCStackCode(ehResponse->ehResult);
    protocolResponse.requesterAddr = &serverRequest->requesterAddr;
    protocolResponse.requestToken = &serverRequest->requestToken;
    protocolResponse.numSendVendorSpecificHeaderOptions = ehResponse->numSendVendorSpecificHeaderOptions;
    protocolResponse.sendVendorSpecificHeaderOptions = ehResponse->sendVendorSpecificHeaderOptions;
    protocolResponse.resourceUri = ehResponse->resourceUri;
    protocolResponse.delayedResNeeded = serverRequest->delayedResNeeded;
    protocolResponse.secured = serverRequest->secured;
    protocolResponse.slowFlag = serverRequest->slowFlag;
    protocolResponse.notificationFlag = serverRequest->notificationFlag;

    //should we put the prefix and suffix here?
    protocolResponse.payload = (unsigned char *) OCMalloc(MAX_RESPONSE_LENGTH);
    if(!protocolResponse.payload)
    {
        return OC_STACK_NO_MEMORY;
    }
    strcpy((char *)protocolResponse.payload, (const char *)OC_JSON_PREFIX);
    strcat((char *)protocolResponse.payload, (const char *)ehResponse->payload);
    strcat((char *)protocolResponse.payload, (const char *)OC_JSON_SUFFIX);
    protocolResponse.payloadSize = strlen((const char *)protocolResponse.payload) + 1;
    protocolResponse.resourceUri = ehResponse->resourceUri;

    //revise the following
    protocolResponse.coapID = serverRequest->coapID;
    if(serverRequest->observeResult == OC_STACK_OK)
    {
        protocolResponse.observationOption = serverRequest->observationOption;
    }
    else
    {
        protocolResponse.observationOption = OC_OBSERVE_NO_OPTION;
    }
    // Make call to OCCoAP layer
    result = OCDoCoAPResponse(&protocolResponse);

    OCFree(protocolResponse.payload);
    //Delete the request
    FindAndDeleteServerRequest(serverRequest);
    return result;
}

OCStackResult HandleAggregateResponse(OCEntityHandlerResponse * ehResponse)
{
    OCStackResult stackRet = OC_STACK_ERROR;
    OCServerRequest * serverRequest = NULL;
    OCServerResponse * serverResponse = NULL;

    OC_LOG_V(INFO, TAG, "Inside HandleAggregateResponse: %s", ehResponse->payload);

    serverRequest = GetServerRequestUsingHandle((OCServerRequest *)ehResponse->requestHandle);
    serverResponse = GetServerResponseUsingHandle((OCServerRequest *)ehResponse->requestHandle);

    if(serverRequest)
    {
        if(!serverResponse)
        {
            OC_LOG(INFO, TAG, PCF("This is the first response fragment"));
            stackRet = AddServerResponse(&serverResponse, ehResponse->requestHandle);
            if (OC_STACK_OK != stackRet)
            {
                OC_LOG(ERROR, TAG, PCF("Error adding server response"));
                return stackRet;
            }
            VERIFY_NON_NULL(serverResponse);
            VERIFY_NON_NULL(serverResponse->payload);
        }

        if((serverResponse->remainingPayloadSize >= ehResponse->payloadSize + 1 &&
                serverRequest->numResponses == 1) ||
                (serverResponse->remainingPayloadSize >= ehResponse->payloadSize + 2 &&
                        serverRequest->numResponses > 1))
        {
            OC_LOG(INFO, TAG, PCF("There is room in response buffer"));
            // append
            snprintf((char *)serverResponse->payload, serverResponse->remainingPayloadSize, "%s%s", (char *)serverResponse->payload, (char *)ehResponse->payload);
            OC_LOG_V(INFO, TAG, "Current aggregated response  ...%s", serverResponse->payload);
            serverResponse->remainingPayloadSize -= ehResponse->payloadSize;
            (serverRequest->numResponses)--;
            if(serverRequest->numResponses == 0)
            {
                OC_LOG(INFO, TAG, PCF("This is the last response fragment"));
                ehResponse->payload = serverResponse->payload;
                ehResponse->payloadSize = strlen((char *) serverResponse->payload) + 1;
                stackRet = HandleSingleResponse(ehResponse);
                //Delete the request and response
                FindAndDeleteServerRequest(serverRequest);
                FindAndDeleteServerResponse(serverResponse);
            }
            else
            {
                OC_LOG(INFO, TAG, PCF("More response fragment to come"));
                // TODO: we should consider using strcat rather than setting a char by char here!
                snprintf((char *)serverResponse->payload, serverResponse->remainingPayloadSize, "%s%c", (char *)serverResponse->payload,OC_JSON_SEPARATOR);
                OC_LOG_V(INFO, TAG, "Current aggregated response  ...%s", serverResponse->payload);
                (serverResponse->remainingPayloadSize)--;
                stackRet = OC_STACK_OK;
            }
        }
        else
        {
            OC_LOG(INFO, TAG, PCF("No room in response buffer"));
            //Delete the request and response
            FindAndDeleteServerRequest(serverRequest);
            FindAndDeleteServerResponse(serverResponse);
            stackRet = OC_STACK_NO_MEMORY;
        }
    }
exit:
    return stackRet;
}
