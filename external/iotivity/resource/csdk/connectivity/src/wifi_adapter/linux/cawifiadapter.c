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

#include "cawifiadapter.h"

#include "config.h"
#include "coap.h"

#include "cawificore.h"

#include "logger.h"

#define TAG PCF("CA")

// received packet callback
static CANetworkPacketReceivedCallback gWifiReceivedCallback = NULL;

static void CAWiFiPacketReceiveCallback(const char* address, const char* data)
{
    OIC_LOG_V(DEBUG, TAG,
            "CAWiFiPacketReceiveCallback, from: %s, data: %s", address, data);

    // call the callback
    if (gWifiReceivedCallback != NULL)
    {
        CARemoteEndpoint_t* endpoint = NULL;
        endpoint = (CARemoteEndpoint_t*) OICMalloc(sizeof(CARemoteEndpoint_t));

        // set address
        memset((void*) endpoint->addressInfo.IP.ipAddress, 0, CA_IPADDR_SIZE);
        if (CA_IPADDR_SIZE > strlen(address))
            strcpy((char*) endpoint->addressInfo.IP.ipAddress, address);

        // set connectivity type
        endpoint->connectivityType = CA_WIFI;

        gWifiReceivedCallback(endpoint, data, strlen(data));
    }
}

CAResult_t CAInitializeWifi(CARegisterConnectivityCallback registerCallback,
        CANetworkPacketReceivedCallback reqRespCallback, CANetworkChangeCallback netCallback)
{
    OIC_LOG(DEBUG, TAG, "IntializeWifi");

    gWifiReceivedCallback = reqRespCallback;

    // register handlers
    CAConnectivityHandler_t handler;
    memset(&handler, 0, sizeof(CAConnectivityHandler_t));

    handler.startAdapter = CAStartWIFI;
    handler.startListenServer = CAStartWIFIListeningServer;
    handler.startDiscoverServer = CAStartWIFIDiscoveryServer;
    handler.sendData = CASendWIFIUnicastData;
    handler.sendDataToAll = CASendWIFIMulticastData;
    handler.startNotifyServer = CAStartWIFINotifyRecvServers;
    handler.sendNotification = CASendWIFINotification;
    handler.GetnetInfo = CAGetWIFIInterfaceInformation;
    handler.readData = CAReadWIFIData;
    handler.stopAdapter = CAStopWIFI;
    handler.terminate = CATerminateWIfI;

    registerCallback(handler, CA_WIFI);

    CAWiFiSetCallback(CAWiFiPacketReceiveCallback);

    return 0;
}

void CATerminateWIfI()
{
    OIC_LOG(DEBUG, TAG, "TerminateWifi");

    CAWiFiTerminate();
}

CAResult_t CAStartWIFI()
{
    OIC_LOG(DEBUG, TAG, "CAStartWIFI");
    CAWiFiInitialize();

    OIC_LOG(DEBUG, TAG, "CAWiFiStartUnicastServer");
    CAWiFiStartUnicastServer("0.0.0.0", atoi("5283"));

    return 0;
}

CAResult_t CAStopWIFI()
{
    OIC_LOG(DEBUG, TAG, "CAStopWIFI");

    // ToDo:

    return 0;
}

CAResult_t CAStartWIFIListeningServer()
{
    OIC_LOG(DEBUG, TAG, "StartWIFIListeningServer");

    CAWiFiStartMulticastServer("0.0.0.0", atoi("5283"));

    return 0;
}

CAResult_t CAStartWIFIDiscoveryServer()
{
    OIC_LOG(DEBUG, TAG, "StartWIFIDiscoveryServer");

    CAWiFiStartMulticastServer("0.0.0.0", atoi("5283"));

    return 0;
}

uint32_t CASendWIFIUnicastData(const CARemoteEndpoint_t* endpoint, void* data, uint32_t dataLen)
{
    OIC_LOG(DEBUG, TAG, "SendWIFIUnicastData");

    CAWiFiSendUnicastMessage(endpoint->addressInfo.IP.ipAddress, data, dataLen);

    return 0;
}

uint32_t CASendWIFIMulticastData(void* data, uint32_t dataLen)
{
    OIC_LOG(DEBUG, TAG, "CASendWIFIMulticastData");

    CAWiFiSendMulticastMessage("0.0.0.0", (char*) data);

    return 0;
}

CAResult_t CAStartWIFINotifyRecvServers()
{
    OIC_LOG(DEBUG, TAG, "StartWIFINotifyRecvServers");

    // ToDo:

    return 0;
}

uint32_t CASendWIFINotification(const CARemoteEndpoint_t* endpoint, void* data, uint32_t dataLen)
{
    OIC_LOG(DEBUG, TAG, "SendWIFINotification");

    // ToDo:

    return 0;
}

CAResult_t CAGetWIFIInterfaceInformation(CALocalConnectivityt_t** info, uint32_t* size)
{
    OIC_LOG(DEBUG, TAG, "GetWIFIInterfaceInformation");

    // ToDo:

    return 0;
}

CAResult_t CAReadWIFIData()
{
    OIC_LOG(DEBUG, TAG, "Read WIFI Data");

    // ToDo:

    return 0;
}

