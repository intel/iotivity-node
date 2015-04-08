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


//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#define _POSIX_C_SOURCE 200112L
#include <string.h>

#include "ocstack.h"
#include "ocstackinternal.h"
#include "ocresourcehandler.h"
#include "occlientcb.h"
#include "ocobserve.h"
#include "ocrandom.h"
#include "debug.h"
#include "occoap.h"
#include "ocmalloc.h"
#include "ocserverrequest.h"

//-----------------------------------------------------------------------------
// Typedefs
//-----------------------------------------------------------------------------
typedef enum {
    OC_STACK_UNINITIALIZED = 0, OC_STACK_INITIALIZED, OC_STACK_UNINIT_IN_PROGRESS
} OCStackState;

#ifdef WITH_PRESENCE
typedef enum {
    OC_PRESENCE_UNINITIALIZED = 0, OC_PRESENCE_INITIALIZED
} OCPresenceState;
#endif

//-----------------------------------------------------------------------------
// Private variables
//-----------------------------------------------------------------------------
static OCStackState stackState = OC_STACK_UNINITIALIZED;

OCResource *headResource = NULL;
#ifdef WITH_PRESENCE
static OCPresenceState presenceState = OC_PRESENCE_UNINITIALIZED;
static PresenceResource presenceResource;
uint8_t PresenceTimeOutSize = 0;
uint32_t PresenceTimeOut[] = {50, 75, 85, 95, 100};
#endif

OCMode myStackMode;
OCDeviceEntityHandler defaultDeviceHandler;

//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------
#define TAG  PCF("OCStack")
#define VERIFY_NON_NULL(arg, logLevel, retVal) { if (!(arg)) { OC_LOG((logLevel), \
             TAG, PCF(#arg " is NULL")); return (retVal); } }

//TODO: we should allow the server to define this
#define MAX_OBSERVE_AGE (0x2FFFFUL)
//-----------------------------------------------------------------------------
// Externs
//-----------------------------------------------------------------------------
extern void DeinitOCSecurityInfo();

//-----------------------------------------------------------------------------
// Internal API function
//-----------------------------------------------------------------------------

// This internal function is called to update the stack with the status of
// observers and communication failures
OCStackResult OCStackFeedBack(OCCoAPToken * token, uint8_t status)
{
    OCStackResult result = OC_STACK_ERROR;
    ResourceObserver * observer = NULL;
    OCEntityHandlerRequest ehRequest = {0};

    switch(status)
    {
    case OC_OBSERVER_NOT_INTERESTED:
        OC_LOG(DEBUG, TAG, PCF("observer is not interested in our notifications anymore"));
        observer = GetObserverUsingToken (token);
        if(observer)
        {
            result = FormOCEntityHandlerRequest(&ehRequest, (OCRequestHandle) NULL,
                    OC_REST_NOMETHOD, (OCResourceHandle) NULL, NULL, NULL, 0,
                    NULL, OC_OBSERVE_DEREGISTER, observer->observeId);
            if(result != OC_STACK_OK)
            {
                return result;
            }
            observer->resource->entityHandler(OC_OBSERVE_FLAG, &ehRequest);
        }
        //observer is not observing anymore
        result = DeleteObserverUsingToken (token);
        if(result == OC_STACK_OK)
        {
            OC_LOG(DEBUG, TAG, PCF("Removed observer successfully"));
        }
        else
        {
            result = OC_STACK_OK;
            OC_LOG(DEBUG, TAG, PCF("Observer Removal failed"));
        }
        break;
    case OC_OBSERVER_STILL_INTERESTED:
        //observer is still interested
        OC_LOG(DEBUG, TAG, PCF("observer is interested in our \
                notifications, reset the failedCount"));
        observer = GetObserverUsingToken(token);
        if(observer)
        {
            observer->forceHighQos = 0;
            observer->failedCommCount = 0;
            result = OC_STACK_OK;
        }
        else
        {
            result = OC_STACK_OBSERVER_NOT_FOUND;
        }
        break;
    case OC_OBSERVER_FAILED_COMM:
        //observer is not reachable
        OC_LOG(DEBUG, TAG, PCF("observer is unreachable"));
        observer = GetObserverUsingToken(token);
        if(observer)
        {
            if(observer->failedCommCount >= MAX_OBSERVER_FAILED_COMM)
            {
                result = FormOCEntityHandlerRequest(&ehRequest, (OCRequestHandle) NULL,
                        OC_REST_NOMETHOD, (OCResourceHandle) NULL, NULL, NULL, 0,
                        NULL, OC_OBSERVE_DEREGISTER, observer->observeId);
                if(result != OC_STACK_OK)
                {
                    return OC_STACK_ERROR;
                }
                observer->resource->entityHandler(OC_OBSERVE_FLAG, &ehRequest);
                //observer is unreachable
                result = DeleteObserverUsingToken (token);
                if(result == OC_STACK_OK)
                {
                    OC_LOG(DEBUG, TAG, PCF("Removed observer successfully"));
                }
                else
                {
                    result = OC_STACK_OK;
                    OC_LOG(DEBUG, TAG, PCF("Observer Removal failed"));
                }
            }
            else
            {
                observer->failedCommCount++;
                result = OC_STACK_CONTINUE;
            }
            observer->forceHighQos = 1;
            OC_LOG_V(DEBUG, TAG, "Failed count for this observer is %d",observer->failedCommCount);
        }
        break;
    default:
        OC_LOG(ERROR, TAG, PCF("Unknown status"));
        result = OC_STACK_ERROR;
        break;
        }
    return result;
}

//This function will be called back by occoap layer when a request is received
OCStackResult HandleStackRequests(OCServerProtocolRequest * protocolRequest)
{
    OC_LOG(INFO, TAG, PCF("Entering HandleStackRequests (OCStack Layer)"));

    OCStackResult result = OC_STACK_ERROR;
    ResourceHandling resHandling;
    OCResource *resource;

    OCServerRequest * request = GetServerRequestUsingToken(protocolRequest->requestToken);
    if(!request)
    {
        OC_LOG(INFO, TAG, PCF("This is a new Server Request"));
        result = AddServerRequest(&request, protocolRequest->coapID,
                protocolRequest->delayedResNeeded, protocolRequest->secured, 0,
                protocolRequest->method, protocolRequest->numRcvdVendorSpecificHeaderOptions,
                protocolRequest->observationOption, protocolRequest->qos,
                protocolRequest->query, protocolRequest->rcvdVendorSpecificHeaderOptions,
                protocolRequest->reqJSONPayload, &protocolRequest->requestToken,
                &protocolRequest->requesterAddr, protocolRequest->resourceUrl,
                protocolRequest->reqTotalSize);
        if (OC_STACK_OK != result)
        {
            OC_LOG(ERROR, TAG, PCF("Error adding server request"));
            return result;
        }
        VERIFY_NON_NULL(request, ERROR, OC_STACK_NO_MEMORY);

        if(!protocolRequest->reqMorePacket)
        {
            request->requestComplete = 1;
        }
    }
    else
    {
        OC_LOG(INFO, TAG, PCF("This is either a repeated Server Request or blocked Server Request"));
    }

    if(request->requestComplete)
    {
        OC_LOG(INFO, TAG, PCF("This Server Request is complete"));
        result = DetermineResourceHandling (request, &resHandling, &resource);
        if (result == OC_STACK_OK)
        {
            result = ProcessRequest(resHandling, resource, request);
        }
        else
        {
            result = OC_STACK_ERROR;
        }
    }
    else
    {
        OC_LOG(INFO, TAG, PCF("This Server Request is incomplete"));
        result = OC_STACK_CONTINUE;
    }
    return result;
}

//This function will be called back by occoap layer when a response is received
void HandleStackResponses(OCResponse * response)
{
    OCStackApplicationResult result = OC_STACK_DELETE_TRANSACTION;
    OC_LOG(INFO, TAG, PCF("Entering HandleStackResponses (OCStack Layer)"));

    if (response->cbNode)
    {
        OC_LOG(INFO, TAG, PCF("Calling into application address space"));
        result = response->cbNode->callBack(response->cbNode->context,
                response->cbNode->handle, response->clientResponse);
        if (result == OC_STACK_DELETE_TRANSACTION ||
                response->clientResponse->result == OC_STACK_COMM_ERROR ||
                response->clientResponse->result == OC_STACK_RESOURCE_DELETED)
        {
            FindAndDeleteClientCB(response->cbNode);
        }
    }
}

int ParseIPv4Address(unsigned char * ipAddrStr, uint8_t * ipAddr, uint16_t * port)
{
    size_t index = 0;
    unsigned char *itr, *coap;
    uint8_t dotCount = 0;

    ipAddr[index] = 0;
    *port = 0;
    /* search for scheme */
    itr = ipAddrStr;
    if (!isdigit((unsigned char) *ipAddrStr))
    {
        coap = (unsigned char *) OC_COAP_SCHEME;
        while (*coap && tolower(*itr) == *coap)
        {
            coap++;
            itr++;
        }
    }
    ipAddrStr = itr;

    while (*ipAddrStr) {
        if (isdigit((unsigned char) *ipAddrStr))
        {
            ipAddr[index] *= 10;
            ipAddr[index] += *ipAddrStr - '0';
        }
        else if ((unsigned char) *ipAddrStr == '.')
        {
            index++;
            dotCount++;
            ipAddr[index] = 0;
        }
        else
        {
            break;
        }
        ipAddrStr++;
    }
    if(*ipAddrStr == ':')
    {
        ipAddrStr++;
        while (*ipAddrStr){
            if (isdigit((unsigned char) *ipAddrStr))
            {
                *port *= 10;
                *port += *ipAddrStr - '0';
            }
            else
            {
                break;
            }
            ipAddrStr++;
        }
    }


    if (ipAddr[0] < 255 && ipAddr[1] < 255 && ipAddr[2] < 255 && ipAddr[3] < 255
            && dotCount == 3)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

//-----------------------------------------------------------------------------
// Private internal function prototypes
//-----------------------------------------------------------------------------

static OCDoHandle GenerateInvocationHandle();
static OCStackResult initResources();
static void insertResource(OCResource *resource);
static OCResource *findResource(OCResource *resource);
static void insertResourceType(OCResource *resource,
        OCResourceType *resourceType);
static OCResourceType *findResourceTypeAtIndex(OCResourceHandle handle,
        uint8_t index);
static void insertResourceInterface(OCResource *resource,
        OCResourceInterface *resourceInterface);
static OCResourceInterface *findResourceInterfaceAtIndex(
        OCResourceHandle handle, uint8_t index);
static void deleteResourceType(OCResourceType *resourceType);
static void deleteResourceInterface(OCResourceInterface *resourceInterface);
static void deleteResourceElements(OCResource *resource);
static int deleteResource(OCResource *resource);
static void deleteAllResources();
static void incrementSequenceNumber(OCResource * resPtr);
static OCStackResult verifyUriQueryLength(const char * inputUri,
        uint16_t uriLen);
static uint8_t OCIsPacketTransferRequired(const char *request, const char *response, uint16_t size);
OCStackResult getResourceType(const char * uri, unsigned char** resourceType, char ** newURI);

//-----------------------------------------------------------------------------
// Public APIs
//-----------------------------------------------------------------------------

/**
 * Initialize the OC Stack.  Must be called prior to starting the stack.
 *
 * @param ipAddr
 *     IP Address of host device
 * @param port
 *     Port of host device
 * @param mode
 *     Host device is client, server, or client-server
 *
 * @return
 *     OC_STACK_OK    - no errors
 *     OC_STACK_ERROR - stack init error
 */
OCStackResult OCInit(const char *ipAddr, uint16_t port, OCMode mode)
{
    OCStackResult result = OC_STACK_ERROR;
    OC_LOG(INFO, TAG, PCF("Entering OCInit"));

    if (ipAddr)
    {
        OC_LOG_V(INFO, TAG, "IP Address = %s", ipAddr);
    }

    switch (mode)
    {
    case OC_CLIENT:
        OC_LOG(INFO, TAG, PCF("Client mode"));
        break;
    case OC_SERVER:
        OC_LOG(INFO, TAG, PCF("Server mode"));
        break;
    case OC_CLIENT_SERVER:
        OC_LOG(INFO, TAG, PCF("Client-server mode"));
        break;
    default:
        OC_LOG(ERROR, TAG, PCF("Invalid mode"));
        return OC_STACK_ERROR;
        break;
    }
    myStackMode = mode;

    defaultDeviceHandler = NULL;

#ifdef WITH_PRESENCE
    PresenceTimeOutSize = sizeof(PresenceTimeOut)/sizeof(PresenceTimeOut[0]) - 1;
#endif // WITH_PRESENCE

    // Make call to OCCoAP layer
    result = OCInitCoAP(ipAddr, (uint16_t) port, myStackMode);
    if (result == OC_STACK_OK)
    {
        stackState = OC_STACK_INITIALIZED;
    }
    // Initialize resource
    if(result == OC_STACK_OK && myStackMode != OC_CLIENT)
    {
        result = initResources();
    }
    if(result != OC_STACK_OK)
    {
        OC_LOG(ERROR, TAG, PCF("Stack initialization error"));
    }
    return result;
}

/**
 * Stop the OC stack.  Use for a controlled shutdown.
 * @return
 *     OC_STACK_OK    - no errors
 *     OC_STACK_ERROR - stack not initialized
 */
OCStackResult OCStop()
{
    OCStackResult result = OC_STACK_ERROR;

    OC_LOG(INFO, TAG, PCF("Entering OCStop"));

    if (stackState == OC_STACK_UNINIT_IN_PROGRESS)
    {
        OC_LOG(DEBUG, TAG, PCF("Stack already stopping, exiting"));
        return OC_STACK_OK;
    }
    else if (stackState != OC_STACK_INITIALIZED)
    {
        OC_LOG(ERROR, TAG, PCF("Stack not initialized"));
        return OC_STACK_ERROR;
    }

    stackState = OC_STACK_UNINIT_IN_PROGRESS;

    #ifdef WITH_PRESENCE
    // Ensure that the TTL associated with ANY and ALL presence notifications originating from
    // here send with the code "OC_STACK_PRESENCE_STOPPED" result.
    presenceResource.presenceTTL = 0;
    #endif // WITH_PRESENCE

    // Free memory dynamically allocated for resources
    deleteAllResources();
    DeleteDeviceInfo();

    // Make call to OCCoAP layer
    if (OCStopCoAP() == OC_STACK_OK)
    {
        // Remove all observers
        DeleteObserverList();
        // Remove all the client callbacks
        DeleteClientCBList();
        stackState = OC_STACK_UNINITIALIZED;
        result = OC_STACK_OK;
    } else {
        stackState = OC_STACK_INITIALIZED;
        result = OC_STACK_ERROR;
    }

    // Deinit security blob
    DeinitOCSecurityInfo();

    if (result != OC_STACK_OK) {
        OC_LOG(ERROR, TAG, PCF("Stack stop error"));
    }

    return result;
}

/**
 * Verify the lengths of the URI and the query separately
 *
 * @param inputUri       - Input URI and query.
 * @param uriLen         - The length of the initial URI with query.
 *
 * Note: The '?' that appears after the URI is not considered as
 * a part of the query.
 */
OCStackResult verifyUriQueryLength(const char *inputUri, uint16_t uriLen)
{
    char *query;

    query = strchr (inputUri, '?');

    if (query != NULL)
    {
        if((query - inputUri) > MAX_URI_LENGTH)
        {
            return OC_STACK_INVALID_URI;
        }

        if((inputUri + uriLen - 1 - query) > MAX_QUERY_LENGTH)
        {
            return OC_STACK_INVALID_QUERY;
        }
    }
    else if(uriLen > MAX_URI_LENGTH)
    {
        return OC_STACK_INVALID_URI;
    }
    return OC_STACK_OK;
}

/**
 * Discover or Perform requests on a specified resource (specified by that Resource's respective URI).
 *
 * @param handle             - @ref OCDoHandle to refer to the request sent out on behalf of calling this API.
 * @param method             - @ref OCMethod to perform on the resource
 * @param requiredUri        - URI of the resource to interact with
 * @param referenceUri       - URI of the reference resource
 * @param request            - JSON encoded request
 * @param qos                - quality of service
 * @param cbData             - struct that contains asynchronous callback function that is invoked
 *                             by the stack when discovery or resource interaction is complete
 * @param options            - The address of an array containing the vendor specific header
 *                             header options to be sent with the request
 * @param numOptions         - Number of vendor specific header options to be included
 *
 * @return
 *     OC_STACK_OK               - no errors
 *     OC_STACK_INVALID_CALLBACK - invalid callback function pointer
 *     OC_STACK_INVALID_METHOD   - invalid resource method
 *     OC_STACK_INVALID_URI      - invalid required or reference URI
 */

OCStackResult OCDoResource(OCDoHandle *handle, OCMethod method, const char *requiredUri,
                           const char *referenceUri, const char *request,
                           OCQualityOfService qos, OCCallbackData *cbData,
                           OCHeaderOption * options, uint8_t numOptions)
{
    OCStackResult result = OC_STACK_ERROR;
    OCCoAPToken token;
    ClientCB *clientCB = NULL;
    unsigned char * requestUri = NULL;
    unsigned char * resourceType = NULL;
    char * newUri = (char *)requiredUri;
    (void) referenceUri;

    OC_LOG(INFO, TAG, PCF("Entering OCDoResource"));

    // Validate input parameters
    VERIFY_NON_NULL(cbData, FATAL, OC_STACK_INVALID_CALLBACK);
    VERIFY_NON_NULL(cbData->cb, FATAL, OC_STACK_INVALID_CALLBACK);

    TODO ("Need to form the final query by concatenating require and reference URI's");
    VERIFY_NON_NULL(requiredUri, FATAL, OC_STACK_INVALID_URI);

    uint16_t uriLen = strlen(requiredUri);

    // ToDo: We should also check if the requiredUri has a mutlicast address, then qos has to be OC_Low_QOS
    switch (method)
    {
        case OC_REST_GET:
        case OC_REST_PUT:
        case OC_REST_POST:
        case OC_REST_DELETE:
        case OC_REST_OBSERVE:
        case OC_REST_OBSERVE_ALL:
        case OC_REST_CANCEL_OBSERVE:
            break;
        #ifdef WITH_PRESENCE
        case OC_REST_PRESENCE:
            break;
        #endif
        default:
            result = OC_STACK_INVALID_METHOD;
            goto exit;
    }

    if((result = verifyUriQueryLength(requiredUri, uriLen)) != OC_STACK_OK)
    {
        goto exit;
    }

    if((request) && (strlen(request) > MAX_REQUEST_LENGTH))
    {
        result = OC_STACK_INVALID_PARAM;
        goto exit;
    }

#ifdef WITH_PRESENCE
    if(method == OC_REST_PRESENCE)
    {
        result = getResourceType(requiredUri, &resourceType, &newUri);
        if(resourceType) {
            OC_LOG_V(DEBUG, TAG, "Got Resource Type: %s", resourceType);
        }
        else
        {
            OC_LOG(DEBUG, TAG, "Got Resource Type is NULL.");
        }
        if(result != OC_STACK_OK)
        {
            goto exit;
        }
    }
#endif // WITH_PRESENCE

    requestUri = (unsigned char *) OCMalloc(uriLen + 1);
    if(requestUri)
    {
        memcpy(requestUri, newUri, (uriLen + 1));
    }
    else
    {
        result = OC_STACK_NO_MEMORY;
        goto exit;
    }

    *handle = GenerateInvocationHandle();
    if(!*handle)
    {
        result = OC_STACK_NO_MEMORY;
        goto exit;
    }

    // Generate token which will be used by OCStack to match responses received
    // with the request
    OCGenerateCoAPToken(&token);

    if((result = AddClientCB(&clientCB, cbData, &token, handle, method, requestUri, resourceType))
            != OC_STACK_OK)
    {
        result = OC_STACK_NO_MEMORY;
        goto exit;
    }

    // Make call to OCCoAP layer
    result = OCDoCoAPResource(method, qos, &token, newUri, request, options, numOptions);

exit:
    if(newUri != requiredUri)
    {
        OCFree(newUri);
    }
    if (result != OC_STACK_OK)
    {
        OC_LOG(ERROR, TAG, PCF("OCDoResource error"));
        FindAndDeleteClientCB(clientCB);
    }
    return result;
}

/**
 * Cancel a request associated with a specific @ref OCDoResource invocation.
 *
 * @param handle - Used to identify a specific OCDoResource invocation.
 * @param qos    - used to specify Quality of Service (read below for more info)
 * @param options- used to specify vendor specific header options when sending
 *                 explicit observe cancellation
 * @param numOptions- Number of header options to be included
 *
 * @return
 *     OC_STACK_OK               - No errors; Success
 *     OC_STACK_INVALID_PARAM    - The handle provided is invalid.
 */
OCStackResult OCCancel(OCDoHandle handle, OCQualityOfService qos, OCHeaderOption * options,
        uint8_t numOptions) {
    /*
     * This ftn is implemented one of two ways in the case of observation:
     *
     * 1. qos == OC_NON_CONFIRMABLE. When observe is unobserved..
     *      Remove the callback associated on client side.
     *      When the next notification comes in from server,
     *      reply with RESET message to server.
     *      Keep in mind that the server will react to RESET only
     *      if the last notification was sent ans CON
     *
     * 2. qos == OC_CONFIRMABLE. When OCCancel is called,
     *      and it is associated with an observe request
     *      (i.e. ClientCB->method == OC_REST_OBSERVE || OC_REST_OBSERVE_ALL),
     *      Send CON Observe request to server with
     *      observe flag = OC_RESOURCE_OBSERVE_DEREGISTER.
     *      Remove the callback associated on client side.
     */
    OCStackResult ret = OC_STACK_OK;

    if(!handle) {
        return OC_STACK_INVALID_PARAM;
    }

    OC_LOG(INFO, TAG, PCF("Entering OCCancel"));

    ClientCB *clientCB = GetClientCB(NULL, handle, NULL);

    if(clientCB) {
        switch (clientCB->method)
        {
            case OC_REST_OBSERVE:
            case OC_REST_OBSERVE_ALL:
                if(qos == OC_HIGH_QOS)
                {
                    ret = OCDoCoAPResource(OC_REST_CANCEL_OBSERVE, qos,
                            &(clientCB->token), (const char *) clientCB->requestUri, NULL, options,
                            numOptions);
                }
                else
                {
                    FindAndDeleteClientCB(clientCB);
                }
                break;
            #ifdef WITH_PRESENCE
            case OC_REST_PRESENCE:
                FindAndDeleteClientCB(clientCB);
                break;
            #endif
            default:
                return OC_STACK_INVALID_METHOD;
        }
    }
    return ret;
}
#ifdef WITH_PRESENCE
OCStackResult OCProcessPresence()
{
    OCStackResult result = OC_STACK_OK;
    uint8_t ipAddr[4] = { 0 };
    uint16_t port = 0;

    OC_LOG(INFO, TAG, PCF("Entering RequestPresence"));
    ClientCB* cbNode = NULL;
    OCDevAddr dst;
    OCClientResponse clientResponse;
    OCResponse * response = NULL;

    LL_FOREACH(cbList, cbNode) {
        if(OC_REST_PRESENCE == cbNode->method)
        {
            if(cbNode->presence)
            {
                uint32_t now = GetTime(0);
                OC_LOG_V(DEBUG, TAG, "----------------this TTL level %d", cbNode->presence->TTLlevel);
                OC_LOG_V(DEBUG, TAG, "----------------current ticks %d", now);


                if(cbNode->presence->TTLlevel >= (PresenceTimeOutSize + 1))
                {
                    goto exit;
                }

                if(cbNode->presence->TTLlevel < PresenceTimeOutSize){
                    OC_LOG_V(DEBUG, TAG, "----------------timeout ticks %d",
                            cbNode->presence->timeOut[cbNode->presence->TTLlevel]);
                }

                if(cbNode->presence->TTLlevel >= PresenceTimeOutSize)
                {
                    OC_LOG(DEBUG, TAG, PCF("----------------No more timeout ticks"));
                    if (ParseIPv4Address( cbNode->requestUri, ipAddr, &port))
                    {
                        OCBuildIPv4Address(ipAddr[0], ipAddr[1], ipAddr[2], ipAddr[3], port,
                                &dst);
                        result = FormOCClientResponse(&clientResponse, OC_STACK_PRESENCE_TIMEOUT,
                                (OCDevAddr *) &dst, 0, NULL);
                        if(result != OC_STACK_OK)
                        {
                            goto exit;
                        }
                        result = FormOCResponse(&response, cbNode, 0, &clientResponse);
                        if(result != OC_STACK_OK)
                        {
                            goto exit;
                        }

                        // Increment the TTLLevel (going to a next state), so we don't keep
                        // sending presence notification to client.
                        cbNode->presence->TTLlevel++;
                        OC_LOG_V(DEBUG, TAG, "----------------moving to TTL level %d",
                                                cbNode->presence->TTLlevel);
                    }
                    else
                    {
                        result = OC_STACK_INVALID_IP;
                        goto exit;
                    }
                    HandleStackResponses(response);
                }
                if(now >= cbNode->presence->timeOut[cbNode->presence->TTLlevel])
                {
                    OC_LOG(DEBUG, TAG, PCF("time to test server presence =========="));
                    OCCoAPToken token;
                    OCGenerateCoAPToken(&token);
                    result = OCDoCoAPResource(OC_REST_GET, OC_LOW_QOS,
                            &token, (const char *)cbNode->requestUri, NULL, NULL, 0);
                    if(result != OC_STACK_OK)
                    {
                        goto exit;
                    }
                    cbNode->presence->TTLlevel++;
                    OC_LOG_V(DEBUG, TAG, "----------------moving to TTL level %d", cbNode->presence->TTLlevel);
                }
            }
        }
    }
exit:
    if (result != OC_STACK_OK)
    {
        OC_LOG(ERROR, TAG, PCF("OCProcessPresence error"));
    }
    return result;
}
#endif

/**
 * Called in main loop of OC client or server.  Allows low-level processing of
 * stack services.
 *
 * @return
 *     OC_STACK_OK    - no errors
 *     OC_STACK_ERROR - stack process error
 */
OCStackResult OCProcess() {

    OC_LOG(INFO, TAG, PCF("Entering OCProcess"));
    #ifdef WITH_PRESENCE
    OCProcessPresence();
    #endif
    OCProcessCoAP();

    return OC_STACK_OK;
}

#ifdef WITH_PRESENCE
/**
 * When operating in @ref OCServer or @ref OCClientServer mode, this API will start sending out
 * presence notifications to clients via multicast. Once this API has been called with a success,
 * clients may query for this server's presence and this server's stack will respond via multicast.
 *
 * Server can call this function when it comes online for the first time, or when it comes back
 * online from offline mode, or when it re enters network.
 *
 * @param ttl - Time To Live in seconds
 * Note: If ttl is '0', then the default stack value will be used (60 Seconds).
 *
 * @return
 *     OC_STACK_OK      - No errors; Success
 */
OCStackResult OCStartPresence(const uint32_t ttl)
{
    OCChangeResourceProperty(
            &(((OCResource *)presenceResource.handle)->resourceProperties),
            OC_ACTIVE, 1);

    if(ttl > 0)
    {
        presenceResource.presenceTTL = ttl;
    }

    if(OC_PRESENCE_UNINITIALIZED == presenceState)
    {
        OCDevAddr multiCastAddr;
        OCCoAPToken token;

        presenceState = OC_PRESENCE_INITIALIZED;
        OCGenerateCoAPToken(&token);
        OCBuildIPv4Address(224, 0, 1, 187, 5683, &multiCastAddr);
        //add the presence observer
        AddObserver(OC_PRESENCE_URI, NULL, 0, &token, &multiCastAddr,
            (OCResource *)presenceResource.handle, OC_LOW_QOS);
    }

    // Each time OCStartPresence is called
    // a different random 32-bit integer number is used
    ((OCResource *)presenceResource.handle)->sequenceNum = OCGetRandom();

    return SendPresenceNotification(NULL);
}

/**
 * When operating in @ref OCServer or @ref OCClientServer mode, this API will stop sending out
 * presence notifications to clients via multicast. Once this API has been called with a success,
 * this server's stack will not respond to clients querying for this server's presence.
 *
 * Server can call this function when it is terminating, going offline, or when going
 * away from network.
 *
 * @return
 *     OC_STACK_OK      - No errors; Success
 */
OCStackResult OCStopPresence()
{
    OCStackResult result = OC_STACK_ERROR;
    //make resource inactive
    result = OCChangeResourceProperty(
            &(((OCResource *) presenceResource.handle)->resourceProperties),
            OC_ACTIVE, 0);
    result = SendPresenceNotification(NULL);

    return result;
}
#endif


OCStackResult OCSetDefaultDeviceEntityHandler(OCDeviceEntityHandler entityHandler)
{
    defaultDeviceHandler = entityHandler;

    return OC_STACK_OK;
}

OCStackResult OCSetDeviceInfo(OCDeviceInfo deviceInfo)
{
    OC_LOG(INFO, TAG, PCF("Entering OCSetDeviceInfo"));

    if(myStackMode == OC_CLIENT)
    {
        return OC_STACK_ERROR;
    }

    return SaveDeviceInfo(deviceInfo);
}

/**
 * Create a resource
 *
 * @param handle - pointer to handle to newly created resource.  Set by ocstack.  Used to refer to resource
 * @param resourceTypeName - name of resource type.  Example: "core.led"
 * @param resourceInterfaceName - name of resource interface.  Example: "core.rw"
 * @param uri - URI of the resource.  Example:  "/a/led"
 * @param entityHandler - entity handler function that is called by ocstack to handle requests, etc
 *                        NULL for default entity handler
 * @param resourceProperties - properties supported by resource.  Example: OC_DISCOVERABLE|OC_OBSERVABLE
 *
 * @return
 *     OC_STACK_OK    - no errors
 *     OC_STACK_ERROR - stack process error
 */
OCStackResult OCCreateResource(OCResourceHandle *handle,
        const char *resourceTypeName,
        const char *resourceInterfaceName,
        const char *uri, OCEntityHandler entityHandler,
        uint8_t resourceProperties) {

    OCResource *pointer = NULL;
    char *str = NULL;
    size_t size;
    OCStackResult result = OC_STACK_ERROR;

    OC_LOG(INFO, TAG, PCF("Entering OCCreateResource"));

    if(myStackMode == OC_CLIENT)
    {
        return result;
    }
    // Validate parameters
    if(!uri || (strlen(uri) == 0))
    {
        OC_LOG(ERROR, TAG, PCF("URI is invalid"));
        return OC_STACK_INVALID_URI;
    }
    // Is it presented during resource discovery?
    if (!handle || !resourceTypeName) {
        OC_LOG(ERROR, TAG, PCF("Input parameter is NULL"));
        return OC_STACK_INVALID_PARAM;
    }

    if(!resourceInterfaceName || strlen(resourceInterfaceName) == 0) {
        resourceInterfaceName = OC_RSRVD_INTERFACE_DEFAULT;
    }

    // Make sure resourceProperties bitmask has allowed properties specified
    if (resourceProperties
            > (OC_ACTIVE | OC_DISCOVERABLE | OC_OBSERVABLE | OC_SLOW | OC_SECURE)) {
        OC_LOG(ERROR, TAG, PCF("Invalid property"));
        return OC_STACK_INVALID_PARAM;
    }

    // If the headResource is NULL, then no resources have been created...
    pointer = headResource;
    if (pointer) {
        // At least one resources is in the resource list, so we need to search for
        // repeated URLs, which are not allowed.  If a repeat is found, exit with an error
        while (pointer) {
            if (strcmp(uri, pointer->uri) == 0) {
                OC_LOG(ERROR, TAG, PCF("URI already in use"));
                return OC_STACK_INVALID_PARAM;
            }
            pointer = pointer->next;
        }
    }
    // Create the pointer and insert it into the resource list
    pointer = (OCResource *) OCCalloc(1, sizeof(OCResource));
    if (!pointer) {
        goto exit;
    }
    pointer->sequenceNum = OC_OFFSET_SEQUENCE_NUMBER;

    insertResource(pointer);

    // Set the uri
    size = strlen(uri) + 1;
    str = (char *) OCMalloc(size);
    if (!str) {
        goto exit;
    }
    strncpy(str, uri, size);
    pointer->uri = str;

    // Set properties.  Set OC_ACTIVE
    pointer->resourceProperties = (OCResourceProperty) (resourceProperties
            | OC_ACTIVE);

    // Add the resourcetype to the resource
    result = BindResourceTypeToResource(pointer, resourceTypeName);
    if (result != OC_STACK_OK) {
        OC_LOG(ERROR, TAG, PCF("Error adding resourcetype"));
        goto exit;
    }

    // Add the resourceinterface to the resource
    result = BindResourceInterfaceToResource(pointer, resourceInterfaceName);
    if (result != OC_STACK_OK) {
        OC_LOG(ERROR, TAG, PCF("Error adding resourceinterface"));
        goto exit;
    }

    // If an entity handler has been passed, attach it to the newly created
    // resource.  Otherwise, set the default entity handler.
    if (entityHandler)
    {
        pointer->entityHandler = entityHandler;
    }
    else
    {
        pointer->entityHandler = defaultResourceEHandler;
    }

    *handle = pointer;
    result = OC_STACK_OK;

    #ifdef WITH_PRESENCE
    if(presenceResource.handle)
    {
        ((OCResource *)presenceResource.handle)->sequenceNum = OCGetRandom();
        SendPresenceNotification(pointer->rsrcType);
    }
    #endif
exit:
    if (result != OC_STACK_OK)
    {
        // Deep delete of resource and other dynamic elements that it contains
        deleteResource(pointer);
        OCFree(str);
    }
    return result;
}



/**
 * Create a resource. with host ip address for remote resource
 *
 * @param handle - pointer to handle to newly created resource.  Set by ocstack.
 *                 Used to refer to resource
 * @param resourceTypeName - name of resource type.  Example: "core.led"
 * @param resourceInterfaceName - name of resource interface.  Example: "core.rw"
 * @param host - HOST address of the remote resource.  Example:  "coap://xxx.xxx.xxx.xxx:xxxxx"
 * @param uri - URI of the resource.  Example:  "/a/led"
 * @param entityHandler - entity handler function that is called by ocstack to handle requests, etc
 *                        NULL for default entity handler
 * @param resourceProperties - properties supported by resource.
 *                             Example: OC_DISCOVERABLE|OC_OBSERVABLE
 *
 * @return
 *     OC_STACK_OK    - no errors
 *     OC_STACK_ERROR - stack process error
 */

OCStackResult OCCreateResourceWithHost(OCResourceHandle *handle,
        const char *resourceTypeName,
        const char *resourceInterfaceName,
        const char *host,
        const char *uri,
        OCEntityHandler entityHandler,
        uint8_t resourceProperties)
{
    char *str = NULL;
    size_t size;
    OCStackResult result = OC_STACK_ERROR;

    result = OCCreateResource(handle, resourceTypeName, resourceInterfaceName,
                                uri, entityHandler, resourceProperties);

    if (result != OC_STACK_ERROR)
    {
        // Set the uri
        size = strlen(host) + 1;
        str = (char *) OCMalloc(size);
        if (!str)
        {
            return OC_STACK_ERROR;
        }
        strncpy(str, host, size);
        ((OCResource *) *handle)->host = str;
    }

    return result;
}

/**
 * Add a resource to a collection resource.
 *
 * @param collectionHandle - handle to the collection resource
 * @param resourceHandle - handle to resource to be added to the collection resource
 *
 * @return
 *     OC_STACK_OK    - no errors
 *     OC_STACK_ERROR - stack process error
 *     OC_STACK_INVALID_PARAM - invalid collectionhandle
 */
OCStackResult OCBindResource(
        OCResourceHandle collectionHandle, OCResourceHandle resourceHandle) {
    OCResource *resource;
    uint8_t i;

    OC_LOG(INFO, TAG, PCF("Entering OCBindResource"));

    // Validate parameters
    VERIFY_NON_NULL(collectionHandle, ERROR, OC_STACK_ERROR);
    VERIFY_NON_NULL(resourceHandle, ERROR, OC_STACK_ERROR);
    // Container cannot contain itself
    if (collectionHandle == resourceHandle) {
        OC_LOG(ERROR, TAG, PCF("Added handle equals collection handle"));
        return OC_STACK_INVALID_PARAM;
    }

    // Use the handle to find the resource in the resource linked list
    resource = findResource((OCResource *) collectionHandle);
    if (!resource) {
        OC_LOG(ERROR, TAG, PCF("Collection handle not found"));
        return OC_STACK_INVALID_PARAM;
    }

    // Look for an open slot to add add the child resource.
    // If found, add it and return success
    for (i = 0; i < MAX_CONTAINED_RESOURCES; i++) {
        if (!resource->rsrcResources[i]) {
            resource->rsrcResources[i] = (OCResource *) resourceHandle;
            OC_LOG(INFO, TAG, PCF("resource bound"));
            return OC_STACK_OK;
        }
    }

    #ifdef WITH_PRESENCE
    if(presenceResource.handle)
    {
        ((OCResource *)presenceResource.handle)->sequenceNum = OCGetRandom();
        SendPresenceNotification(((OCResource *) resourceHandle)->rsrcType);
    }
    #endif

    // Unable to add resourceHandle, so return error
    return OC_STACK_ERROR;
}

/**
 * Remove a resource from a collection resource.
 *
 * @param collectionHandle - handle to the collection resource
 * @param resourceHandle - handle to resource to be added to the collection resource
 *
 * @return
 *     OC_STACK_OK    - no errors
 *     OC_STACK_ERROR - stack process error
 *     OC_STACK_INVALID_PARAM - invalid collectionHandle
 */
OCStackResult OCUnBindResource(
        OCResourceHandle collectionHandle, OCResourceHandle resourceHandle) {
    OCResource *resource;
    uint8_t i;

    OC_LOG(INFO, TAG, PCF("Entering OCUnBindResource"));

    // Validate parameters
    VERIFY_NON_NULL(collectionHandle, ERROR, OC_STACK_ERROR);
    VERIFY_NON_NULL(resourceHandle, ERROR, OC_STACK_ERROR);
    // Container cannot contain itself
    if (collectionHandle == resourceHandle) {
        OC_LOG(ERROR, TAG, PCF("removing handle equals collection handle"));
        return OC_STACK_INVALID_PARAM;
    }

    // Use the handle to find the resource in the resource linked list
    resource = findResource((OCResource *) collectionHandle);
    if (!resource) {
        OC_LOG(ERROR, TAG, PCF("Collection handle not found"));
        return OC_STACK_INVALID_PARAM;
    }

    // Look for an open slot to add add the child resource.
    // If found, add it and return success
    for (i = 0; i < MAX_CONTAINED_RESOURCES; i++) {
        if (resourceHandle == resource->rsrcResources[i]) {
            resource->rsrcResources[i] = (OCResource *) NULL;
            OC_LOG(INFO, TAG, PCF("resource unbound"));
            return OC_STACK_OK;
        }
    }

    OC_LOG(INFO, TAG, PCF("resource not found in collection"));

    #ifdef WITH_PRESENCE
    if(presenceResource.handle)
    {
        ((OCResource *)presenceResource.handle)->sequenceNum = OCGetRandom();
        SendPresenceNotification(((OCResource *) resourceHandle)->rsrcType);
    }
    #endif

    // Unable to add resourceHandle, so return error
    return OC_STACK_ERROR;
}

OCStackResult BindResourceTypeToResource(OCResource* resource,
                                            const char *resourceTypeName)
{
    OCResourceType *pointer = NULL;
    char *str = NULL;
    size_t size;
    OCStackResult result = OC_STACK_ERROR;

    OC_LOG(INFO, TAG, PCF("Entering BindResourceTypeToResource"));

    // Validate parameters
    VERIFY_NON_NULL(resourceTypeName, ERROR, OC_STACK_INVALID_PARAM);
    // TODO:  Does resource attribute resentation really have to be maintained in stack?
    // Is it presented during resource discovery?

    TODO ("Make sure that the resourcetypename doesn't already exist in the resource");

    // Create the resourcetype and insert it into the resource list
    pointer = (OCResourceType *) OCCalloc(1, sizeof(OCResourceType));
    if (!pointer) {
        goto exit;
    }

    // Set the resourceTypeName
    size = strlen(resourceTypeName) + 1;
    str = (char *) OCMalloc(size);
    if (!str) {
        goto exit;
    }
    strncpy(str, resourceTypeName, size);
    pointer->resourcetypename = str;

    insertResourceType(resource, pointer);
    result = OC_STACK_OK;

    exit: if (result != OC_STACK_OK) {
        OCFree(pointer);
        OCFree(str);
    }

    return result;
}

OCStackResult BindResourceInterfaceToResource(OCResource* resource,
        const char *resourceInterfaceName)
{
    OCResourceInterface *pointer = NULL;
    char *str = NULL;
    size_t size;
    OCStackResult result = OC_STACK_ERROR;

    OC_LOG(INFO, TAG, PCF("Entering BindResourceInterfaceToResource"));

    // Validate parameters
    VERIFY_NON_NULL(resourceInterfaceName, ERROR, OC_STACK_INVALID_PARAM);

    TODO ("Make sure that the resourceinterface name doesn't already exist in the resource");

    // Create the resourceinterface and insert it into the resource list
    pointer = (OCResourceInterface *) OCCalloc(1, sizeof(OCResourceInterface));
    if (!pointer) {
        goto exit;
    }

    // Set the resourceinterface name
    size = strlen(resourceInterfaceName) + 1;
    str = (char *) OCMalloc(size);
    if (!str) {
        goto exit;
    }
    strncpy(str, resourceInterfaceName, size);
    pointer->name = str;

    // Bind the resourceinterface to the resource
    insertResourceInterface(resource, pointer);

    result = OC_STACK_OK;

    exit: if (result != OC_STACK_OK) {
        OCFree(pointer);
        OCFree(str);
    }

    return result;
}

/**
 * Bind a resourcetype to a resource.
 *
 * @param handle - handle to the resource
 * @param resourceTypeName - name of resource type.  Example: "core.led"
 *
 * @return
 *     OC_STACK_OK    - no errors
 *     OC_STACK_ERROR - stack process error
 */
OCStackResult OCBindResourceTypeToResource(OCResourceHandle handle,
        const char *resourceTypeName) {

    OCStackResult result = OC_STACK_ERROR;
    OCResource *resource;

    // Make sure resource exists
    resource = findResource((OCResource *) handle);
    if (!resource) {
        OC_LOG(ERROR, TAG, PCF("Resource not found"));
        return OC_STACK_ERROR;
    }

    // call internal function
    result = BindResourceTypeToResource(resource, resourceTypeName);

    #ifdef WITH_PRESENCE
    if(presenceResource.handle)
    {
        ((OCResource *)presenceResource.handle)->sequenceNum = OCGetRandom();
        SendPresenceNotification(resource->rsrcType);
    }
    #endif

    return result;
}

/**
 * Bind a resourceinterface to a resource.
 *
 * @param handle - handle to the resource
 * @param resourceInterfaceName - name of resource interface.  Example: "oc.mi.b"
 *
 * @return
 *     OC_STACK_OK    - no errors
 *     OC_STACK_ERROR - stack process error
 */

OCStackResult OCBindResourceInterfaceToResource(OCResourceHandle handle,
        const char *resourceInterfaceName) {

    OCStackResult result = OC_STACK_ERROR;
    OCResource *resource;

    // Make sure resource exists
    resource = findResource((OCResource *) handle);
    if (!resource) {
        OC_LOG(ERROR, TAG, PCF("Resource not found"));
        return OC_STACK_ERROR;
    }

    // call internal function
    result = BindResourceInterfaceToResource(resource, resourceInterfaceName);

    #ifdef WITH_PRESENCE
    if(presenceResource.handle)
    {
        ((OCResource *)presenceResource.handle)->sequenceNum = OCGetRandom();
        SendPresenceNotification(resource->rsrcType);
    }
    #endif

    return result;
}

/**
 * Get the number of resources that have been created in the stack.
 *
 * @param numResources - pointer to count variable
 *
 * @return
 *     OC_STACK_OK    - no errors
 *     OC_STACK_ERROR - stack process error

 */
OCStackResult OCGetNumberOfResources(uint8_t *numResources) {
    OCResource *pointer = headResource;

    OC_LOG(INFO, TAG, PCF("Entering OCGetNumberOfResources"));
    VERIFY_NON_NULL(numResources, ERROR, OC_STACK_INVALID_PARAM);
    *numResources = 0;
    while (pointer) {
        *numResources = *numResources + 1;
        pointer = pointer->next;
    }
    return OC_STACK_OK;
}

/**
 * Get a resource handle by index.
 *
 * @param index - index of resource, 0 to Count - 1
 *
 * @return
 *    Resource handle - if found
 *    NULL - if not found
 */
OCResourceHandle OCGetResourceHandle(uint8_t index) {
    OCResource *pointer = headResource;
    uint8_t i = 0;

    OC_LOG(INFO, TAG, PCF("Entering OCGetResourceHandle"));

    // Iterate through the list
    while ((i < index) && pointer) {
        i++;
        pointer = pointer->next;
    }
    return (OCResourceHandle) pointer;
}

/**
 * Delete resource specified by handle.  Deletes resource and all resourcetype and resourceinterface
 * linked lists.
 *
 * @param handle - handle of resource to be deleted
 *
 * @return
 *     OC_STACK_OK              - no errors
 *     OC_STACK_ERROR           - stack process error
 *     OC_STACK_NO_RESOURCE     - resource not found
 *     OC_STACK_INVALID_PARAM   - invalid param
 */
OCStackResult OCDeleteResource(OCResourceHandle handle) {
    OC_LOG(INFO, TAG, PCF("Entering OCDeleteResource"));

    if (!handle) {
        OC_LOG(ERROR, TAG, PCF("Invalid param"));
        return OC_STACK_INVALID_PARAM;
    }

    OCResource *resource = findResource((OCResource *) handle);
    if (resource == NULL) {
        OC_LOG(ERROR, TAG, PCF("Resource not found"));
        return OC_STACK_NO_RESOURCE;
    }

    if (deleteResource((OCResource *) handle) == 0) {
        OC_LOG(ERROR, TAG, PCF("Error deleting resource"));
        return OC_STACK_ERROR;
    }

    return OC_STACK_OK;
}

/**
 * Get the URI of the resource specified by handle.
 *
 * @param handle - handle of resource
 * @return
 *    URI string - if resource found
 *    NULL - resource not found
 */
const char *OCGetResourceUri(OCResourceHandle handle) {
    OCResource *resource;
    OC_LOG(INFO, TAG, PCF("Entering OCGetResourceUri"));

    resource = findResource((OCResource *) handle);
    if (resource) {
        return resource->uri;
    }
    return (const char *) NULL;
}

/**
 * Get the properties of the resource specified by handle.
 * NOTE: that after a resource is created, the OC_ACTIVE property is set
 * for the resource by the stack.
 *
 * @param handle - handle of resource
 * @return
 *    property bitmap - if resource found
 *    NULL - resource not found
 */
uint8_t OCGetResourceProperties(OCResourceHandle handle) {
    OCResource *resource;
    OC_LOG(INFO, TAG, PCF("Entering OCGetResourceProperties"));

    resource = findResource((OCResource *) handle);
    if (resource) {
        return resource->resourceProperties;
    }
    return 0;
}

/**
 * Get the number of resource types of the resource.
 *
 * @param handle - handle of resource
 * @param numResourceTypes - pointer to count variable
 *
 * @return
 *     OC_STACK_OK    - no errors
 *     OC_STACK_ERROR - stack process error
 */
OCStackResult OCGetNumberOfResourceTypes(OCResourceHandle handle,
        uint8_t *numResourceTypes) {
    OCResource *resource;
    OCResourceType *pointer;

    OC_LOG(INFO, TAG, PCF("Entering OCGetNumberOfResourceTypes"));
    VERIFY_NON_NULL(numResourceTypes, ERROR, OC_STACK_INVALID_PARAM);
    VERIFY_NON_NULL(handle, ERROR, OC_STACK_INVALID_PARAM);

    *numResourceTypes = 0;

    resource = findResource((OCResource *) handle);
    if (resource) {
        pointer = resource->rsrcType;
        while (pointer) {
            *numResourceTypes = *numResourceTypes + 1;
            pointer = pointer->next;
        }
    }
    return OC_STACK_OK;
}

/**
 * Get name of resource type of the resource.
 *
 * @param handle - handle of resource
 * @param index - index of resource, 0 to Count - 1
 *
 * @return
 *    resource type name - if resource found
 *    NULL - resource not found
 */
const char *OCGetResourceTypeName(OCResourceHandle handle, uint8_t index) {
    OCResourceType *resourceType;

    OC_LOG(INFO, TAG, PCF("Entering OCGetResourceTypeName"));

    resourceType = findResourceTypeAtIndex(handle, index);
    if (resourceType) {
        return resourceType->resourcetypename;
    }
    return (const char *) NULL;
}



/**
 * Get the number of resource interfaces of the resource.
 *
 * @param handle - handle of resource
 * @param numResources - pointer to count variable
 *
 * @return
 *     OC_STACK_OK    - no errors
 *     OC_STACK_ERROR - stack process error
 */
OCStackResult OCGetNumberOfResourceInterfaces(OCResourceHandle handle,
        uint8_t *numResourceInterfaces) {
    OCResourceInterface *pointer;
    OCResource *resource;

    OC_LOG(INFO, TAG, PCF("Entering OCGetNumberOfResourceInterfaces"));

    VERIFY_NON_NULL(handle, ERROR, OC_STACK_INVALID_PARAM);
    VERIFY_NON_NULL(numResourceInterfaces, ERROR, OC_STACK_INVALID_PARAM);

    *numResourceInterfaces = 0;
    resource = findResource((OCResource *) handle);
    if (resource) {
        pointer = resource->rsrcInterface;
        while (pointer) {
            *numResourceInterfaces = *numResourceInterfaces + 1;
            pointer = pointer->next;
        }
    }
    return OC_STACK_OK;
}

/**
 * Get name of resource interface of the resource.
 *
 * @param handle - handle of resource
 * @param index - index of resource, 0 to Count - 1
 *
 * @return
 *    resource interface name - if resource found
 *    NULL - resource not found
 */
const char *OCGetResourceInterfaceName(OCResourceHandle handle, uint8_t index) {
    OCResourceInterface *resourceInterface;

    OC_LOG(INFO, TAG, PCF("Entering OCGetResourceInterfaceName"));

    resourceInterface = findResourceInterfaceAtIndex(handle, index);
    if (resourceInterface) {
        return resourceInterface->name;
    }
    return (const char *) NULL;
}

/**
 * Get resource handle from the collection resource by index.
 *
 * @param collectionHandle - handle of collection resource
 * @param index - index of contained resource, 0 to Count - 1
 *
 * @return
 *    handle to resource - if resource found
 *    NULL - resource not found
 */
OCResourceHandle OCGetResourceHandleFromCollection(OCResourceHandle collectionHandle,
        uint8_t index) {
    OCResource *resource;

    OC_LOG(INFO, TAG, PCF("Entering OCGetContainedResource"));

    if (index >= MAX_CONTAINED_RESOURCES) {
        return NULL;
    }

    resource = findResource((OCResource *) collectionHandle);
    if (!resource) {
        return NULL;
    }

    return resource->rsrcResources[index];
}

/**
 * Bind an entity handler to the resource.
 *
 * @param handle - handle to the resource that the contained resource is to be bound
 * @param entityHandler - entity handler function that is called by ocstack to handle requests, etc
 * @return
 *     OC_STACK_OK    - no errors
 *     OC_STACK_ERROR - stack process error
 */
OCStackResult OCBindResourceHandler(OCResourceHandle handle,
        OCEntityHandler entityHandler) {
    OCResource *resource;

    OC_LOG(INFO, TAG, PCF("Entering OCBindResourceHandler"));

    // Validate parameters
    VERIFY_NON_NULL(handle, ERROR, OC_STACK_INVALID_PARAM);
    //VERIFY_NON_NULL(entityHandler, ERROR, OC_STACK_INVALID_PARAM);

    // Use the handle to find the resource in the resource linked list
    resource = findResource((OCResource *)handle);
    if (!resource) {
        OC_LOG(ERROR, TAG, PCF("Resource not found"));
        return OC_STACK_ERROR;
    }

    // Bind the handler
    resource->entityHandler = entityHandler;

    #ifdef WITH_PRESENCE
    if(presenceResource.handle)
    {
        ((OCResource *)presenceResource.handle)->sequenceNum = OCGetRandom();
        SendPresenceNotification(resource->rsrcType);
    }
    #endif

    return OC_STACK_OK;
}

/**
 * Get the entity handler for a resource.
 *
 * @param handle - handle of resource
 *
 * @return
 *    entity handler - if resource found
 *    NULL - resource not found
 */
OCEntityHandler OCGetResourceHandler(OCResourceHandle handle) {
    OCResource *resource;

    OC_LOG(INFO, TAG, PCF("Entering OCGetResourceHandler"));

    // Use the handle to find the resource in the resource linked list
    resource = findResource((OCResource *)handle);
    if (!resource) {
        OC_LOG(ERROR, TAG, PCF("Resource not found"));
        return NULL;
    }

    // Bind the handler
    return resource->entityHandler;
}

void incrementSequenceNumber(OCResource * resPtr)
{
    // Increment the sequence number
    resPtr->sequenceNum += 1;
    if (resPtr->sequenceNum == MAX_SEQUENCE_NUMBER)
    {
        resPtr->sequenceNum = OC_OFFSET_SEQUENCE_NUMBER+1;
    }
    return;
}

#ifdef WITH_PRESENCE
/**
 * Notify Presence subscribers that a resource has been modified
 *
 * @param resourceType - Handle to the resourceType linked list of resource
 *                       that was modified.
 * @param qos          - Quality Of Service
 *
 */
OCStackResult SendPresenceNotification(OCResourceType *resourceType)
{
    OCResource *resPtr = NULL;
    OCStackResult result;
    OCMethod method = OC_REST_PRESENCE;
    uint32_t maxAge = 0;
    resPtr = findResource((OCResource *) presenceResource.handle);
    if(NULL == resPtr)
    {
        return OC_STACK_NO_RESOURCE;
    }
    if((((OCResource *) presenceResource.handle)->resourceProperties) & OC_ACTIVE)
    {
        maxAge = presenceResource.presenceTTL;
    }
    else
    {
        maxAge = 0;
    }

    result = SendAllObserverNotification(method, resPtr, maxAge, resourceType, OC_LOW_QOS);
    return result;
}
#endif

/**
 * Notify observers that an observed value has changed.
 *
 * @param handle - handle of resource
 *
 * @return
 *     OC_STACK_OK    - no errors
 *     OC_STACK_NO_RESOURCE - invalid resource handle
 *     OC_STACK_NO_OBSERVERS - no more observers intrested in resource
 */
OCStackResult OCNotifyAllObservers(OCResourceHandle handle, OCQualityOfService qos) {

    OC_LOG(INFO, TAG, PCF("Entering OCNotifyAllObservers"));

    OCResource *resPtr = NULL;
    OCStackResult result;
    OCMethod method = OC_REST_NOMETHOD;
    uint32_t maxAge = 0;

    OC_LOG(INFO, TAG, PCF("Entering OCNotifyAllObservers"));
    #ifdef WITH_PRESENCE
    if(handle == presenceResource.handle)
    {
        return OC_STACK_OK;
    }
    #endif // WITH_PRESENCE
    VERIFY_NON_NULL(handle, ERROR, OC_STACK_ERROR);

    // Verify that the resource exists
    resPtr = findResource ((OCResource *) handle);
    if (NULL == resPtr)
    {
        return OC_STACK_NO_RESOURCE;
    }
    else
    {
        //only increment in the case of regular observing (not presence)
        incrementSequenceNumber(resPtr);
        method = OC_REST_OBSERVE;
        maxAge = MAX_OBSERVE_AGE;
        #ifdef WITH_PRESENCE
        result = SendAllObserverNotification (method, resPtr, maxAge, NULL, qos);
        #else
        result = SendAllObserverNotification (method, resPtr, maxAge, qos);
        #endif
        return result;
    }
}

OCStackResult
OCNotifyListOfObservers (OCResourceHandle handle,
                         OCObservationId  *obsIdList,
                         uint8_t          numberOfIds,
                         unsigned char    *notificationJSONPayload,
                         OCQualityOfService qos)
{
    OC_LOG(INFO, TAG, PCF("Entering OCNotifyListOfObservers"));

    OCResource *resPtr = NULL;
    //TODO: we should allow the server to define this
    uint32_t maxAge = MAX_OBSERVE_AGE;

    VERIFY_NON_NULL(handle, ERROR, OC_STACK_ERROR);
    VERIFY_NON_NULL(obsIdList, ERROR, OC_STACK_ERROR);
    VERIFY_NON_NULL(notificationJSONPayload, ERROR, OC_STACK_ERROR);

    // Verify that the resource exists
    resPtr = findResource ((OCResource *) handle);
    if (NULL == resPtr || myStackMode == OC_CLIENT)
    {
        return OC_STACK_NO_RESOURCE;
    }
    else
    {
        incrementSequenceNumber(resPtr);
    }
    return (SendListObserverNotification(resPtr, obsIdList, numberOfIds,
            notificationJSONPayload, maxAge, qos));
}

/**
 * Send a response to a request.
 * The response can be a regular, slow, or block (i.e. a response that
 * is too large to be sent in a single PDU and must span multiple transmissions)
 *
 * @param response - pointer to structure that contains response parameters
 *
 * @return
 *     OC_STACK_OK                         - No errors; Success
 *     OC_STACK_INVALID_PARAM              - Invalid pointer to OCServerResponse
 *     OC_STACK_INVALID_REQUEST_HANDLE     - Request handle not found
 *     OC_STACK_PERSISTENT_BUFFER_REQUIRED - Block transfer needed for response, so a
 *                                           persistent response buffer is necessary
 */
OCStackResult OCDoResponse(OCEntityHandlerResponse *ehResponse)
{
    OCStackResult result = OC_STACK_ERROR;
    OCServerRequest *serverRequest = NULL;

    OC_LOG(INFO, TAG, PCF("Entering OCDoResponse"));

    // Validate input parameters
    VERIFY_NON_NULL(ehResponse, ERROR, OC_STACK_INVALID_PARAM);
    VERIFY_NON_NULL(ehResponse->requestHandle, ERROR, OC_STACK_INVALID_PARAM);

    // TODO: Placeholder for creating a response entry when implementing
    // block transfer feature

    // If a response payload is present, check if block transfer is required
    if (ehResponse->payload && OCIsPacketTransferRequired(NULL,
            (const char *)ehResponse->payload, ehResponse->payloadSize))
    {
        OC_LOG(INFO, TAG, PCF("Block transfer required"));

        // Persistent response buffer is needed for block transfer
        if (!ehResponse->persistentBufferFlag)
        {
            OC_LOG(WARNING, TAG, PCF("Persistent response buffer required"));
            return OC_STACK_PERSISTENT_BUFFER_REQUIRED;
        }
        // TODO: Placeholder for block transfer handling
        // TODO: Placeholder for setting the the response handle in the OCServerResponse struct
            // when implementing the block transfer feature
    }
    else
    {
        // Normal response

        // Get pointer to request info
        serverRequest = GetServerRequestUsingHandle((OCServerRequest *)ehResponse->requestHandle);
        if(serverRequest)
        {
            result = serverRequest->ehResponseHandler(ehResponse);
        }
    }
    return result;
}

/**
 * Cancel a response.  Applies to a block response
 *
 * @param responseHandle - response handle set by stack in OCServerResponse after
 *                         OCDoResponse is called
 *
 * @return
 *     OC_STACK_OK               - No errors; Success
 *     OC_STACK_INVALID_PARAM    - The handle provided is invalid.
 */
OCStackResult OCCancelResponse(OCResponseHandle responseHandle)
{
    OCStackResult result = OC_STACK_NOTIMPL;

    OC_LOG(INFO, TAG, PCF("Entering OCCancelResponse"));

    // TODO: validate response handle

    return result;
}

//-----------------------------------------------------------------------------
// Private internal function definitions
//-----------------------------------------------------------------------------
/**
 * Generate handle of OCDoResource invocation for callback management.
 */
static OCDoHandle GenerateInvocationHandle()
{
    OCDoHandle handle = NULL;
    // Generate token here, it will be deleted when the transaction is deleted
    handle = (OCDoHandle) OCMalloc(sizeof(uint8_t[MAX_TOKEN_LENGTH]));
    if (handle)
    {
        OCFillRandomMem((uint8_t*)handle, sizeof(uint8_t[MAX_TOKEN_LENGTH]));
    }

    return handle;
}
#ifdef WITH_PRESENCE
OCStackResult OCChangeResourceProperty(OCResourceProperty * inputProperty,
        OCResourceProperty resourceProperties, uint8_t enable)
{
    if (resourceProperties
            > (OC_ACTIVE | OC_DISCOVERABLE | OC_OBSERVABLE | OC_SLOW)) {
        OC_LOG(ERROR, TAG, PCF("Invalid property"));
        return OC_STACK_INVALID_PARAM;
    }
    if(!enable)
    {
        *inputProperty = (OCResourceProperty) (*inputProperty & ~(resourceProperties));
    }
    else
    {
        *inputProperty = (OCResourceProperty) (*inputProperty | resourceProperties);
    }
    return OC_STACK_OK;
}
#endif

/**
 * Initialize resource data structures, variables, etc.
 */
OCStackResult initResources() {
    OCStackResult result = OC_STACK_OK;
    // Init application resource vars
    headResource = NULL;
    // Init Virtual Resources
    #ifdef WITH_PRESENCE
    presenceResource.presenceTTL = OC_DEFAULT_PRESENCE_TTL;
    //presenceResource.token = OCGenerateCoAPToken();
    result = OCCreateResource(&presenceResource.handle,
            OC_RSRVD_RESOURCE_TYPE_PRESENCE,
            "core.r",
            OC_PRESENCE_URI,
            NULL,
            OC_OBSERVABLE);
    //make resource inactive
    result = OCChangeResourceProperty(
            &(((OCResource *) presenceResource.handle)->resourceProperties),
            OC_ACTIVE, 0);
    #endif
    return result;
}

/**
 * Add a resource to the end of the linked list of resources.
 *
 * @param resource - resource to be added
 */
void insertResource(OCResource *resource) {
    OCResource *pointer;

    if (!headResource) {
        headResource = resource;
    } else {
        pointer = headResource;

        while (pointer->next) {
            pointer = pointer->next;
        }
        pointer->next = resource;
    }
    resource->next = NULL;
}

/**
 * Find a resource in the linked list of resources.
 *
 * @param resource - resource to be found
 * @return
 *     NULL                - resource not found
 *     pointer to resource - pointer to resource that was found in the linked list
 */
OCResource *findResource(OCResource *resource) {
    OCResource *pointer = headResource;

    while (pointer) {
        if (pointer == resource) {
            return resource;
        }
        pointer = pointer->next;
    }
    return NULL;
}

void deleteAllResources()
{
    OCResource *pointer = headResource;
    OCResource *temp;

    while (pointer)
    {
        temp = pointer->next;
        #ifdef WITH_PRESENCE
        if(pointer != (OCResource *) presenceResource.handle)
        {
            #endif // WITH_PRESENCE
            deleteResource(pointer);
            #ifdef WITH_PRESENCE
        }
        #endif // WITH_PRESENCE
        pointer = temp;
    }

    #ifdef WITH_PRESENCE
    // Ensure that the last resource to be deleted is the presence resource. This allows for all
    // presence notification attributed to their deletion to be processed.
    deleteResource((OCResource *) presenceResource.handle);
    #endif // WITH_PRESENCE
}

/**
 * Delete the resource from the linked list.
 *
 * @param resource - resource to be deleted
 * @return
 *    0 - error
 *    1 - success
 */
int deleteResource(OCResource *resource) {
    OCResource *prev = NULL;
    OCResource *temp;

    temp = headResource;
    while (temp) {
        if (temp == resource) {
            // Invalidate all Resource Properties.
            resource->resourceProperties = (OCResourceProperty) 0;
            #ifdef WITH_PRESENCE
            if(resource != (OCResource *) presenceResource.handle)
            {
            #endif // WITH_PRESENCE
                OCNotifyAllObservers((OCResourceHandle)resource, OC_HIGH_QOS);
            #ifdef WITH_PRESENCE
            }

            if(presenceResource.handle)
            {
                ((OCResource *)presenceResource.handle)->sequenceNum = OCGetRandom();
                if(resource != (OCResource *) presenceResource.handle)
                {
                    SendPresenceNotification(resource->rsrcType);
                }
                else
                {
                    SendPresenceNotification(NULL);
                }
            }
        #endif

            if (temp == headResource) {
                headResource = temp->next;
            } else {
                prev->next = temp->next;
            }

            deleteResourceElements(temp);
            OCFree(temp);
            return 1;
        } else {
            prev = temp;
            temp = temp->next;
        }
    }

    return 0;
}

/**
 * Delete all of the dynamically allocated elements that were created for the resource.
 *
 * @param resource - specified resource
 */
void deleteResourceElements(OCResource *resource) {
    if (!resource) {
        return;
    }

    // remove URI
    OCFree(resource->uri);

    // Delete resourcetype linked list
    deleteResourceType(resource->rsrcType);

    // Delete resourceinterface linked list
    deleteResourceInterface(resource->rsrcInterface);
}

/**
 * Delete all of the dynamically allocated elements that were created for the resource type.
 *
 * @param resourceType - specified resource type
 */
void deleteResourceType(OCResourceType *resourceType) {
    OCResourceType *pointer = resourceType;
    OCResourceType *next;

    while (pointer) {
        next = pointer->next;
        OCFree(pointer->resourcetypename);
        OCFree(pointer);
        pointer = next;
    }
}

/**
 * Delete all of the dynamically allocated elements that were created for the resource interface.
 *
 * @param resourceInterface - specified resource interface
 */
void deleteResourceInterface(OCResourceInterface *resourceInterface) {
    OCResourceInterface *pointer = resourceInterface;
    OCResourceInterface *next;

    while (pointer) {
        next = pointer->next;
        OCFree(pointer->name);
        OCFree(pointer);
        pointer = next;
    }
}

/**
 * Insert a resource type into a resource's resource type linked list.
 *
 * @param resource - resource where resource type is to be inserted
 * @param resourceType - resource type to be inserted
 */
void insertResourceType(OCResource *resource, OCResourceType *resourceType) {
    OCResourceType *pointer;

    if (resource && !resource->rsrcType) {
        resource->rsrcType = resourceType;
    } else {
        if(resource)
        {
            pointer = resource->rsrcType;
        }
        else
        {
            pointer = resourceType;
        }
        while (pointer->next) {
            pointer = pointer->next;
        }
        pointer->next = resourceType;
    }
    resourceType->next = NULL;
}

/**
 * Get a resource type at the specified index within a resource.
 *
 * @param handle - handle of resource
 * @param index - index of resource type
 *
 * @return
 *    resourcetype - if found
 *    NULL - not found
 */
OCResourceType *findResourceTypeAtIndex(OCResourceHandle handle, uint8_t index) {
    OCResource *resource;
    OCResourceType *pointer;
    uint8_t i;

    // Find the specified resource
    resource = findResource((OCResource *) handle);
    if (!resource) {
        return NULL;
    }

    // Make sure a resource has a resourcetype
    if (!resource->rsrcType) {
        return NULL;
    }

    // Iterate through the list
    pointer = resource->rsrcType;
    i = 0;
    while ((i < index) && pointer) {
        i++;
        pointer = pointer->next;
    }
    return pointer;
}

/**
 * Finds a resource type in an OCResourceType link-list.
 *
 * @param resourceTypeList - the link-list to be searched through
 * @param resourceTypeName - the key to search for
 *
 * @return
 *      resourceType that matches the key (ie. resourceTypeName)
 *      NULL - either an invalid parameter or this function was unable to find the key.
 */
OCResourceType *findResourceType(OCResourceType * resourceTypeList, const char * resourceTypeName)
{
    if(resourceTypeList && resourceTypeName)
    {
        OCResourceType * rtPointer = resourceTypeList;
        while(resourceTypeName && rtPointer)
        {
            if(rtPointer->resourcetypename &&
                    strcmp(resourceTypeName, (const char *)
                    (rtPointer->resourcetypename)) == 0)
            {
                break;
            }
            rtPointer = rtPointer->next;
        }
        return rtPointer;
    }
    return NULL;
}
/**
 * Insert a resource interface into a resource's resource interface linked list.
 *
 * @param resource - resource where resource interface is to be inserted
 * @param resourceInterface - resource interface to be inserted
 */
void insertResourceInterface(OCResource *resource,
        OCResourceInterface *resourceInterface) {
    OCResourceInterface *pointer;

    if (!resource->rsrcInterface) {
        resource->rsrcInterface = resourceInterface;
    } else {
        pointer = resource->rsrcInterface;
        while (pointer->next) {
            pointer = pointer->next;
        }
        pointer->next = resourceInterface;
    }
    resourceInterface->next = NULL;
}

/**
 * Get a resource interface at the specified index within a resource.
 *
 * @param handle - handle of resource
 * @param index - index of resource interface
 *
 * @return
 *    resourceinterface - if found
 *    NULL - not found
 */
OCResourceInterface *findResourceInterfaceAtIndex(OCResourceHandle handle,
        uint8_t index) {
    OCResource *resource;
    OCResourceInterface *pointer;
    uint8_t i = 0;

    // Find the specified resource
    resource = findResource((OCResource *) handle);
    if (!resource) {
        return NULL;
    }

    // Make sure a resource has a resourceinterface
    if (!resource->rsrcInterface) {
        return NULL;
    }

    // Iterate through the list
    pointer = resource->rsrcInterface;

    while ((i < index) && pointer) {
        i++;
        pointer = pointer->next;
    }
    return pointer;
}

/**
 * Determine if a request/response must be sent in a block transfer because it is too large to be
 * sent in a single PDU.  This function can be used for either a request or a response
 *
 * @param request  - NULL or pointer to request
 * @param response - NULL or pointer to response
 * @param size     - 0 or size of the request/response.  If 0, strlen is used for determining
 *                   the length of the request/response
 *
 * @return
 *    0 - packet transfer NOT required (i.e. normal request/response)
 *    1 - packet transfer required (i.e. block transfer needed)
 */
uint8_t OCIsPacketTransferRequired(const char *request, const char *response, uint16_t size)
{
    uint8_t result = 0;

    // Determine if we are checking a request or a response
    if (request)
    {
        // If size is greater than 0, use it for the request size value, otherwise
        // assume request is null terminated and use strlen for size value
        if ((size > MAX_REQUEST_LENGTH) || (strlen(request) > MAX_REQUEST_LENGTH))
        {
            result = 1;
        }
    }
    else if (response)
    {
        // If size is greater than 0, use it for the response size value, otherwise
        // assume response is null terminated and use strlen for size value
        if ((size > MAX_RESPONSE_LENGTH) || (strlen(response) > MAX_RESPONSE_LENGTH))
        {
            result = 1;
        }
    }
    return result;
}

/**
 * Retrieves a resource type based upon a uri string if the uri string contains only just one
 * resource attribute (and that has to be of type "rt").
 *
 * @remark This API malloc's memory for the resource type and newURI. Do not malloc resourceType
 * or newURI before passing in.
 *
 * @param uri - Valid URI for "requiredUri" parameter to OCDoResource API.
 * @param resourceType - The resource type to be populated; pass by reference.
 * @param newURI - Return URI without resourceType appended to the end of it. This is used to
 *                 ensure that the uri parameter is not modified; pass by reference.
 *
 * @return
 *  OC_STACK_INVALID_URI   - Returns this if the URI is invalid/NULL.
 *  OC_STACK_INVALID_PARAM - Returns this if the resourceType parameter is invalid/NULL.
 *  OC_STACK_OK            - Success
 */
OCStackResult getResourceType(const char * uri, unsigned char** resourceType, char ** newURI)
{
    if(!uri)
    {
        return OC_STACK_INVALID_URI;
    }
    if(!resourceType || !newURI)
    {
        return OC_STACK_INVALID_PARAM;
    }
    char * leftToken = NULL;
    char * tempURI = (char *) OCMalloc(strlen(uri) + 1);
    if(!tempURI)
    {
        goto exit;
    }
    strcpy(tempURI, uri);
    char* strTokPtr;
    leftToken = strtok_r((char *)tempURI, "?", &strTokPtr);

    while(leftToken != NULL)
    {
        if(strncmp(leftToken, "rt=", 3) == 0)
        {
            *resourceType = (unsigned char *) OCMalloc(strlen(leftToken)-3);
            if(!*resourceType)
            {
                goto exit;
            }
            strcpy((char *)*resourceType, ((const char *)&leftToken[3]));
            break;
        }
        leftToken = strtok_r(NULL, "?", &strTokPtr);
    }

    *newURI = tempURI;

    return OC_STACK_OK;

    exit:
        return OC_STACK_NO_MEMORY;
}
