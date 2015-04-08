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

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "cainterface.h"
#include "caremotehandler.h"
#include "camessagehandler.h"
#include "canetworkconfigurator.h"
#include "cainterfacecontroller.h"
#include "logger.h"

#define TAG PCF("CA")

static void CAMessageHandler(int32_t id, CADetachErrorCode code)
{
}

CAResult_t CAInitialize()
{
    OIC_LOG_V(DEBUG, TAG, "CAInitialize");

    CASetMessageHandlerCallback(CAMessageHandler);

    CAResult_t res = CAInitializeMessageHandler();

    if (res != CA_STATUS_OK)
    {
        return res;
    }

    return CA_STATUS_OK;
}

void CATerminate()
{
    OIC_LOG_V(DEBUG, TAG, "CATerminate");

    CATerminateMessageHandler();
}

CAResult_t CAStartListeningServer()
{
    OIC_LOG_V(DEBUG, TAG, "CAStartListeningServer");

    return CAStartListeningServerAdapters();
}

CAResult_t CAStartDiscoveryServer()
{
    OIC_LOG_V(DEBUG, TAG, "CAStartDiscoveryServer");

    return CAStartDiscoveryServerAdapters();
}

CAResult_t CARegisterHandler(CARequestCallback ReqHandler, CAResponseCallback RespHandler)
{
    OIC_LOG_V(DEBUG, TAG, "CARegisterHandler");

    CASetRequestResponseCallbacks(ReqHandler, RespHandler);

    return CA_STATUS_OK;
}

CAResult_t CACreateRemoteEndpoint(const CAURI_t uri, CARemoteEndpoint_t** remoteEndpoint)
{
    OIC_LOG_V(DEBUG, TAG, "CACreateRemoteEndpoint");

    CARemoteEndpoint_t* remote = CACreateRemoteEndpointUriInternal(uri);

    *remoteEndpoint = remote;

    if (remote == NULL)
        return CA_STATUS_FAILED;

    return CA_STATUS_OK;
}

void CADestroyRemoteEndpoint(CARemoteEndpoint_t* rep)
{
    OIC_LOG_V(DEBUG, TAG, "CADestroyRemoteEndpoint");

    CADestroyRemoteEndpointInternal(rep);
}

CAResult_t CAGenerateToken(CAToken_t* token)
{
    OIC_LOG_V(DEBUG, TAG, "CAGenerateToken");

    return CAGenerateTokenInternal(token);
}

void CADestroyToken(CAToken_t token)
{
    OIC_LOG_V(DEBUG, TAG, "CADestroyToken");

    CADestroyTokenInternal(token);
}

CAResult_t CAGetNetworkInformation(CALocalConnectivityt_t **info, uint32_t* size)
{
    OIC_LOG_V(DEBUG, TAG, "CAGetNetworkInformation");

    return CA_NOT_SUPPORTED;
}

CAResult_t CAFindResource(const CAURI_t resourceUri)
{
    OIC_LOG_V(DEBUG, TAG, "CAFindResource");

    int32_t actionId = -1;

    actionId = CADetachMessageResourceUri(resourceUri);

    if (actionId == -1)
        return CA_SEND_FAILED;

    OIC_LOG_V(DEBUG, TAG, "action id : %d", actionId);

    return CA_STATUS_OK;
}

CAResult_t CASendRequest(const CARemoteEndpoint_t* object, CARequestInfo_t* requestInfo)
{
    OIC_LOG_V(DEBUG, TAG, "CASendGetRequest");

    int32_t actionId = -1;

    actionId = CADetachRequestMessage(object, requestInfo);

    if (actionId == -1)
        return CA_SEND_FAILED;

    OIC_LOG_V(DEBUG, TAG, "action id : %d", actionId);

    return CA_STATUS_OK;
}

CAResult_t CASendNotification(const CARemoteEndpoint_t* object, CAResponseInfo_t* responseInfo)
{
    OIC_LOG_V(DEBUG, TAG, "CASendNotification");

    return CA_NOT_SUPPORTED;
}

CAResult_t CASendResponse(const CARemoteEndpoint_t* object, CAResponseInfo_t* responseInfo)
{
    OIC_LOG_V(DEBUG, TAG, "CASendResponse");

    int32_t actionId = -1;

    actionId = CADetachResponseMessage(object, responseInfo);

    if (actionId == -1)
        return CA_SEND_FAILED;

    OIC_LOG_V(DEBUG, TAG, "action id : %d", actionId);

    return CA_STATUS_OK;
}

CAResult_t CAAdvertiseResource(const CAURI_t uri, CAHeaderOption_t* options, uint8_t numOptions)
{
    OIC_LOG_V(DEBUG, TAG, "CAAdvertiseResource");

    return CA_NOT_SUPPORTED;
}

CAResult_t CASelectNetwork(const uint32_t interestedNetwork)
{
    OIC_LOG_V(DEBUG, TAG, "Selected network : %d", interestedNetwork);

    if (!(interestedNetwork & 0xf))
    {
        return CA_NOT_SUPPORTED;
    }
    CAResult_t res;

    if (interestedNetwork & CA_ETHERNET)
    {
        res = CAAddNetworkType(CA_ETHERNET);
        if (res != CA_STATUS_OK)
        {
            return res;
        }
    }

    if (interestedNetwork & CA_WIFI)
    {
        res = CAAddNetworkType(CA_WIFI);
        if (res != CA_STATUS_OK)
        {
            return res;
        }
    }

    if (interestedNetwork & CA_EDR)
    {
        res = CAAddNetworkType(CA_EDR);
        if (res != CA_STATUS_OK)
        {
            return res;
        }
    }

    if (interestedNetwork & CA_LE)
    {
        res = CAAddNetworkType(CA_LE);
        if (res != CA_STATUS_OK)
        {
            return res;
        }
    }

    return CA_STATUS_OK;
}

CAResult_t CAUnSelectNetwork(const uint32_t nonInterestedNetwork)
{
    OIC_LOG_V(DEBUG, TAG, "unselected network : %d", nonInterestedNetwork);

    if (!(nonInterestedNetwork & 0xf))
    {
        return CA_NOT_SUPPORTED;
    }

    CAResult_t res;

    if (nonInterestedNetwork & CA_ETHERNET)
    {
        res = CARemoveNetworkType(CA_ETHERNET);
        if (res != CA_STATUS_OK)
        {
            return res;
        }
    }

    if (nonInterestedNetwork & CA_WIFI)
    {
        res = CARemoveNetworkType(CA_WIFI);
        if (res != CA_STATUS_OK)
        {
            return res;
        }
    }

    if (nonInterestedNetwork & CA_EDR)
    {
        res = CARemoveNetworkType(CA_EDR);
        if (res != CA_STATUS_OK)
        {
            return res;
        }
    }

    if (nonInterestedNetwork & CA_LE)
    {
        res = CARemoveNetworkType(CA_LE);
        if (res != CA_STATUS_OK)
        {
            return res;
        }
    }

    return CA_STATUS_OK;
}

CAResult_t CAHandleRequestResponse()
{
    OIC_LOG_V(DEBUG, TAG, "CAHandleRequestResponse");

    CAHandleRequestResponseCallbacks();

    return CA_STATUS_OK;
}

