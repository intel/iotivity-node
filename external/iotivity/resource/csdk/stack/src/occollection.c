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

#define _POSIX_C_SOURCE 200112L
#include <string.h>
#include "ocstack.h"
#include "ocstackinternal.h"
#include "ocresourcehandler.h"
#include "logger.h"
#include "debug.h"
#include "cJSON.h"
/// Module Name
#include <stdio.h>

#define WITH_GROUPACTION 1

#include "oicgroup.h"

#define TAG PCF("occollection")

#define NUM_PARAM_IN_QUERY  2

static OCStackResult CheckRTParamSupport(const OCResource* resource, const char* rtPtr)
{
    OCResourceType* rTPointer = resource->rsrcType;
    while (rTPointer)
    {
        if( strcmp (rTPointer->resourcetypename, rtPtr) == 0)
            return OC_STACK_OK;

        rTPointer = rTPointer->next;
    }
    return OC_STACK_ERROR;
}

static OCStackResult CheckIFParamSupport(const OCResource* resource, const char* ifPtr)
{
    OCResourceInterface* iFPointer = resource->rsrcInterface;
    while (iFPointer)
    {
        if( strcmp (iFPointer->name, ifPtr) == 0)
             return OC_STACK_OK;

        iFPointer = iFPointer->next;
    }
    return OC_STACK_ERROR;
}

static OCStackResult
ValidateQuery (const unsigned char *query, OCResourceHandle resource,
                             OCStackIfTypes *ifParam, char **rtParam)
{
    uint8_t numFields = 0, numParam;

    //TODO: Query and URL validation is being done for virtual resource case
    // using ValidateUrlQuery function. We should be able to merge it with this
    // function.
    OC_LOG(INFO, TAG, PCF("Entering ValidateQuery"));

    if (!query)
        return OC_STACK_ERROR;

    if (!(*query))
    {
        // Query string is empty
        OC_LOG_V(INFO, TAG, PCF("Empty query string, use default IF and RT"));
        *ifParam = STACK_IF_DEFAULT;
        *rtParam = (char *) OCGetResourceTypeName (resource, 0);
        return OC_STACK_OK;
    }

    // Break the query string to validate it and determine IF and RT parameters
    // Validate there are atmost 2 parameters in string and that one is 'if' and other 'rt'
    char *endStr, *ifPtr = NULL, *rtPtr = NULL;
    char *token = strtok_r ((char *)query, "&", &endStr);

    // External loop breaks query string into fields using the & separator
    while (token != NULL)
    {
        numFields++;
        char *endToken;
        char *innerToken = strtok_r (token, "=", &endToken);
        numParam = 0;

        // Internal loop parses the field to extract values (parameters) assigned to each field
        while (innerToken != NULL)
        {
            numParam++;
            if (strcmp (innerToken, OC_RSRVD_INTERFACE) == 0)
            {
                // Determine the value of IF parameter
                innerToken = strtok_r (NULL, "=", &endToken);
                ifPtr = innerToken;
            } else if (strcmp (innerToken, OC_RSRVD_RESOURCE_TYPE) == 0) {
                // Determine the value of RT parameter
                innerToken = strtok_r (NULL, "=", &endToken);
                rtPtr = innerToken;
            } else {
                innerToken = strtok_r (NULL, "=", &endToken);
            }
        }
        if (numParam != 2)
        {
            // Query parameter should be of the form if=<string>. String should not have & or =
            return OC_STACK_INVALID_QUERY;
        }
        token = strtok_r (NULL, "&", &endStr);
    }
    if (numFields > NUM_PARAM_IN_QUERY)
    {
        // M1 release supports one IF value, one RT value and no other params
        return OC_STACK_INVALID_QUERY;
    }

    if (ifPtr)
    {
        if(CheckIFParamSupport((OCResource *)resource, ifPtr) != OC_STACK_OK)
        {
            return OC_STACK_INVALID_QUERY;
        }
        if (strcmp (ifPtr, OC_RSRVD_INTERFACE_DEFAULT) == 0)
        {
            *ifParam = STACK_IF_DEFAULT;
        }
        else if (strcmp (ifPtr, OC_RSRVD_INTERFACE_LL) == 0)
        {
            *ifParam = STACK_IF_LL;
        }
        else if (strcmp (ifPtr, OC_RSRVD_INTERFACE_BATCH) == 0)
        {
            *ifParam = STACK_IF_BATCH;
        }
        else if(strcmp (ifPtr, OC_RSRVD_INTERFACE_GROUP) == 0)
        {
            *ifParam = STACK_IF_GROUP;
        }
        else
        {
            return OC_STACK_ERROR;
        }
    }
    else
    {
        // IF not specified in query, use default IF
        *ifParam = STACK_IF_DEFAULT;
    }

    if (rtPtr)
    {
        if (CheckRTParamSupport((OCResource *)resource, rtPtr) == OC_STACK_OK)
        {
            *rtParam = rtPtr;
        }
        else
        {
            return OC_STACK_INVALID_QUERY;
        }
    }
    else
    {
        // RT not specified in query. Use the first resource type for the resource as default.
        *rtParam = (char *) OCGetResourceTypeName (resource, 0);
    }
    OC_LOG_V(INFO, TAG, "Query params: IF = %d, RT = %s", *ifParam, *rtParam);

    return OC_STACK_OK;
}


static OCStackResult BuildRootResourceJSON(OCResource *resource,
        unsigned char * bufferPtr, uint16_t *remaining)
{
    OCStackResult ret = OC_STACK_ERROR;
    cJSON *resObj;
    char *jsonStr;
    uint16_t jsonLen;

    OC_LOG(INFO, TAG, PCF("Entering BuildRootResourceJSON"));
    resObj = cJSON_CreateObject();

    if (resource)
    {
        cJSON_AddItemToObject (resObj, OC_RSRVD_HREF, cJSON_CreateString(resource->uri));
    }
    jsonStr = cJSON_PrintUnformatted (resObj);
    jsonLen = strlen(jsonStr);
    if (jsonLen < *remaining)
    {
        strcpy((char*) bufferPtr, jsonStr);
        *remaining -= jsonLen;
        bufferPtr += jsonLen;
        ret = OC_STACK_OK;
    }

    cJSON_Delete (resObj);
    free (jsonStr);

    return ret;
}


static OCStackResult
HandleLinkedListInterface(OCEntityHandlerRequest *ehRequest,
                       uint8_t filterOn, char *filterValue)
{
    OCStackResult ret = OC_STACK_ERROR;
    unsigned char jsonbuffer[MAX_RESPONSE_LENGTH] = {0};
    size_t jsonbufferLength = 0;
    uint16_t remaining = 0;
    unsigned char * ptr = NULL;
    OCResource * collResource = (OCResource *) ehRequest->resource;

    ptr = jsonbuffer;
    remaining = MAX_RESPONSE_LENGTH;

    ret = BuildRootResourceJSON(collResource, ptr, &remaining);
    ptr += strlen((char*)ptr);

    if (ret == OC_STACK_OK && remaining >= (sizeof(OC_JSON_SEPARATOR) + 1))
    {
        *ptr = OC_JSON_SEPARATOR;
        ptr++;
        remaining--;
    }
    else
    {
        ret = OC_STACK_ERROR;
    }
    *(ptr + 1) = '\0';

    if (ret == OC_STACK_OK)
    {
        for  (int i = 0; i < MAX_CONTAINED_RESOURCES; i++)
        {
            OCResource* temp = collResource->rsrcResources[i];
            if (temp)
            {
                ret = BuildVirtualResourceResponse(temp, filterOn, filterValue, (char*)ptr, &remaining);
                if (ret != OC_STACK_OK)
                {
                    break;
                }
                ptr += strlen((char*)ptr);
                if (collResource->rsrcResources[i+1] && remaining > sizeof(OC_JSON_SEPARATOR))
                {
                    *ptr = OC_JSON_SEPARATOR;
                    ptr++;
                    remaining--;
                }
                *(ptr + 1) = '\0';
            }
            else
            {
                break;
            }
        }
    }

    jsonbufferLength = strlen((const char *)jsonbuffer);
    if(ret == OC_STACK_OK && jsonbufferLength)
    {
        OCEntityHandlerResponse response = {0};
        response.ehResult = OC_EH_OK;
        response.payload = jsonbuffer;
        response.payloadSize = jsonbufferLength + 1;
        response.persistentBufferFlag = 0;
        response.requestHandle = (OCRequestHandle) ehRequest->requestHandle;
        response.resourceHandle = (OCResourceHandle) collResource;
        ret = OCDoResponse(&response);
    }
    return ret;
}

static OCStackResult
HandleBatchInterface(OCEntityHandlerRequest *ehRequest)
{
    OCStackResult stackRet = OC_STACK_ERROR;
    OCEntityHandlerResult ehResult = OC_EH_ERROR;
    unsigned char jsonbuffer[MAX_RESPONSE_LENGTH] = {0};
    size_t jsonbufferLength = 0;
    uint16_t remaining = 0;
    unsigned char * ptr = NULL;
    OCResource * collResource = (OCResource *) ehRequest->resource;

    ptr = jsonbuffer;
    remaining = MAX_RESPONSE_LENGTH;

    stackRet = BuildRootResourceJSON(collResource, ptr, &remaining);
    ptr += strlen((char*)ptr);
    *(ptr + 1) = '\0';

    jsonbufferLength = strlen((const char *)jsonbuffer);
    if(jsonbufferLength)
    {
        OCEntityHandlerResponse response = {0};
        response.ehResult = OC_EH_OK;
        response.payload = jsonbuffer;
        response.payloadSize = jsonbufferLength + 1;
        response.persistentBufferFlag = 0;
        response.requestHandle = (OCRequestHandle) ehRequest->requestHandle;
        response.resourceHandle = (OCResourceHandle) collResource;
        stackRet = OCDoResponse(&response);
    }

    if (stackRet == OC_STACK_OK)
    {
        for  (int i = 0; i < MAX_CONTAINED_RESOURCES; i++)
        {
            OCResource* temp = collResource->rsrcResources[i];
            if (temp)
            {
                // Note that all entity handlers called through a collection
                // will get the same pointer to ehRequest, the only difference
                // is ehRequest->resource
                ehRequest->resource = (OCResourceHandle) temp;

                ehResult = temp->entityHandler(OC_REQUEST_FLAG, ehRequest);

                // The default collection handler is returning as OK
                if(stackRet != OC_STACK_SLOW_RESOURCE)
                {
                    stackRet = OC_STACK_OK;
                }
                // if a single resource is slow, then entire response will be treated
                // as slow response
                if(ehResult == OC_EH_SLOW)
                {
                    OC_LOG(INFO, TAG, PCF("This is a slow resource"));
                    ((OCServerRequest *)ehRequest->requestHandle)->slowFlag = 1;
                    stackRet = EntityHandlerCodeToOCStackCode(ehResult);
                }
            }
            else
            {
                break;
            }
        }
        ehRequest->resource = (OCResourceHandle) collResource;
    }
    return stackRet;
}

uint8_t GetNumOfResourcesInCollection (OCResource *resource)
{
    uint8_t num = 0;
    for (int i = 0; i < MAX_CONTAINED_RESOURCES; i++)
    {
        if (resource->rsrcResources[i])
        {
            num++;
        }
    }
    return num;
}


OCStackResult DefaultCollectionEntityHandler (OCEntityHandlerFlag flag,
                                              OCEntityHandlerRequest *ehRequest)
{
    OCStackResult result = OC_STACK_ERROR;
    OCStackIfTypes ifQueryParam = STACK_IF_INVALID;
    char *rtQueryParam = NULL;

    OC_LOG_V(INFO, TAG, "DefaultCollectionEntityHandler with query %s", ehRequest->query);

    if (flag != OC_REQUEST_FLAG)
        return OC_STACK_ERROR;

    result = ValidateQuery ((const unsigned char *)ehRequest->query,
                            ehRequest->resource, &ifQueryParam, &rtQueryParam);

    if (result != OC_STACK_OK)
        return result;

  
    if(!((ehRequest->method == OC_REST_GET) || 
        (ehRequest->method == OC_REST_PUT) ||
        (ehRequest->method == OC_REST_POST)))
        return OC_STACK_ERROR;

    if (ehRequest->method == OC_REST_GET)
    {
        switch (ifQueryParam)
        {
            case STACK_IF_DEFAULT:
                // Get attributes of collection resource and properties of contined resource
                // M1 release does not support attributes for collection resource, so the GET
                // operation is same as the GET on LL interface.
                OC_LOG(INFO, TAG, PCF("STACK_IF_DEFAULT"));
                return HandleLinkedListInterface(ehRequest, STACK_RES_DISCOVERY_NOFILTER, NULL);

            case STACK_IF_LL:
                OC_LOG(INFO, TAG, PCF("STACK_IF_LL"));
                return HandleLinkedListInterface(ehRequest, STACK_RES_DISCOVERY_NOFILTER, NULL);

            case STACK_IF_BATCH:
                OC_LOG(INFO, TAG, PCF("STACK_IF_BATCH"));
                ((OCServerRequest *)ehRequest->requestHandle)->ehResponseHandler = HandleAggregateResponse;
                ((OCServerRequest *)ehRequest->requestHandle)->numResponses =
                        GetNumOfResourcesInCollection((OCResource *)ehRequest->resource) + 1;
                return HandleBatchInterface(ehRequest);
            case STACK_IF_GROUP:
                return BuildCollectionGroupActionJSONResponse(OC_REST_GET/*flag*/,
                        (OCResource *) ehRequest->resource, ehRequest);
            default:
                return OC_STACK_ERROR;
        }
    } else if (ehRequest->method == OC_REST_PUT) {
        switch (ifQueryParam)
        {
            case STACK_IF_DEFAULT:
                // M1 release does not support PUT on default interface
                return OC_STACK_ERROR;

            case STACK_IF_LL:
                // LL interface only supports GET
                return OC_STACK_ERROR;

            case STACK_IF_BATCH:
                ((OCServerRequest *)ehRequest->requestHandle)->ehResponseHandler = HandleAggregateResponse;
                ((OCServerRequest *)ehRequest->requestHandle)->numResponses =
                        GetNumOfResourcesInCollection((OCResource *)ehRequest->resource) + 1;
                return HandleBatchInterface(ehRequest);

            case STACK_IF_GROUP:
            {
                OC_LOG_V(INFO, TAG, "IF_COLLECTION PUT with request ::\n%s\n ",
                        ehRequest->reqJSONPayload);
                return BuildCollectionGroupActionJSONResponse(OC_REST_PUT/*flag*/,
                        (OCResource *) ehRequest->resource, ehRequest);
            }
            default:
                return OC_STACK_ERROR;
        }
    }
    else if (ehRequest->method == OC_REST_POST)
    {

        switch (ifQueryParam)
        {
            case STACK_IF_GROUP:
            {
                OC_LOG_V(INFO, TAG, "IF_COLLECTION POST with request :: \n%s\n ",
                        ehRequest->reqJSONPayload);
                return BuildCollectionGroupActionJSONResponse(OC_REST_POST/*flag*/,
                        (OCResource *) ehRequest->resource, ehRequest);
            }
            default:
                return OC_STACK_ERROR;
        }
    }
    return result;
}

