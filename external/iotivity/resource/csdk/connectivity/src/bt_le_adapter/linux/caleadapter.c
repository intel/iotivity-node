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

#include "caleadapter.h"
#include "logger.h"

#define TAG PCF("CA")

static CANetworkPacketReceivedCallback gLEReceivedCallback = NULL;

CAResult_t CAInitializeLE(CARegisterConnectivityCallback registerCallback,
        CANetworkPacketReceivedCallback reqRespCallback, CANetworkChangeCallback netCallback)
{
    OIC_LOG_V(DEBUG, TAG, "CAInitializeLE");

    gLEReceivedCallback = reqRespCallback;

    // register handlers
    CAConnectivityHandler_t handler;
    memset(&handler, 0, sizeof(CAConnectivityHandler_t));

    handler.startAdapter = CAStartLE;
    handler.startListenServer = CAStartLEListeningServer;
    handler.startDiscoverServer = CAStartLEDiscoveryServer;
    handler.sendData = CASendLEUnicastData;
    handler.sendDataToAll = CASendLEMulticastData;
    handler.startNotifyServer = CAStartLENotifyServer;
    handler.sendNotification = CASendLENotification;
    handler.GetnetInfo = CAGetLEInterfaceInformation;
    handler.readData = CAReadLEData;
    handler.stopAdapter = CAStopLE;
    handler.terminate = CATerminateLE;

    registerCallback(handler, CA_LE);

    return CA_STATUS_OK;
}

CAResult_t CAStartLE()
{
    OIC_LOG_V(DEBUG, TAG, "CAStartLE");

    return CA_STATUS_OK;
}

CAResult_t CAStartLEListeningServer()
{
    OIC_LOG_V(DEBUG, TAG, "CAStartLEListeningServer");

    return CA_STATUS_OK;
}

CAResult_t CAStartLEDiscoveryServer()
{
    OIC_LOG_V(DEBUG, TAG, "CAStartLEDiscoveryServer");

    return CA_STATUS_OK;
}

uint32_t CASendLEUnicastData(const CARemoteEndpoint_t* endpoint, void* data, uint32_t dataLen)
{
    OIC_LOG_V(DEBUG, TAG, "CASendLEUnicastData");

    return 0;
}

uint32_t CASendLEMulticastData(void* data, uint32_t dataLen)
{
    OIC_LOG_V(DEBUG, TAG, "CASendLEMulticastData");

    return 0;
}

CAResult_t CAStartLENotifyServer()
{
    OIC_LOG_V(DEBUG, TAG, "CAStartLENotifyServer");

    return CA_STATUS_OK;
}

uint32_t CASendLENotification(const CARemoteEndpoint_t* endpoint, void* data, uint32_t dataLen)
{
    OIC_LOG_V(DEBUG, TAG, "CASendLENotification");

    return 0;
}

CAResult_t CAGetLEInterfaceInformation(CALocalConnectivityt_t** info, uint32_t* size)
{
    OIC_LOG_V(DEBUG, TAG, "CAGetLEInterfaceInformation");

    return CA_STATUS_OK;
}

CAResult_t CAReadLEData()
{
    OIC_LOG_V(DEBUG, TAG, "Read LE Data");

    return CA_STATUS_OK;
}

CAResult_t CAStopLE()
{
    OIC_LOG_V(DEBUG, TAG, "CAStopLE");

    return CA_STATUS_OK;
}

void CATerminateLE()
{
    OIC_LOG_V(DEBUG, TAG, "TerminatLE");
}
