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
#include <string.h>

#include "caethernetadapter.h"
#include "logger.h"

#define TAG PCF("CA")

static CANetworkPacketReceivedCallback gEthernetReceivedCallback = NULL;

CAResult_t CAInitializeEthernet(CARegisterConnectivityCallback registerCallback,
        CANetworkPacketReceivedCallback reqRespCallback, CANetworkChangeCallback netCallback)
{
    OIC_LOG_V(DEBUG, TAG, "CAInitializeEthernet");

    gEthernetReceivedCallback = reqRespCallback;

    // register handlers
    CAConnectivityHandler_t handler;
    memset(&handler, 0, sizeof(CAConnectivityHandler_t));

    handler.startAdapter = CAStartEthernet;
    handler.startListenServer = CAStartEthernetListeningServer;
    handler.startDiscoverServer = CAStartEthernetDiscoveryServer;
    handler.sendData = CASendEthernetUnicastData;
    handler.sendDataToAll = CASendEthernetMulticastData;
    handler.startNotifyServer = CAStartEthernetNotifyServer;
    handler.sendNotification = CASendEthernetNotification;
    handler.GetnetInfo = CAGetEthernetInterfaceInformation;
    handler.readData = CAReadEthernetData;
    handler.stopAdapter = CAStopEthernet;
    handler.terminate = CATerminateEthernet;

    registerCallback(handler, CA_ETHERNET);

    return CA_STATUS_OK;
}

CAResult_t CAStartEthernet()
{
    OIC_LOG_V(DEBUG, TAG, "CAStartEthernet");

    return CA_STATUS_OK;
}

CAResult_t CAStartEthernetListeningServer()
{
    OIC_LOG_V(DEBUG, TAG, "CAStartEthernetListeningServer");

    return CA_STATUS_OK;
}

CAResult_t CAStartEthernetDiscoveryServer()
{
    OIC_LOG_V(DEBUG, TAG, "CAStartEthernetDiscoveryServer");

    return CA_STATUS_OK;
}

uint32_t CASendEthernetUnicastData(const CARemoteEndpoint_t* endpoint, void* data, uint32_t dataLen)
{
    OIC_LOG_V(DEBUG, TAG, "CASendEthernetUnicastData");

    return 0;
}

uint32_t CASendEthernetMulticastData(void* data, uint32_t dataLen)
{
    OIC_LOG_V(DEBUG, TAG, "CASendEthernetMulticastData");

    return 0;
}

CAResult_t CAStartEthernetNotifyServer()
{
    OIC_LOG_V(DEBUG, TAG, "CAStartEthernetNotifyServer");

    return CA_STATUS_OK;
}

uint32_t CASendEthernetNotification(const CARemoteEndpoint_t* endpoint, void* data,
        uint32_t dataLen)
{
    OIC_LOG_V(DEBUG, TAG, "CASendEthernetNotification");

    return 0;
}

CAResult_t CAGetEthernetInterfaceInformation(CALocalConnectivityt_t** info, uint32_t* size)
{
    OIC_LOG_V(DEBUG, TAG, "CAGetEthernetInterfaceInformation");

    return CA_STATUS_OK;
}

CAResult_t CAReadEthernetData()
{
    OIC_LOG_V(DEBUG, TAG, "Read Ethernet Data");

    return CA_STATUS_OK;
}

CAResult_t CAStopEthernet()
{
    OIC_LOG_V(DEBUG, TAG, "CAStopEthernet");

    return CA_STATUS_OK;
}

void CATerminateEthernet()
{
    OIC_LOG_V(DEBUG, TAG, "CATerminateEthernet");
}
