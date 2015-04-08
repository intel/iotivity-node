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
#include "cawifiethernetadapter.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "caadapterutils.h"
#include "umutex.h"

#ifdef __TIZEN__
#include "cawifiserver.h"
#include "cawificlient.h"
#include "cawifimonitor.h"
#elif defined(OIC_ARDUINODUE)
#include <logger.h>
#include <string.h>
#include "cawifiadapter.h"
#endif //#ifdef __TIZEN__
/**
 * @def WIFI_ETHERNET_ADAPTER_TAG
 * @brief Logging tag for module name
 */
#define WIFI_ETHERNET_ADAPTER_TAG "WIFI_ETHERNET_ADAPTER"

/**
 * @def CA_PORT
 * @brief Port to listen for incoming data
 */
#define CA_PORT   5283

#define CA_MCAST_PORT   5298

/**
 * @def CA_MULTICAST_IP
 * @brief Multicast IP Address
 */
#define CA_MULTICAST_IP "224.0.1.187"

/**
 * @var gNetworkCallback
 * @brief Network callback
 */
static CANetworkChangeCallback gNetworkCallback = NULL;

/**
 * @var gIsMulticastServerStarted
 * @brief Flag to check if multicast server is started
 */
static int gIsMulticastServerStarted = 0;
static u_mutex gMutexIsMulticastServerStarted = NULL;

void CAInitializeMutex()
{
    u_mutex_init();
    gMutexIsMulticastServerStarted = u_mutex_new();
}

int CAWIFIRegisterNetworkNotifications(CANetworkChangeCallback netCallback)
{
    OCLog(DEBUG, WIFI_ETHERNET_ADAPTER_TAG, "IN");

    gNetworkCallback = netCallback; //TODO: Remove and set using API define in arduino specific file
#ifdef __TIZEN__
            if (netCallback != NULL)
            {
                CAInitializeWIFIAdapter();
                CASetWIFINetworkChangeCallback(netCallback);
            }
            else
            {
                CADeinitializeWIFIAdapter();
            }
#endif

    OCLog(DEBUG, WIFI_ETHERNET_ADAPTER_TAG, "OUT");
    return CA_STATUS_OK;
}

CAResult_t CAInitializeWifi(CARegisterConnectivityCallback registerCallback,
        CANetworkPacketReceivedCallback reqRespCallback, CANetworkChangeCallback netCallback)
{
    OCLog(DEBUG, WIFI_ETHERNET_ADAPTER_TAG, "IN");
    CAResult_t ret = CA_STATUS_OK;

    VERIFY_NON_NULL(registerCallback, WIFI_ETHERNET_ADAPTER_TAG, "Invalid argument");
    VERIFY_NON_NULL(reqRespCallback, WIFI_ETHERNET_ADAPTER_TAG, "Invalid argument");
    VERIFY_NON_NULL(netCallback, WIFI_ETHERNET_ADAPTER_TAG, "Invalid argument");

    CAConnectivityHandler_t wifiHandler;
    wifiHandler.startAdapter = CAStartWIFI;
    wifiHandler.startListenServer = CAStartWIFIListeningServer;
    wifiHandler.startDiscoverServer = CAStartWIFIDiscoveryServer;
    wifiHandler.sendData = CASendWIFIUnicastData;
    wifiHandler.sendDataToAll = CASendWIFIMulticastData;
    wifiHandler.startNotifyServer = CAStartWIFINotifyRecvServers;
    wifiHandler.sendNotification = CASendWIFINotification;
    wifiHandler.GetnetInfo = CAGetWIFIInterfaceInformation;
    wifiHandler.readData = CAReadWIFIData;
    wifiHandler.stopAdapter = CAStopWIFI;
    wifiHandler.terminate = CATerminateWifi;
    registerCallback(wifiHandler, CA_WIFI);

    CASetWIFINetworkPacketCallback(reqRespCallback);
    CAWIFIRegisterNetworkNotifications(netCallback);
    CAInitializeMutex();
    CAInitializeServerMutex();
    //ret = CAStartWIFIUnicastServer();

    OCLog(INFO, WIFI_ETHERNET_ADAPTER_TAG, "IntializeWifi Success");

    OCLog(DEBUG, WIFI_ETHERNET_ADAPTER_TAG, "OUT");
    return ret;
}

CAResult_t CAStartWIFI()
{
    OCLog(DEBUG, WIFI_ETHERNET_ADAPTER_TAG, "IN");
    int16_t unicastPort = CA_PORT;
    CAResult_t ret = CA_STATUS_OK;

#ifdef __TIZEN__
    CASetIsStartServerInvoked();
    int retVal = CAIsWIFIConnected();
    if(retVal == 0)
    {
        OCLog(DEBUG, WIFI_ETHERNET_ADAPTER_TAG, "WIFI is not Connected");
        return ret;
    }
#endif
    /*Address is hardcoded as we are using Single Interface*/
    ret = CAStartUnicastServer("0.0.0.0", &unicastPort);

    OCLog(DEBUG, WIFI_ETHERNET_ADAPTER_TAG, "OUT");
    return ret;
}

CAResult_t CAStartWIFIListeningServer()
{
    OCLog(DEBUG, WIFI_ETHERNET_ADAPTER_TAG, "IN");

    CAResult_t ret = CA_STATUS_OK;
    int16_t multicastPort = CA_MCAST_PORT;

    if (gIsMulticastServerStarted == 1)
    {
        OCLog(DEBUG, WIFI_ETHERNET_ADAPTER_TAG, "Server is already Started");
        return CA_STATUS_OK;
    }
#ifdef __TIZEN__
    int retVal = CAIsWIFIConnected();
    if(retVal == 0)
    {
        OCLog(DEBUG, WIFI_ETHERNET_ADAPTER_TAG, "WIFI is not Connected");
        return CA_STATUS_FAILED;
    }
#endif
    ret = CAStartMulticastServer(CA_MULTICAST_IP, "0.0.0.0", &multicastPort);
    if (ret == CA_STATUS_OK)
    {
        OCLog(DEBUG, WIFI_ETHERNET_ADAPTER_TAG, "Multicast Server is Started Successfully");
        u_mutex_lock(gMutexIsMulticastServerStarted);
        gIsMulticastServerStarted = 1;
        u_mutex_unlock(gMutexIsMulticastServerStarted);
    }

    OCLog(DEBUG, WIFI_ETHERNET_ADAPTER_TAG, "OUT");
    return ret;
}

CAResult_t CAStartWIFIDiscoveryServer()
{
    OCLog(DEBUG, WIFI_ETHERNET_ADAPTER_TAG, "IN");

    CAResult_t ret = CA_STATUS_OK;
    int16_t multicastPort = CA_MCAST_PORT;

    if (gIsMulticastServerStarted == 1)
    {
        OCLog(DEBUG, WIFI_ETHERNET_ADAPTER_TAG, "Server is already Started");
        return CA_STATUS_OK;
    }
#ifdef __TIZEN__
    /*int retVal = CAIsWIFIConnected();
     if(retVal == 0)
     {
     OCLog(DEBUG, WIFI_ETHERNET_ADAPTER_TAG, "WIFI is not Connected");
     return CA_STATUS_FAILED;
     }*/ //VV
#endif
    ret = CAStartMulticastServer(CA_MULTICAST_IP, "0.0.0.0", &multicastPort);
    if (ret == CA_STATUS_OK)
    {
        OCLog(DEBUG, WIFI_ETHERNET_ADAPTER_TAG, "Multicast Server is Started Successfully");
        u_mutex_lock(gMutexIsMulticastServerStarted);
        gIsMulticastServerStarted = 1;
        u_mutex_unlock(gMutexIsMulticastServerStarted);
    }

    OCLog(DEBUG, WIFI_ETHERNET_ADAPTER_TAG, "OUT");
    return ret;
}

uint32_t CASendWIFIUnicastData(const CARemoteEndpoint_t *remoteEndpoint, void *data,
        uint32_t dataLen)
{
    OCLog(DEBUG, WIFI_ETHERNET_ADAPTER_TAG, "IN");

    uint32_t dataSize = 0;

    VERIFY_NON_NULL_RET(remoteEndpoint, WIFI_ETHERNET_ADAPTER_TAG, "Invalid argument", dataSize);
    VERIFY_NON_NULL_RET(data, WIFI_ETHERNET_ADAPTER_TAG, "Invalid argument", dataSize);

    if (dataLen <= 0)
    {
        OCLog(DEBUG, WIFI_ETHERNET_ADAPTER_TAG, "Data Length is 0");
        return dataSize;

    }

    dataSize = CAWIFISendData(remoteEndpoint->addressInfo.IP.ipAddress,
            remoteEndpoint->addressInfo.IP.port, data, dataLen, 0);

    OCLog(DEBUG, WIFI_ETHERNET_ADAPTER_TAG, "OUT");
    return dataSize;
}

uint32_t CASendWIFIMulticastData(void *data, uint32_t dataLength)
{
    OCLog(DEBUG, WIFI_ETHERNET_ADAPTER_TAG, "IN");

    uint32_t dataSize = 0;

    VERIFY_NON_NULL_RET(data, WIFI_ETHERNET_ADAPTER_TAG, "Invalid argument", dataSize);

    if (dataLength <= 0)
    {
        OCLog(DEBUG, WIFI_ETHERNET_ADAPTER_TAG, "Data Length is 0");
        return 0;

    }
    dataSize = CAWIFISendData(CA_MULTICAST_IP, CA_MCAST_PORT, data, dataLength, 1);

    OCLog(DEBUG, WIFI_ETHERNET_ADAPTER_TAG, "OUT");
    return dataSize;
}

CAResult_t CAStartWIFINotifyRecvServers()
{
    OCLog(DEBUG, WIFI_ETHERNET_ADAPTER_TAG, "IN");

    OCLog(DEBUG, WIFI_ETHERNET_ADAPTER_TAG, "OUT");
    return CA_STATUS_OK;
}

uint32_t CASendWIFINotification(const CARemoteEndpoint_t* endpoint, void* data, uint32_t dataLen)
{
    OCLog(DEBUG, WIFI_ETHERNET_ADAPTER_TAG, "IN");

    OCLog(DEBUG, WIFI_ETHERNET_ADAPTER_TAG, "OUT");
    return CA_STATUS_OK;
}

CAResult_t CAGetWIFIInterfaceInformation(CALocalConnectivityt_t **info, uint32_t* size)
{
    OCLog(DEBUG, WIFI_ETHERNET_ADAPTER_TAG, "IN");

    VERIFY_NON_NULL(info, WIFI_ETHERNET_ADAPTER_TAG, "Invalid argument");
    //VERIFY_NON_NULL(size, WIFI_ETHERNET_ADAPTER_TAG, "Invalid argument");
    char localIpAddress[CA_IPADDR_SIZE];
    int32_t localIpAddressLen = sizeof(localIpAddress);
#ifdef __TIZEN__
    OCLog(DEBUG, WIFI_ETHERNET_ADAPTER_TAG, "Getting WIFI address");
    CAGetInterfaceAddress(localIpAddress, localIpAddressLen);
    OCLog(DEBUG, WIFI_ETHERNET_ADAPTER_TAG, "Got WIFI address");
#else
    OCLog(DEBUG, WIFI_ETHERNET_ADAPTER_TAG, "Getting shield address");
    arduinoGetInterfaceAddress(localIpAddress, localIpAddressLen);
    OCLog(DEBUG, WIFI_ETHERNET_ADAPTER_TAG, "Got shield address");
#endif //#if __ARDUINO__
    /**
     * Create local endpoint using util function
     */
    (*info) = CreateLocalEndpoint(CA_WIFI, localIpAddress, "WiFi");
    if (NULL == (*info))
    {
        OCLog(ERROR, WIFI_ETHERNET_ADAPTER_TAG, "Error while Createing Memory");
        return CA_STATUS_FAILED;
    }

    (*size) = 1;

    OCLog(INFO, WIFI_ETHERNET_ADAPTER_TAG, "GetWIFIInterfaceInformation success");

    OCLog(DEBUG, WIFI_ETHERNET_ADAPTER_TAG, "OUT");
    return CA_STATUS_OK;
}

CAResult_t CAReadWIFIData()
{
    OCLog(DEBUG, WIFI_ETHERNET_ADAPTER_TAG, "IN");
#ifdef __ARDUINO__
    arduoinoCheckServerData();
#endif //#ifdef __ARDUINO__
    OCLog(DEBUG, WIFI_ETHERNET_ADAPTER_TAG, "OUT");
    return CA_STATUS_OK;
}

CAResult_t CAStopWIFI()
{
    CAResult_t result = CA_STATUS_FAILED;
    result = CAStopUnicastServer();
    if (result != CA_STATUS_OK)
    {
        OCLog(ERROR, WIFI_ETHERNET_ADAPTER_TAG, "Error while stopping UnicastServer");
        return CA_STATUS_FAILED;
    }
#ifdef __TIZEN__
    CAUnsetIsStartServerInvoked();
#endif
    result = CAStopMulticastServer();
    if (result != CA_STATUS_OK)
    {
        OCLog(ERROR, WIFI_ETHERNET_ADAPTER_TAG, "Error while stopping MulticastServer");
        return CA_STATUS_FAILED;
    }
    else
    {
        u_mutex_lock(gMutexIsMulticastServerStarted);
        gIsMulticastServerStarted = 0;
        u_mutex_unlock(gMutexIsMulticastServerStarted);
    }
    return CA_STATUS_OK;
}

void CATerminateWifi()
{
    OCLog(DEBUG, WIFI_ETHERNET_ADAPTER_TAG, "IN");
    CAResult_t result = CA_STATUS_FAILED;

    CASetWIFINetworkPacketCallback(NULL);
    result = CAWIFIRegisterNetworkNotifications(NULL);
    if (result != CA_STATUS_OK)
    {
        OCLog(ERROR, WIFI_ETHERNET_ADAPTER_TAG,
                "Error while wifiRegisterNetworkNotifications(NULL)");
    }

    OCLog(INFO, WIFI_ETHERNET_ADAPTER_TAG, "TerminateWifi Success");

    OCLog(DEBUG, WIFI_ETHERNET_ADAPTER_TAG, "OUT");
    return;
}
