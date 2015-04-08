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

#include "caedradapter.h"
#include "logger.h"

#define TAG PCF("CA")

static CANetworkPacketReceivedCallback gEDRReceivedCallback = NULL;

CAResult_t CAInitializeEDR(CARegisterConnectivityCallback registerCallback,
        CANetworkPacketReceivedCallback reqRespCallback, CANetworkChangeCallback netCallback)
{
    OIC_LOG_V(DEBUG, TAG, "CAInitializeEDR");

    gEDRReceivedCallback = reqRespCallback;

    // register handlers
    CAConnectivityHandler_t handler;
    memset(&handler, 0, sizeof(CAConnectivityHandler_t));

    handler.startAdapter = CAStartEDR;
    handler.startListenServer = CAStartEDRListeningServer;
    handler.startDiscoverServer = CAStartEDRDiscoveryServer;
    handler.sendData = CASendEDRUnicastData;
    handler.sendDataToAll = CASendEDRMulticastData;
    handler.startNotifyServer = CAStartEDRNotifyServer;
    handler.sendNotification = CASendEDRNotification;
    handler.GetnetInfo = CAGetEDRInterfaceInformation;
    handler.readData = CAReadEDRData;
    handler.stopAdapter = CAStopEDR;
    handler.terminate = CATerminateEDR;

    registerCallback(handler, CA_EDR);

    return CA_STATUS_OK;
}

CAResult_t CAStartEDR()
{
    OIC_LOG_V(DEBUG, TAG, "CAStartEDR");

    return CA_STATUS_OK;
}

CAResult_t CAStartEDRListeningServer()
{
    OIC_LOG_V(DEBUG, TAG, "CAStartEDRListeningServer");

    return CA_STATUS_OK;
}

CAResult_t CAStartEDRDiscoveryServer()
{
    OIC_LOG_V(DEBUG, TAG, "CAStartEDRDiscoveryServer");

    return CA_STATUS_OK;
}

uint32_t CASendEDRUnicastData(const CARemoteEndpoint_t* endpoint, void* data, uint32_t dataLen)
{
    OIC_LOG_V(DEBUG, TAG, "CASendEDRUnicastData");

    return 0;
}

uint32_t CASendEDRMulticastData(void* data, uint32_t dataLen)
{
    OIC_LOG_V(DEBUG, TAG, "CASendEDRMulticastData");

    return 0;
}

CAResult_t CAStartEDRNotifyServer()
{
    OIC_LOG_V(DEBUG, TAG, "CAStartEDRNotifyServer");

    return CA_STATUS_OK;
}

uint32_t CASendEDRNotification(const CARemoteEndpoint_t* endpoint, void* data, uint32_t dataLen)
{
    OIC_LOG_V(DEBUG, TAG, "CASendEDRNotification");

    return 0;
}

CAResult_t CAGetEDRInterfaceInformation(CALocalConnectivityt_t** info, uint32_t* size)
{
    OIC_LOG_V(DEBUG, TAG, "CAGetEDRInterfaceInformation");

    return CA_STATUS_OK;
}

CAResult_t CAReadEDRData()
{
    OIC_LOG_V(DEBUG, TAG, "Read EDR Data");

    return CA_STATUS_OK;
}

CAResult_t CAStopEDR()
{
    OIC_LOG_V(DEBUG, TAG, "CAStopEDR");

    return CA_STATUS_OK;
}

void CATerminateEDR()
{
    OIC_LOG_V(DEBUG, TAG, "CATerminateEDR");
}
