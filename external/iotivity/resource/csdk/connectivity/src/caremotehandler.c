/******************************************************************
 *
 * Copyright 2014 Samsung Electronics All Rights Reserved.
 *
 *
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ******************************************************************/

#include "oic_malloc.h"
#include "caremotehandler.h"
#include "logger.h"

#define TAG PCF("CA")

CARemoteEndpoint_t* CACloneRemoteEndpoint(const CARemoteEndpoint_t* rep)
{
    char* temp = NULL;
    int len = 0;

    if (rep == NULL)
        return NULL;

    // allocate the remote end point structure.
    CARemoteEndpoint_t* clone = (CARemoteEndpoint_t*) OICMalloc(sizeof(CARemoteEndpoint_t));
    if (clone == NULL)
    {
        OIC_LOG_V(DEBUG, TAG, "memory alloc error!!");
        return NULL;
    }
    memset(clone, 0, sizeof(CARemoteEndpoint_t));
    memcpy(clone, rep, sizeof(CARemoteEndpoint_t));

    if (rep->resourceUri != NULL)
    {
        // allocate reference uri field
        len = strlen(rep->resourceUri);

        temp = (char*) OICMalloc(sizeof(char) * (len + 1));
        if (temp == NULL)
        {
            OIC_LOG_V(DEBUG, TAG, "memory alloc error!!");

            CADestroyRemoteEndpointInternal(clone);

            return NULL;
        }
        memset(temp, 0, sizeof(char) * (len + 1));
        strncpy(temp, rep->resourceUri, len);

        // save the uri
        clone->resourceUri = temp;
    }

    return clone;
}

#define COAP_PREFIX         "coap://"
#define COAP_PREFIX_LEN     7

// return 1 : ip
// return 0 : mac
static int32_t getCAAddress(const char* pAddress, CAAddress_t* outAddress)
{
    if (pAddress == NULL || outAddress == NULL)
        return -1;

    // simple parse, it will be change.
    // 10.11.12.13:4545 (ip)
    // 10:11:12:13:45:45 (mac)

    int32_t len = strlen(pAddress);

    int32_t isIp = 0;
    int32_t ipLen = 0;
    int32_t port = 0;

    int i = 0;
    for (i = 0; i < len; i++)
    {
        if (pAddress[i] == '.')
        {
            isIp = 1;
        }

        // found port number start index
        if (isIp && pAddress[i] == ':')
        {
            ipLen = i;
            break;
        }
    }

    if (isIp)
    {
        strncpy(outAddress->IP.ipAddress, pAddress, ipLen == 0 ? len : ipLen);

        if (ipLen > 0)
            outAddress->IP.port = atoi(pAddress + ipLen + 1);

        OIC_LOG_V(DEBUG, TAG, "ip: %s,port: %d", outAddress->IP.ipAddress, outAddress->IP.port);
    }
    else
    {
        strncpy(outAddress->BT.btMacAddress, pAddress, CA_MACADDR_SIZE - 1);

        OIC_LOG_V(DEBUG, TAG, "mac address : %s", outAddress->BT.btMacAddress);
    }

    return isIp;
}

CARemoteEndpoint_t* CACreateRemoteEndpointUriInternal(const CAURI_t uri)
{
    // support URI type
    // coap://10.11.12.13:4545/resource_uri
    // coap://10:11:12:13:45:45/resource_uri

    if (uri == NULL)
        return NULL;

    // parse uri
    // #1. check prefix
    int startIndex = 0;

    if (strncmp(COAP_PREFIX, uri, COAP_PREFIX_LEN) == 0)
    {
        OIC_LOG_V(DEBUG, TAG, "uri has '%s' prefix.", COAP_PREFIX);
        startIndex = COAP_PREFIX_LEN;
    }

    // #2. copy uri for parse
    char* cloneUri = NULL;
    int32_t len = strlen(uri) - startIndex;

    if (len <= 0)
    {
        OIC_LOG_V(DEBUG, TAG, "uri length is 0!");
        return NULL;
    }

    cloneUri = (char*) OICMalloc(sizeof(char) * (len + 1));
    if (cloneUri == NULL)
    {
        OIC_LOG_V(DEBUG, TAG, "memory error!!");
        return NULL;
    }
    memset(cloneUri, 0, sizeof(char) * (len + 1));
    memcpy(cloneUri, &uri[startIndex], sizeof(char) * (len + 1));

    // #3. parse address
    // #4. parse resource uri
    char* pAddress = cloneUri;
    char* pResourceUri = NULL;

    int32_t i = 0;
    for (i = 0; i < len; i++)
    {
        if (cloneUri[i] == '/')
        {
            // separate
            cloneUri[i] = 0;

            pResourceUri = &cloneUri[i + 1];

            break;
        }

    }

    OIC_LOG_V(DEBUG, TAG, "pAddress : %s", pAddress);

    OIC_LOG_V(DEBUG, TAG, "pResourceUri : %s", pResourceUri == NULL ? "" : pResourceUri);

    // address
    CAAddress_t address;
    memset(&address, 0, sizeof(CAAddress_t));

    int resType = getCAAddress(pAddress, &address);
    if (resType == -1)
    {
        OIC_LOG_V(DEBUG, TAG, "address parse error");

        OICFree(cloneUri);
        return NULL;
    }

    // resource uri
    CAURI_t resourceUri = pResourceUri;

    // connectivity type
    CAConnectivityType_t type;

    if (resType == 1)
    {
        type = CA_WIFI;
    }
    else
    {
        type = CA_EDR;
    }

    CARemoteEndpoint_t* remoteEndpoint = CACreateRemoteEndpointInternal(resourceUri, address, type);

    OICFree(cloneUri);

    return remoteEndpoint;
}

CARemoteEndpoint_t* CACreateRemoteEndpointInternal(const CAURI_t resourceUri,
        const CAAddress_t addr, const CAConnectivityType_t type)
{
    char* temp = NULL;
    int len = 0;

    if (resourceUri == NULL)
    {
        OIC_LOG_V(DEBUG, TAG, "uri is null value");
        return NULL;
    }

    // allocate the remote end point structure.
    CARemoteEndpoint_t* rep = (CARemoteEndpoint_t*) OICMalloc(sizeof(CARemoteEndpoint_t));

    if (rep == NULL)
    {
        OIC_LOG_V(DEBUG, TAG, "memory alloc error");
        return NULL;
    }
    memset(rep, 0, sizeof(CARemoteEndpoint_t));

    // allocate reference uri field
    len = strlen(resourceUri);

    temp = (char*) OICMalloc(sizeof(char) * (len + 1));
    if (temp == NULL)
    {
        OIC_LOG_V(DEBUG, TAG, "memory alloc error");

        CADestroyRemoteEndpointInternal(rep);

        return NULL;
    }
    memset(temp, 0, sizeof(char) * (len + 1));
    strncpy(temp, resourceUri, len);

    // save the uri
    rep->resourceUri = temp;

    // save the addressInfo
    memcpy(&(rep->addressInfo), &addr, sizeof(CAAddress_t));

    // save the type
    rep->connectivityType = type;

    return rep;
}

CARequestInfo_t* CACloneRequestInfo(const CARequestInfo_t* rep)
{
    char* temp = NULL;
    int len = 0;

    if (rep == NULL)
        return NULL;

    // allocate the request info structure.
    CARequestInfo_t* clone = (CARequestInfo_t*) OICMalloc(sizeof(CARequestInfo_t));
    if (clone == NULL)
    {
        OIC_LOG_V(DEBUG, TAG, "memory alloc error!!");
        return NULL;
    }
    memset(clone, 0, sizeof(CARequestInfo_t));
    memcpy(clone, rep, sizeof(CARequestInfo_t));

    if (rep->info.token != NULL)
    {
        // allocate token field
        len = strlen(rep->info.token);

        temp = (char*) OICMalloc(sizeof(char) * (len + 1));
        if (temp == NULL)
        {
            OIC_LOG_V(DEBUG, TAG, "memory alloc error!!");

            CADestroyRequestInfoInternal(clone);

            return NULL;
        }
        memset(temp, 0, sizeof(char) * (len + 1));
        strncpy(temp, rep->info.token, len);

        // save the token
        clone->info.token = temp;
    }

    if (rep->info.options != NULL)
    {
        // save the options
        clone->info.options = (CAHeaderOption_t*) OICMalloc(sizeof(CAHeaderOption_t));
        memset(clone->info.options, 0, sizeof(CAHeaderOption_t));
        memcpy(clone->info.options, rep->info.options, sizeof(CAHeaderOption_t));
    }

    if (rep->info.payload != NULL)
    {
        // allocate payload field
        len = strlen(rep->info.payload);

        temp = (char*) OICMalloc(sizeof(char) * (len + 1));
        if (temp == NULL)
        {
            OIC_LOG_V(DEBUG, TAG, "memory alloc error!!");

            CADestroyRequestInfoInternal(clone);

            return NULL;
        }
        memset(temp, 0, sizeof(char) * (len + 1));
        strncpy(temp, rep->info.payload, len);

        // save the payload
        clone->info.payload = temp;
    }

    return clone;
}

CAResponseInfo_t* CACloneResponseInfo(const CAResponseInfo_t* rep)
{
    char* temp = NULL;
    int len = 0;

    if (rep == NULL)
        return NULL;

    // allocate the response info structure.
    CAResponseInfo_t* clone = (CAResponseInfo_t*) OICMalloc(sizeof(CAResponseInfo_t));
    if (clone == NULL)
    {
        OIC_LOG_V(DEBUG, TAG, "memory alloc error!!");
        return NULL;
    }
    memset(clone, 0, sizeof(CAResponseInfo_t));
    memcpy(clone, rep, sizeof(CAResponseInfo_t));

    if (rep->info.token != NULL)
    {
        // allocate token field
        len = strlen(rep->info.token);

        temp = (char*) OICMalloc(sizeof(char) * (len + 1));
        if (temp == NULL)
        {
            OIC_LOG_V(DEBUG, TAG, "memory alloc error!!");

            CADestroyResponseInfoInternal(clone);

            return NULL;
        }
        memset(temp, 0, sizeof(char) * (len + 1));
        strncpy(temp, rep->info.token, len);

        // save the token
        clone->info.token = temp;
    }

    if (rep->info.options != NULL)
    {
        // save the options
        clone->info.options = (CAHeaderOption_t*) OICMalloc(sizeof(CAHeaderOption_t));
        memset(clone->info.options, 0, sizeof(CAHeaderOption_t));
        memcpy(clone->info.options, rep->info.options, sizeof(CAHeaderOption_t));
    }

    if (rep->info.payload != NULL)
    {
        // allocate payload field
        len = strlen(rep->info.payload);

        temp = (char*) OICMalloc(sizeof(char) * (len + 1));
        if (temp == NULL)
        {
            OIC_LOG_V(DEBUG, TAG, "memory alloc error!!");

            CADestroyResponseInfoInternal(clone);

            return NULL;
        }
        memset(temp, 0, sizeof(char) * (len + 1));
        strncpy(temp, rep->info.payload, len);

        // save the payload
        clone->info.payload = temp;
    }

    return clone;
}

void CADestroyRemoteEndpointInternal(CARemoteEndpoint_t* rep)
{
    if (rep == NULL)
        return;

    // free uri field
    if (rep->resourceUri != NULL)
    {
        OICFree((char*) rep->resourceUri);
    }

    // free remote end point structure.
    OICFree(rep);
}

void CADestroyRequestInfoInternal(CARequestInfo_t* rep)
{
    if (rep == NULL)
        return;

    // free token field
    if (rep->info.token != NULL)
    {
        OICFree((char*) rep->info.token);
    }

    // free options field
    if (rep->info.options != NULL)
    {
        OICFree((CAHeaderOption_t*) rep->info.options);
    }

    // free payload field
    if (rep->info.payload != NULL)
    {
        OICFree((char*) rep->info.payload);
    }

    OICFree(rep);
}

void CADestroyResponseInfoInternal(CAResponseInfo_t* rep)
{
    if (rep == NULL)
        return;

    // free token field
    if (rep->info.token != NULL)
    {
        OICFree((char*) rep->info.token);
    }

    // free options field
    if (rep->info.options != NULL)
    {
        OICFree((CAHeaderOption_t*) rep->info.options);
    }

    // free payload field
    if (rep->info.payload != NULL)
    {
        OICFree((char*) rep->info.payload);
    }

    OICFree(rep);
}
