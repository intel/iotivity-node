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
#include "ocstackconfig.h"
#include "ocstackinternal.h"
#include "ocresourcehandler.h"
#include "ocobserve.h"
#include "occollection.h"
#include "occoap.h"
#include "logger.h"
#include "debug.h"
#include "cJSON.h"

/// Module Name
#define TAG PCF("ocresource")
#define VERIFY_SUCCESS(op, successCode) { if (op != successCode) \
            {OC_LOG_V(FATAL, TAG, "%s failed!!", #op); goto exit;} }

#define VERIFY_NON_NULL(arg, logLevel, retVal) { if (!(arg)) { OC_LOG((logLevel), \
             TAG, PCF(#arg " is NULL")); return (retVal); } }

extern OCResource *headResource;
static cJSON *savedDeviceInfo = NULL;

static const char * VIRTUAL_RSRCS[] = {
       "/oc/core",
       "/oc/core/d",
       "/oc/core/types/d",
       #ifdef WITH_PRESENCE
       "/oc/presence"
       #endif
};

//-----------------------------------------------------------------------------
// Default resource entity handler function
//-----------------------------------------------------------------------------
OCEntityHandlerResult defaultResourceEHandler(OCEntityHandlerFlag flag,
        OCEntityHandlerRequest * request) {
    TODO ("Implement me!!!!");
    // TODO:  remove silence unused param warnings
    (void) flag;
    (void) request;
    return  OC_EH_OK; // Making sure that the Default EH and the Vendor EH have matching signatures
}

static OCStackResult ValidateUrlQuery (unsigned char *url, unsigned char *query,
                                uint8_t *filterOn, char **filterValue)
{
    char *filterParam;

    OC_LOG(INFO, TAG, PCF("Entering ValidateUrlQuery"));
    if (!url)
        return OC_STACK_INVALID_URI;

    if (strcmp ((char *)url, GetVirtualResourceUri(OC_WELL_KNOWN_URI)) == 0 ||
                strcmp ((char *)url, GetVirtualResourceUri(OC_DEVICE_URI)) == 0) {
        *filterOn = STACK_RES_DISCOVERY_NOFILTER;
        if (query && *query) {
            char* strTokPtr;
            filterParam = strtok_r((char *)query, "=", &strTokPtr);
            *filterValue = strtok_r(NULL, " ", &strTokPtr);
            if (!(*filterValue)) {
                return OC_STACK_INVALID_QUERY;
            } else if (strcmp (filterParam, OC_RSRVD_INTERFACE) == 0) {
                // Resource discovery with interface filter
                *filterOn = STACK_RES_DISCOVERY_IF_FILTER;
            } else if (strcmp (filterParam, OC_RSRVD_RESOURCE_TYPE) == 0) {
                // Resource discovery with resource type filter
                *filterOn = STACK_RES_DISCOVERY_RT_FILTER;
            } else if (strcmp (filterParam, OC_RSRVD_DEVICE_ID) == 0) {
                //Device ID filter
                *filterOn = STACK_DEVICE_DISCOVERY_DI_FILTER;
            } else if (strcmp (filterParam, OC_RSRVD_DEVICE_NAME) == 0) {
                //Device Name filter
                *filterOn = STACK_DEVICE_DISCOVERY_DN_FILTER;
            } else {
                // Other filter types not supported
                return OC_STACK_INVALID_QUERY;
            }
        }
    }
    #ifdef WITH_PRESENCE
    else if (strcmp((char *)url, GetVirtualResourceUri(OC_PRESENCE)) == 0) {
        //Nothing needs to be done, except for pass a OC_PRESENCE query through as OC_STACK_OK.
        OC_LOG(INFO, TAG, PCF("OC_PRESENCE Request"));
        *filterOn = STACK_RES_DISCOVERY_NOFILTER;
    }
    #endif
    else {
        // Other URIs not yet supported
        return OC_STACK_INVALID_URI;
    }
    OC_LOG(INFO, TAG, PCF("Exiting ValidateUrlQuery"));
    return OC_STACK_OK;
}

OCStackResult BuildVirtualResourceResponse(OCResource *resourcePtr, uint8_t filterOn,
                                            char *filterValue, char * out, uint16_t *remaining)
{
    OCResourceType *resourceTypePtr;
    OCResourceInterface *interfacePtr;
    cJSON *resObj, *propObj, *rtArray;
    char *jsonStr;
    uint8_t encodeRes = 0;
    OCStackResult ret = OC_STACK_OK;
    uint16_t jsonLen;

    OC_LOG(INFO, TAG, PCF("Entering BuildVirtualResourceResponse"));
    resObj = cJSON_CreateObject();

    if (resourcePtr)
    {
        encodeRes = 0;
        if (filterOn == STACK_RES_DISCOVERY_RT_FILTER) {
            resourceTypePtr = resourcePtr->rsrcType;
            while (resourceTypePtr) {
                if (strcmp (resourceTypePtr->resourcetypename, filterValue) == 0) {
                    encodeRes = 1;
                    break;
                }
                resourceTypePtr = resourceTypePtr->next;
            }
        } else if (filterOn == STACK_RES_DISCOVERY_IF_FILTER) {
            interfacePtr = resourcePtr->rsrcInterface;
            while (interfacePtr) {
                if (strcmp (interfacePtr->name, filterValue) == 0) {
                    encodeRes = 1;
                    break;
                }
                interfacePtr = interfacePtr->next;
            }
        } else if (filterOn == STACK_RES_DISCOVERY_NOFILTER) {
            encodeRes = 1;
        } else {
            //TODO: Unsupported query filter
            return OC_STACK_INVALID_QUERY;
        }

        if (encodeRes) {
            // Add URIs
            cJSON_AddItemToObject (resObj, OC_RSRVD_HREF, cJSON_CreateString(resourcePtr->uri));

            cJSON_AddItemToObject (resObj, "prop", propObj = cJSON_CreateObject());
            // Add resource types
            cJSON_AddItemToObject (propObj, OC_RSRVD_RESOURCE_TYPE, rtArray = cJSON_CreateArray());
            resourceTypePtr = resourcePtr->rsrcType;
            while (resourceTypePtr) {
                cJSON_AddItemToArray (rtArray,
                                      cJSON_CreateString(resourceTypePtr->resourcetypename));
                resourceTypePtr = resourceTypePtr->next;
            }
            // Add interface types
            cJSON_AddItemToObject (propObj, OC_RSRVD_INTERFACE, rtArray = cJSON_CreateArray());
            interfacePtr = resourcePtr->rsrcInterface;
            while (interfacePtr) {
                cJSON_AddItemToArray (rtArray, cJSON_CreateString(interfacePtr->name));
                interfacePtr = interfacePtr->next;
            }
            // If resource is observable, set observability flag.
            // Resources that are not observable will not have the flag.
            if (resourcePtr->resourceProperties & OC_OBSERVABLE) {
                cJSON_AddItemToObject (propObj, OC_RSRVD_OBSERVABLE,
                                       cJSON_CreateNumber(OC_RESOURCE_OBSERVABLE));
            }
            // Set secure flag for secure resources
            if (resourcePtr->resourceProperties & OC_SECURE) {
                uint16_t port;
                cJSON_AddNumberToObject (propObj, OC_RSRVD_SECURE, OC_RESOURCE_SECURE);
                //Set the IP port also as secure resources are hosted on a different port
                if (OCGetResourceEndPointInfo (resourcePtr, &port) == OC_STACK_OK) {
                    cJSON_AddNumberToObject (propObj, OC_RSRVD_HOSTING_PORT, port);
                }
            }

        }
    }
    jsonStr = cJSON_PrintUnformatted (resObj);

    jsonLen = strlen(jsonStr);
    if (jsonLen < *remaining)
    {
        strcpy(out, jsonStr);
        *remaining = *remaining - jsonLen;
    }
    else
    {
        ret = OC_STACK_ERROR;
    }
    cJSON_Delete (resObj);
    free (jsonStr);

    OC_LOG(INFO, TAG, PCF("Exiting BuildVirtualResourceResponse"));
    return ret;
}

OCStackResult BuildVirtualResourceResponseForDevice(uint8_t filterOn, char *filterValue,
                                                    char *out, uint16_t *remaining)
{
    OCStackResult ret = OC_STACK_ERROR;

    if (savedDeviceInfo != NULL)
    {
        char *jsonStr = NULL;
        uint16_t jsonLen = 0;
        cJSON *repObj = cJSON_GetObjectItem(savedDeviceInfo, "rep");

        OC_LOG(INFO, TAG, PCF("Entering BuildVirtualResourceResponseForDevice"));

        if (filterOn == STACK_DEVICE_DISCOVERY_DI_FILTER)
        {
            if((cJSON_GetObjectItem(repObj,"di") != NULL) &&
                    strcmp(cJSON_GetObjectItem(repObj,"di")->valuestring, filterValue) == 0)
            {
                ret = OC_STACK_OK;
            }
        }
        else if (filterOn == STACK_DEVICE_DISCOVERY_DN_FILTER)
        {
            if((cJSON_GetObjectItem(repObj,"dn") != NULL) &&
                    strcmp(cJSON_GetObjectItem(repObj,"dn")->valuestring, filterValue) == 0)
            {
                ret = OC_STACK_OK;
            }
        }
        else if (filterOn == STACK_RES_DISCOVERY_NOFILTER)
        {
            ret = OC_STACK_OK;
        }
        else
        {
            ret = OC_STACK_INVALID_QUERY;
        }

        if (ret == OC_STACK_OK)
        {
            jsonStr = cJSON_PrintUnformatted (savedDeviceInfo);

            if(jsonStr)
            {
                jsonLen = strlen(jsonStr);

                if (jsonLen < *remaining)
                {
                    strncpy(out, jsonStr, (jsonLen + 1));
                    *remaining = *remaining - jsonLen;
                    ret = OC_STACK_OK;
                }
                else
                {
                    ret = OC_STACK_ERROR;
                }

                free(jsonStr);
            }
            else
            {
                ret = OC_STACK_ERROR;
            }
        }
        else
        {
            ret = OC_STACK_INVALID_DEVICE_INFO;
        }
    }
    else
    {
        //error so that stack won't respond with empty payload
        ret = OC_STACK_INVALID_DEVICE_INFO;
    }

    OC_LOG(INFO, TAG, PCF("Exiting BuildVirtualResourceResponseForDevice"));
    return ret;
}

TODO ("Does it make sense to make this method as inline")
const char * GetVirtualResourceUri( OCVirtualResources resource)
{
    if (resource < OC_MAX_VIRTUAL_RESOURCES)
    {
        return VIRTUAL_RSRCS[resource];
    }

    return NULL;
}

uint8_t IsVirtualResource(const char* resourceUri)
{
    for (int i = 0; i < OC_MAX_VIRTUAL_RESOURCES; i++)
    {
        if (strcmp(resourceUri, GetVirtualResourceUri((OCVirtualResources)i)) == 0)
        {
            return 1;
        }
    }
    return 0;
}

uint8_t IsCollectionResource (OCResource *resource)
{
    for (int i = 0; i < MAX_CONTAINED_RESOURCES; i++)
    {
        if (resource->rsrcResources[i])
        {
            return 1;
        }
    }
    return 0;
}

OCResource *FindResourceByUri(const char* resourceUri)
{
    OCResource * pointer = headResource;
    while (pointer) {
        if (strcmp(resourceUri, pointer->uri) == 0) {
            return pointer;
        }
        pointer = pointer->next;
    }
    OC_LOG(INFO, TAG, PCF("Resource not found"));
    return NULL;
}


OCStackResult DetermineResourceHandling (OCServerRequest *request,
                                         ResourceHandling *handling,
                                         OCResource **resource)
{

    OC_LOG(INFO, TAG, PCF("Entering DetermineResourceHandling"));

    // Check if virtual resource
    if (IsVirtualResource((const char*)request->resourceUrl))
    {
        *handling = OC_RESOURCE_VIRTUAL;
        *resource = headResource;
        return OC_STACK_OK;
    }
    if (NULL == request->resourceUrl || (strlen((const char*)(request->resourceUrl)) == 0))
    {
        // Resource URL not specified
        *handling = OC_RESOURCE_NOT_SPECIFIED;
        return OC_STACK_OK;
    }
    else
    {
        OCResource *resourcePtr = NULL;
        resourcePtr = FindResourceByUri((const char*)request->resourceUrl);
        *resource = resourcePtr;
        if (!resourcePtr)
        {
            if(defaultDeviceHandler)
            {
                *handling = OC_RESOURCE_DEFAULT_DEVICE_ENTITYHANDLER;
                return OC_STACK_OK;
            }

            // Resource does not exist
            // and default device handler does not exist
            *handling = OC_RESOURCE_NOT_SPECIFIED;
            return OC_STACK_NO_RESOURCE;
        }

        // secure resource will entertain only authorized requests
        if ((resourcePtr->resourceProperties & OC_SECURE) && (request->secured == 0))
        {
            OC_LOG(INFO, TAG, PCF("Un-authorized request. Ignore it!"));
            return OC_STACK_RESOURCE_ERROR;
        }

        if (IsCollectionResource (resourcePtr))
        {
            // Collection resource
            if (resourcePtr->entityHandler != defaultResourceEHandler)
            {
                *handling = OC_RESOURCE_COLLECTION_WITH_ENTITYHANDLER;
                return OC_STACK_OK;
            } else {
                *handling = OC_RESOURCE_COLLECTION_DEFAULT_ENTITYHANDLER;
                return OC_STACK_OK;
            }
        } else {
            // Resource not a collection
            if (resourcePtr->entityHandler != defaultResourceEHandler)
            {
                *handling = OC_RESOURCE_NOT_COLLECTION_WITH_ENTITYHANDLER;
                return OC_STACK_OK;
            } else {
                *handling = OC_RESOURCE_NOT_COLLECTION_DEFAULT_ENTITYHANDLER;
                return OC_STACK_OK;
            }
        }
    }
}

OCStackResult EntityHandlerCodeToOCStackCode(OCEntityHandlerResult ehResult)
{
    OCStackResult result;

    switch (ehResult)
    {
        case OC_EH_OK:
            result = OC_STACK_OK;
            break;
        case OC_EH_SLOW:
            result = OC_STACK_SLOW_RESOURCE;
            break;
        case OC_EH_ERROR:
            result = OC_STACK_ERROR;
            break;
        case OC_EH_FORBIDDEN:
            result = OC_STACK_RESOURCE_ERROR;
            break;
        case OC_EH_RESOURCE_CREATED:
            result = OC_STACK_RESOURCE_CREATED;
            break;
        case OC_EH_RESOURCE_DELETED:
            result = OC_STACK_RESOURCE_DELETED;
            break;
        default:
            result = OC_STACK_ERROR;
    }

    return result;
}

static OCStackResult
HandleVirtualResource (OCServerRequest *request, OCResource* resource)
{
    OCStackResult result = OC_STACK_ERROR;
    char *filterValue = NULL;
    uint8_t filterOn = 0;
    uint16_t remaining = 0;
    unsigned char * ptr = NULL;
    uint8_t firstLoopDone = 0;
    unsigned char discoveryResBuf[MAX_RESPONSE_LENGTH] = {0};

    OC_LOG(INFO, TAG, PCF("Entering HandleVirtualResource"));

    result = ValidateUrlQuery (request->resourceUrl,
            request->query, &filterOn,
            &filterValue);

    if (result == OC_STACK_OK)
    {
        if (strcmp ((char *)request->resourceUrl, GetVirtualResourceUri(OC_WELL_KNOWN_URI)) == 0)
        {
            ptr = discoveryResBuf;
            remaining = MAX_RESPONSE_LENGTH;

            while(resource)
            {
                if((resource->resourceProperties & OC_ACTIVE)
                        && (resource->resourceProperties & OC_DISCOVERABLE))
                {
                    // if there is data on the buffer, we have already added a response,
                    // so we need to add a comma before we do anything
                    if(firstLoopDone
                            && remaining >= (sizeof(OC_JSON_SEPARATOR)+1))
                    {
                        *ptr = OC_JSON_SEPARATOR;
                        ptr++;
                        remaining--;
                    }
                    firstLoopDone = 1;
                    result = BuildVirtualResourceResponse(resource, filterOn, filterValue,
                            (char*)ptr, &remaining);

                    if (result != OC_STACK_OK)
                    {
                        // if this failed, we need to remove the comma added above.
                        if(!firstLoopDone)
                        {
                            ptr--;
                            *ptr = '\0';
                            remaining++;
                        }
                        break;
                    }
                    ptr += strlen((char *)ptr);
                    *(ptr + 1) = '\0';
                }
                resource = resource->next;
            }

            if(strlen((const char *)discoveryResBuf) > 0)
            {
                OCEntityHandlerResponse response = {0};

                response.ehResult = OC_EH_OK;
                response.payload = discoveryResBuf;
                response.payloadSize = strlen((const char *)discoveryResBuf) + 1;
                response.persistentBufferFlag = 0;
                response.requestHandle = (OCRequestHandle) request;
                response.resourceHandle = (OCResourceHandle) resource;

                result = OCDoResponse(&response);
            }
        }
        else if (strcmp ((char *)request->resourceUrl, GetVirtualResourceUri(OC_DEVICE_URI)) == 0)
        {
            remaining = MAX_RESPONSE_LENGTH;
            ptr = discoveryResBuf;

            result = BuildVirtualResourceResponseForDevice(filterOn, filterValue,
                    (char*)ptr, &remaining);

            if(result == OC_STACK_OK)
            {
                ptr += strlen((char*)ptr);
            }

            if(remaining < MAX_RESPONSE_LENGTH)
            {
                OCEntityHandlerResponse response = {0};

                response.ehResult = OC_EH_OK;
                response.payload = discoveryResBuf;
                response.payloadSize = strlen((const char *)discoveryResBuf) + 1;
                response.persistentBufferFlag = 0;
                response.requestHandle = (OCRequestHandle) request;
                response.resourceHandle = (OCResourceHandle) resource;

                result = OCDoResponse(&response);
            }
        }
        #ifdef WITH_PRESENCE
        else
        {
            if(resource->resourceProperties & OC_ACTIVE){
                SendPresenceNotification(NULL);
            }
        }
        #endif
    }
    result = OC_STACK_VIRTUAL_DO_NOT_HANDLE;
    return result;
}

static OCStackResult
HandleDefaultDeviceEntityHandler (OCServerRequest *request)
{
    OCStackResult result = OC_STACK_OK;
    OCEntityHandlerResult ehResult = OC_EH_ERROR;
    OCEntityHandlerRequest ehRequest = {0};

    OC_LOG(INFO, TAG, PCF("Entering HandleResourceWithDefaultDeviceEntityHandler"));
    result = FormOCEntityHandlerRequest(&ehRequest, (OCRequestHandle) request,
            request->method, (OCResourceHandle) NULL, request->query,
            request->reqJSONPayload, request->numRcvdVendorSpecificHeaderOptions,
            request->rcvdVendorSpecificHeaderOptions, (OCObserveAction)request->observationOption, (OCObservationId)0);
    VERIFY_SUCCESS(result, OC_STACK_OK);

    // At this point we know for sure that defaultDeviceHandler exists
    ehResult = defaultDeviceHandler(OC_REQUEST_FLAG, &ehRequest,
                                  (char*) request->resourceUrl);
    if(ehResult == OC_EH_SLOW)
    {
        OC_LOG(INFO, TAG, PCF("This is a slow resource"));
        request->slowFlag = 1;
    }
    else if(ehResult == OC_EH_ERROR)
    {
        FindAndDeleteServerRequest(request);
    }
    result = EntityHandlerCodeToOCStackCode(ehResult);
exit:
    return result;
}

static OCStackResult
HandleResourceWithEntityHandler (OCServerRequest *request,
                                 OCResource *resource,
                                 uint8_t collectionResource)
{
    OCStackResult result = OC_STACK_ERROR;
    OCEntityHandlerResult ehResult = OC_EH_ERROR;
    OCEntityHandlerFlag ehFlag = OC_REQUEST_FLAG;
    ResourceObserver *resObs = NULL;

    OCEntityHandlerRequest ehRequest = {0};

    OC_LOG(INFO, TAG, PCF("Entering HandleResourceWithEntityHandler"));
    result = FormOCEntityHandlerRequest(&ehRequest, (OCRequestHandle) request,
            request->method, (OCResourceHandle) resource, request->query,
            request->reqJSONPayload, request->numRcvdVendorSpecificHeaderOptions,
            request->rcvdVendorSpecificHeaderOptions,
            (OCObserveAction)request->observationOption, 0);
    VERIFY_SUCCESS(result, OC_STACK_OK);

    if(ehRequest.obsInfo.action == OC_OBSERVE_NO_OPTION)
    {
        OC_LOG(INFO, TAG, PCF("No observation requested"));
        ehFlag = OC_REQUEST_FLAG;
    }
    else if(ehRequest.obsInfo.action == OC_OBSERVE_REGISTER &&
            !collectionResource)
    {
        OC_LOG(INFO, TAG, PCF("Registering observation requested"));
        result = GenerateObserverId(&ehRequest.obsInfo.obsId);
        VERIFY_SUCCESS(result, OC_STACK_OK);

        result = AddObserver ((const char*)(request->resourceUrl),
                (const char *)(request->query),
                ehRequest.obsInfo.obsId, &request->requestToken,
                &request->requesterAddr, resource, request->qos);
        if(result == OC_STACK_OK)
        {
            OC_LOG(DEBUG, TAG, PCF("Added observer successfully"));
            request->observeResult = OC_STACK_OK;
            ehFlag = (OCEntityHandlerFlag)(OC_REQUEST_FLAG | OC_OBSERVE_FLAG);
        }
        else
        {
            result = OC_STACK_OK;
            request->observeResult = OC_STACK_ERROR;
            OC_LOG(DEBUG, TAG, PCF("Observer Addition failed"));
            ehFlag = OC_REQUEST_FLAG;
        }

    }
    else if(ehRequest.obsInfo.action == OC_OBSERVE_DEREGISTER &&
            !collectionResource)
    {
        OC_LOG(INFO, TAG, PCF("Deregistering observation requested"));
        resObs = GetObserverUsingToken (&request->requestToken);
        if (NULL == resObs)
        {
            // Stack does not contain this observation request
            // Either token is incorrect or observation list is corrupted
            result = OC_STACK_ERROR;
            goto exit;
        }
        ehRequest.obsInfo.obsId = resObs->observeId;
        ehFlag = (OCEntityHandlerFlag)(ehFlag | OC_OBSERVE_FLAG);

        result = DeleteObserverUsingToken (&request->requestToken);
        if(result == OC_STACK_OK)
        {
            OC_LOG(DEBUG, TAG, PCF("Removed observer successfully"));
            request->observeResult = OC_STACK_OK;
        }
        else
        {
            result = OC_STACK_OK;
            request->observeResult = OC_STACK_ERROR;
            OC_LOG(DEBUG, TAG, PCF("Observer Removal failed"));
        }
    }
    else
    {
        result = OC_STACK_ERROR;
        goto exit;
    }

    ehResult = resource->entityHandler(ehFlag, &ehRequest);
    if(ehResult == OC_EH_SLOW)
    {
        OC_LOG(INFO, TAG, PCF("This is a slow resource"));
        request->slowFlag = 1;
    }
    else if(ehResult == OC_EH_ERROR)
    {
        FindAndDeleteServerRequest(request);
    }
    result = EntityHandlerCodeToOCStackCode(ehResult);
exit:
    return result;
}

static OCStackResult
HandleCollectionResourceDefaultEntityHandler (OCServerRequest *request,
                                              OCResource *resource)
{
    OCStackResult result = OC_STACK_ERROR;
    OCEntityHandlerRequest ehRequest = {0};

    result = FormOCEntityHandlerRequest(&ehRequest, (OCRequestHandle) request,
            request->method, (OCResourceHandle) resource, request->query,
            request->reqJSONPayload, request->numRcvdVendorSpecificHeaderOptions,
            request->rcvdVendorSpecificHeaderOptions,
            (OCObserveAction)request->observationOption, (OCObservationId) 0);
    if(result != OC_STACK_OK)
    {
        return result;
    }

    return (DefaultCollectionEntityHandler (OC_REQUEST_FLAG, &ehRequest));
}

OCStackResult
ProcessRequest(ResourceHandling resHandling, OCResource *resource, OCServerRequest *request)
{
    OCStackResult ret = OC_STACK_OK;

    switch (resHandling)
    {
        case OC_RESOURCE_VIRTUAL:
        {
            ret = HandleVirtualResource (request, resource);
            break;
        }
        case OC_RESOURCE_DEFAULT_DEVICE_ENTITYHANDLER:
        {
            ret = HandleDefaultDeviceEntityHandler(request);
            break;
        }
        case OC_RESOURCE_NOT_COLLECTION_DEFAULT_ENTITYHANDLER:
        {
            OC_LOG(INFO, TAG, PCF("OC_RESOURCE_NOT_COLLECTION_DEFAULT_ENTITYHANDLER"));
            return OC_STACK_ERROR;
        }
        case OC_RESOURCE_NOT_COLLECTION_WITH_ENTITYHANDLER:
        {
            ret = HandleResourceWithEntityHandler (request, resource, 0);
            break;
        }
        case OC_RESOURCE_COLLECTION_WITH_ENTITYHANDLER:
        {
            ret = HandleResourceWithEntityHandler (request, resource, 1);
            break;
        }
        case OC_RESOURCE_COLLECTION_DEFAULT_ENTITYHANDLER:
        {
            ret = HandleCollectionResourceDefaultEntityHandler (request, resource);
            break;
        }
        case OC_RESOURCE_NOT_SPECIFIED:
        {
            ret = OC_STACK_NO_RESOURCE;
            break;
        }
        default:
        {
            OC_LOG(INFO, TAG, PCF("Invalid Resource Determination"));
            return OC_STACK_ERROR;
        }
    }
    return ret;
}

void DeleteDeviceInfo()
{
    if(savedDeviceInfo)
    {
        cJSON_Delete(savedDeviceInfo);
    }
}

OCStackResult SaveDeviceInfo(OCDeviceInfo deviceInfo)
{
    DeleteDeviceInfo();

    savedDeviceInfo = cJSON_CreateObject();
    cJSON *repObj = NULL;

    cJSON_AddItemToObject (savedDeviceInfo, OC_RSRVD_HREF,
            cJSON_CreateString(GetVirtualResourceUri(OC_DEVICE_URI)));

    cJSON_AddItemToObject (savedDeviceInfo, "rep", repObj = cJSON_CreateObject());

    if (deviceInfo.contentType)
    {
        cJSON_AddItemToObject (repObj, "ct",
                cJSON_CreateString(deviceInfo.contentType));
    }

    if (deviceInfo.dateOfManufacture)
    {
        cJSON_AddItemToObject (repObj, "mndt",
                cJSON_CreateString(deviceInfo.dateOfManufacture));
    }

    if (deviceInfo.deviceName)
    {
        cJSON_AddItemToObject (repObj, "dn",
                cJSON_CreateString(deviceInfo.deviceName));
    }

    if (deviceInfo.deviceUUID)
    {
        cJSON_AddItemToObject (repObj, "di",
                cJSON_CreateString(deviceInfo.deviceUUID));
    }

    if (deviceInfo.firmwareVersion)
    {
        cJSON_AddItemToObject (repObj, "mnfv",
                cJSON_CreateString(deviceInfo.firmwareVersion));
    }

    if (deviceInfo.hostName)
    {
        cJSON_AddItemToObject (repObj, "hn", cJSON_CreateString(deviceInfo.hostName));
    }

    if (deviceInfo.manufacturerName)
    {
        if(strlen(deviceInfo.manufacturerName) > MAX_MANUFACTURER_NAME_LENGTH)
        {
            DeleteDeviceInfo();
            return OC_STACK_INVALID_PARAM;
        }

        cJSON_AddItemToObject (repObj, "mnmn",
                cJSON_CreateString(deviceInfo.manufacturerName));
    }

    if (deviceInfo.manufacturerUrl)
    {
        if(strlen(deviceInfo.manufacturerUrl) > MAX_MANUFACTURER_URL_LENGTH)
        {
            DeleteDeviceInfo();
            return OC_STACK_INVALID_PARAM;
        }

        cJSON_AddItemToObject (repObj, "mnml",
                cJSON_CreateString(deviceInfo.manufacturerUrl));
    }

    if (deviceInfo.modelNumber)
    {
        cJSON_AddItemToObject (repObj, "mnmo",
                cJSON_CreateString(deviceInfo.modelNumber));
    }

    if (deviceInfo.platformVersion)
    {
        cJSON_AddItemToObject (repObj, "mnpv",
                cJSON_CreateString(deviceInfo.platformVersion));
    }

    if (deviceInfo.supportUrl)
    {
        cJSON_AddItemToObject (repObj, "mnsl",
                cJSON_CreateString(deviceInfo.supportUrl));
    }

    if (deviceInfo.version)
    {
        cJSON_AddItemToObject (repObj, "icv",
                cJSON_CreateString(deviceInfo.version));
    }

    return OC_STACK_OK;
}

