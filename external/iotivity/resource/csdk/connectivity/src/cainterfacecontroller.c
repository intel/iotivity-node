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
#include <stdint.h>

#include "cainterfacecontroller.h"
#include "caedradapter.h"
#include "caleadapter.h"
#include "cawifiethernetadapter.h"
#include "canetworkconfigurator.h"
#include "logger.h"

#define TAG PCF("CA")

#define CA_CONNECTIVITY_TYPE_NUM   4

static CAConnectivityHandler_t gAdapterHandler[CA_CONNECTIVITY_TYPE_NUM];

static CANetworkPacketReceivedCallback gNetworkPacketReceivedCallback = NULL;

static int8_t CAGetAdapterIndex(CAConnectivityType_t cType)
{
    switch (cType)
    {
        case CA_ETHERNET:
            return 0;
        case CA_WIFI:
            return 1;
        case CA_EDR:
            return 2;
        case CA_LE:
            return 3;
    }
    return -1;
}

static void CARegisterCallback(CAConnectivityHandler_t handler, CAConnectivityType_t cType)
{
    int8_t index = -1;

    index = CAGetAdapterIndex(cType);

    if (index == -1)
    {
        OIC_LOG(DEBUG, TAG, "unknown connectivity type!");
        return;
    }

    memcpy(&gAdapterHandler[index], &handler, sizeof(CAConnectivityHandler_t));

    OIC_LOG_V(DEBUG, TAG, "%d type adapter, register complete!", cType);
}

static void CAReceivedPacketCallback(CARemoteEndpoint_t* endpoint, void* data, uint32_t dataLen)
{
    OIC_LOG(DEBUG, TAG, "receivedPacketCallback in interface controller");

    // Call the callback.
    if (gNetworkPacketReceivedCallback != NULL)
    {
        gNetworkPacketReceivedCallback(endpoint, data, dataLen);
    }
}

static void CANetworkChangedCallback(CALocalConnectivityt_t* info, CANetworkStatus_t status)
{
    OIC_LOG(DEBUG, TAG, "Network Changed callback");
}

void CAInitializeAdapters()
{
    OIC_LOG(DEBUG, TAG, "initialize adapters..");

    memset(gAdapterHandler, 0, sizeof(CAConnectivityHandler_t) * CA_CONNECTIVITY_TYPE_NUM);

    // Initialize adapters and register callback.
#ifdef ETHERNET_ADAPTER
    CAInitializeEthernet(CARegisterCallback, CAReceivedPacketCallback, CANetworkChangedCallback);
#endif /* ETHERNET_ADAPTER */

#ifdef WIFI_ADAPTER
    CAInitializeWifi(CARegisterCallback, CAReceivedPacketCallback, CANetworkChangedCallback);
#endif /* WIFI_ADAPTER */

#ifdef EDR_ADAPTER
    CAInitializeEDR(CARegisterCallback, CAReceivedPacketCallback, CANetworkChangedCallback);
#endif /* EDR_ADAPTER */

#ifdef LE_ADAPTER
    CAInitializeLE(CARegisterCallback, CAReceivedPacketCallback, CANetworkChangedCallback);
#endif /* LE_ADAPTER */

}

void CASetPacketReceivedCallback(CANetworkPacketReceivedCallback callback)
{
    OIC_LOG(DEBUG, TAG, "Set packet received callback");

    gNetworkPacketReceivedCallback = callback;
}

void CAStartAdapter(CAConnectivityType_t cType)
{
    OIC_LOG_V(DEBUG, TAG, "Start the adapter of CAConnectivityType[%d]", cType);

    int8_t index = -1;

    index = CAGetAdapterIndex(cType);

    if (index == -1)
    {
        OIC_LOG(DEBUG, TAG, "unknown connectivity type!");
        return;
    }

    if (gAdapterHandler[index].startAdapter != NULL)
    {
        gAdapterHandler[index].startAdapter();
    }
}

void CAStopAdapter(CAConnectivityType_t cType)
{
    OIC_LOG_V(DEBUG, TAG, "Stop the adapter of CAConnectivityType[%d]", cType);

    int8_t index = -1;

    index = CAGetAdapterIndex(cType);

    if (index == -1)
    {
        OIC_LOG(DEBUG, TAG, "unknown connectivity type!");
        return;
    }

    if (gAdapterHandler[index].stopAdapter != NULL)
    {
        gAdapterHandler[index].stopAdapter();
    }
}

CAResult_t CASendUnicastData(const CARemoteEndpoint_t* endpoint, void* data, uint32_t length)
{
    OIC_LOG(DEBUG, TAG, "Send unicast data to enabled interface..");

    int8_t index = -1;
    CAResult_t res = CA_STATUS_FAILED;

    if (endpoint == NULL)
    {
        OIC_LOG_V(DEBUG, TAG, "Invalid endpoint");
        return CA_STATUS_INVALID_PARAM;
    }

    CAConnectivityType_t type = endpoint->connectivityType;

    index = CAGetAdapterIndex(type);

    if (index == -1)
    {
        OIC_LOG(DEBUG, TAG, "unknown connectivity type!");
        return CA_STATUS_INVALID_PARAM;
    }

    if (gAdapterHandler[index].sendData != NULL)
    {
        res = gAdapterHandler[index].sendData(endpoint, data, length);
    }

    return res;
}

CAResult_t CASendMulticastData(void* data, uint32_t length)
{
    OIC_LOG(DEBUG, TAG, "Send multicast data to enabled interface..");

    uint8_t i, type;
    int8_t index = -1;
    CAResult_t res = CA_STATUS_FAILED;
    u_arraylist_t *list = CAGetSelectedNetworkList();

    if (!list)
    {
        OIC_LOG(DEBUG, TAG, "No selected network");
        return CA_STATUS_FAILED;
    }

    for (i = 0; i < u_arraylist_length(list); i++)
    {
        type = *(int*) u_arraylist_get(list, i);

        index = CAGetAdapterIndex(type);

        if (index == -1)
        {
            OIC_LOG(DEBUG, TAG, "unknown connectivity type!");
            continue;
        }

        if (gAdapterHandler[index].sendDataToAll != NULL)
        {
            res = gAdapterHandler[index].sendDataToAll(data, length);
        }
    }

    return res;
}

CAResult_t CAStartListeningServerAdapters()
{
    OIC_LOG(DEBUG, TAG, "Start listening server from adapters..");

    uint8_t i, type;
    int8_t index = -1;
    u_arraylist_t *list = CAGetSelectedNetworkList();

    if (!list)
    {
        OIC_LOG(DEBUG, TAG, "No selected network");
        return CA_STATUS_FAILED;
    }

    for (i = 0; i < u_arraylist_length(list); i++)
    {
        type = *(int*) u_arraylist_get(list, i);

        index = CAGetAdapterIndex(type);

        if (index == -1)
        {
            OIC_LOG(DEBUG, TAG, "unknown connectivity type!");
            continue;
        }

        if (gAdapterHandler[index].startListenServer != NULL)
        {
            gAdapterHandler[index].startListenServer();
        }
    }

    return CA_STATUS_OK;
}

CAResult_t CAStartDiscoveryServerAdapters()
{
    OIC_LOG(DEBUG, TAG, "Start discovery server from adapters..");

    uint8_t i, type;
    int8_t index = -1;
    u_arraylist_t *list = CAGetSelectedNetworkList();

    if (!list)
    {
        OIC_LOG(DEBUG, TAG, "No selected network");
        return CA_STATUS_FAILED;
    }

    for (i = 0; i < u_arraylist_length(list); i++)
    {
        type = *(int*) u_arraylist_get(list, i);

        index = CAGetAdapterIndex(type);

        if (index == -1)
        {
            OIC_LOG_V(DEBUG, TAG, "unknown connectivity type!");
            continue;
        }

        if (gAdapterHandler[index].startDiscoverServer != NULL)
        {
            gAdapterHandler[index].startDiscoverServer();
        }
    }

    return CA_STATUS_OK;
}

void CATerminateAdapters()
{
    OIC_LOG(DEBUG, TAG, "terminate all adapters..");

    uint8_t index;

    for (index = 0; index < CA_CONNECTIVITY_TYPE_NUM; index++)
    {
        if (gAdapterHandler[index].terminate != NULL)
        {
            gAdapterHandler[index].terminate();
        }
    }
}
