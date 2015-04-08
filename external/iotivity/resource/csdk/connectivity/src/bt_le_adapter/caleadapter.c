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
#include "caleadapter.h"

#ifdef __TIZEN__
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <unistd.h>
#include <pthread.h>
#endif //#ifdef __TIZEN__
#include <stdio.h>
#include <stdlib.h>

#ifdef __TIZEN__
#include "cableserver.h"
#include "cableclient.h"
#include "cacommon.h"
#else // __ARDUINO__
#include "BLEAdapterArduino.h"
#include "caadapterutils.h"
#endif //#ifdef __TIZEN__
#define CALEADAPTER_TAG "CA_BLE_ADAPTER"

static CANetworkChangeCallback gNetworkCallback = NULL;
static char gLocalBLEAddress[16] =
{ 0, };

static int gIsServer = 0;

int CALERegisterNetworkNotifications(CANetworkChangeCallback netCallback);

#ifdef __TIZEN__
int CALEDeviceStateChangedCb(int result, bt_adapter_state_e adapter_state, void *user_data);
#endif //#ifdef __TIZEN__

pthread_mutex_t gBleIsServerMutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;

pthread_mutex_t gBleNetworkCbMutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;

pthread_mutex_t gBleLocalAddressMutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;

CAResult_t CAInitializeLE(CARegisterConnectivityCallback registerCallback,
        CANetworkPacketReceivedCallback reqRespCallback, CANetworkChangeCallback netCallback)
{
    OCLog(DEBUG, CALEADAPTER_TAG, "IN");

    //Input validation
    VERIFY_NON_NULL(registerCallback, NULL, "RegisterConnectivity callback is null");
    VERIFY_NON_NULL(reqRespCallback, NULL, "PacketReceived Callback is null");
    VERIFY_NON_NULL(netCallback, NULL, "NetworkChange Callback is null");

#ifdef __TIZEN__

    int ret = bt_initialize();
    if (0 != ret)
    {
        OCLog(ERROR, CALEADAPTER_TAG, "bt_initialize failed!");
        return CA_STATUS_FAILED;
    }

#endif //#ifdef __TIZEN__
    CASetBLEReqRescallback(reqRespCallback);
    CALERegisterNetworkNotifications(netCallback);

    CAConnectivityHandler_t connHandler;
    connHandler.startListenServer = CAStartLEListeningServer;
    connHandler.startDiscoverServer = CAStartLEDiscoveryServer;
    connHandler.sendData = CASendLEUnicastData;
    connHandler.sendDataToAll = CASendLEMulticastData;
    connHandler.startNotifyServer = CAStartLENotifyServer;
    connHandler.sendNotification = CASendLENotification;
    connHandler.GetnetInfo = CAGetLEInterfaceInformation;
    connHandler.readData = CAReadLEData;
    connHandler.terminate = CATerminateLE;
    registerCallback(connHandler, CA_LE);

    OCLog(DEBUG, CALEADAPTER_TAG, "OUT");

    return CA_STATUS_OK;
}

void CATerminateLE()
{
    OCLog(DEBUG, CALEADAPTER_TAG, "IN");

    CASetBLEReqRescallback(NULL);
    CALERegisterNetworkNotifications(NULL);

    pthread_mutex_lock(&gBleIsServerMutex);
    if (gIsServer == 1)
    {
        CAStopBleGattServer();
    }
    else
    {
        CAStopBLEGattClient();
    }
    pthread_mutex_unlock(&gBleIsServerMutex);

    OCLog(DEBUG, CALEADAPTER_TAG, "OUT");
    return;
}

void CALEServerInitThreadFunc(void *param)
{
    OCLog(DEBUG, CALEADAPTER_TAG, "[CALEServerInitThreadFunc]IN");

    CAStartBleGattServer();

    OCLog(DEBUG, CALEADAPTER_TAG, "[CALEServerInitThreadFunc] OUT");
}

void CALEClientInitThreadFunc(void *param)
{
    OCLog(DEBUG, CALEADAPTER_TAG, "[CALEClientInitThreadFunc]IN");

    CAStartBLEGattClient();

    OCLog(DEBUG, CALEADAPTER_TAG, "[CALEClientInitThreadFunc] OUT");
}

CAResult_t CAStartLEListeningServer()
{
    OCLog(DEBUG, CALEADAPTER_TAG, "IN");

    int init_pthread_status = 0;
    pthread_t pthread_id = 0;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    init_pthread_status = pthread_create(&pthread_id, &attr, CALEServerInitThreadFunc, NULL);

    if (init_pthread_status != 0)
    {
        OCLog(ERROR, CALEADAPTER_TAG, "pthread_create failed!");
        return CA_STATUS_FAILED;
    }

    OCLog(DEBUG, CALEADAPTER_TAG, "OUT");
    pthread_mutex_lock(&gBleIsServerMutex);
    gIsServer = 1;
    pthread_mutex_unlock(&gBleIsServerMutex);
    return CA_STATUS_OK;
}

CAResult_t CAStartLEDiscoveryServer()
{
    OCLog(DEBUG, CALEADAPTER_TAG, "IN");

    int init_pthread_status = 0;
    pthread_t pthread_id = 0;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    init_pthread_status = pthread_create(&pthread_id, &attr, CALEClientInitThreadFunc, NULL);

    if (init_pthread_status != 0)
    {
        OCLog(ERROR, CALEADAPTER_TAG, "pthread_create failed!");
        return CA_STATUS_FAILED;
    }

    OCLog(DEBUG, CALEADAPTER_TAG, "OUT");
    pthread_mutex_lock(&gBleIsServerMutex);
    gIsServer = 0;
    pthread_mutex_unlock(&gBleIsServerMutex);
    return CA_STATUS_OK;
}

CAResult_t CAStartLENotifyServer()
{
    OCLog(DEBUG, CALEADAPTER_TAG, "IN");

    OCLog(DEBUG, CALEADAPTER_TAG, "OUT");

    return CA_STATUS_OK;
}

uint32_t CASendLENotification(const CARemoteEndpoint_t *endpoint, void *data, uint32_t dataLen)
{
    OCLog(DEBUG, CALEADAPTER_TAG, "IN");

    OCLog(DEBUG, CALEADAPTER_TAG, "OUT");

    return CA_STATUS_OK;
}

CAResult_t CAReadLEData()
{
    OCLog(DEBUG, CALEADAPTER_TAG, "IN");

    OCLog(DEBUG, CALEADAPTER_TAG, "OUT");

    return CA_STATUS_OK;
}

uint32_t CASendLEUnicastData(const CARemoteEndpoint_t *endpoint, void *data, uint32_t dataLen)
{
    OCLog(DEBUG, CALEADAPTER_TAG, "IN");

    //Input validation
    VERIFY_NON_NULL(endpoint, NULL, "Remote endpoint is null");
    VERIFY_NON_NULL(data, NULL, "Data is null");

    CAResult_t result = CA_STATUS_FAILED;

#ifdef __TIZEN__
    pthread_mutex_lock(&gBleIsServerMutex);
    if (gIsServer)
    {
        result = CAUpdateCharacteristicsInGattServer(data, dataLen);
        if (CA_STATUS_OK != result)
        {
            OCLogv(ERROR, CALEADAPTER_TAG,
                    "[SendLEUnicastData] sending unicast data to [%s] failed\n", endpoint->addressInfo.BT.btMacAddress);
            pthread_mutex_unlock(&gBleIsServerMutex);
            return 0;
        }
    }
    else
    {

        result = CAUpdateCharacteristicsToGattServer(endpoint->addressInfo.BT.btMacAddress, data,
                dataLen, UNICAST, 0);
        if (CA_STATUS_OK != result)
        {
            OCLogv(ERROR, CALEADAPTER_TAG,
                    "[SendLEUnicastData] sending unicast data to [%s] failed\n", endpoint->addressInfo.BT.btMacAddress);
            pthread_mutex_unlock(&gBleIsServerMutex);
            return 0;
        }
    }
    pthread_mutex_unlock(&gBleIsServerMutex);
#else
    char *tempPath = "temp_path";
    updateCharacteristicsInGattServer(tempPath, (char *) data, dataLen);
#endif //#ifdef __TIZEN__
    OCLog(DEBUG, CALEADAPTER_TAG, "OUT");

    return dataLen;
}

uint32_t CASendLEMulticastData(void *data, uint32_t dataLen)
{
    OCLog(DEBUG, CALEADAPTER_TAG, "IN");

    //Input validation
    VERIFY_NON_NULL(data, NULL, "Data is null");

    if (0 >= dataLen)
    {
        OCLog(ERROR, CALEADAPTER_TAG, "Invalid Parameter");
        return 0;
    }

    CAResult_t result = CA_STATUS_FAILED;
#ifdef __TIZEN__
    pthread_mutex_lock(&gBleIsServerMutex);
    if (gIsServer)
    {
        result = CAUpdateCharacteristicsInGattServer(data, dataLen);
        if (CA_STATUS_OK != result)
        {
            OCLogv(ERROR, CALEADAPTER_TAG,
                    "[CASendLEMulticastData] updating data in server is failed");
            pthread_mutex_unlock(&gBleIsServerMutex);
            return 0;
        }
    }
    else
    {
        result = CAUpdateCharacteristicsToAllGattServers(data, dataLen);
        if (CA_STATUS_OK != result)
        {
            OCLogv(ERROR, CALEADAPTER_TAG,
                    "[SendLEMulticastDataToAll] multicasting data to servers failed" );
            pthread_mutex_unlock(&gBleIsServerMutex);
            return 0;
        }
    }
    pthread_mutex_unlock(&gBleIsServerMutex);
#else
    char *tempPath = "temp_path";
    updateCharacteristicsInGattServer(tempPath, (char *) data, dataLen);
#endif //#ifdef __TIZEN__
    OCLog(DEBUG, CALEADAPTER_TAG, "OUT");
    return dataLen;
}

CAResult_t CAGetLEInterfaceInformation(CALocalConnectivityt_t **info, uint32_t *size)
{
    OCLog(DEBUG, CALEADAPTER_TAG, "IN");

    VERIFY_NON_NULL(info, NULL, "CALocalConnectivity info is null");

#ifdef OIC_ARDUINODUE
    OCLog(DEBUG, CALEADAPTER_TAG, "Info from ARDUINO");
    //1: call corresponding Arduino API
#endif

#if __TIZEN__

    char *local_address = NULL;

    bt_adapter_get_address(&local_address);
    if (NULL == local_address)
    {
        OCLog(ERROR, CALEADAPTER_TAG, "Get local bt adapter address failed");
        return CA_STATUS_FAILED;
    }

#endif //#if ARDUINODUE
    *size = 0;
    (*info) = (CALocalConnectivityt_t *) OICMalloc(sizeof(CALocalConnectivityt_t));
    if (NULL == (*info))
    {
        OCLog(ERROR, CALEADAPTER_TAG, "Malloc failure!");
        return CA_STATUS_FAILED;
    }
    memset((*info), 0x0, sizeof(CALocalConnectivityt_t));

    strncpy((*info)->addressInfo.BT.btMacAddress, local_address, strlen(local_address));
    pthread_mutex_lock(&gBleLocalAddressMutex);
    strncpy(gLocalBLEAddress, local_address, sizeof(gLocalBLEAddress));
    pthread_mutex_unlock(&gBleLocalAddressMutex);

    (*info)->type = CA_LE;
    *size = 1;
    OICFree(local_address);

    OCLog(DEBUG, CALEADAPTER_TAG, "OUT");
    return CA_STATUS_OK;
}

int CALERegisterNetworkNotifications(CANetworkChangeCallback netCallback)
{
    OCLog(DEBUG, CALEADAPTER_TAG, "IN");

    pthread_mutex_lock(&gBleNetworkCbMutex);
    gNetworkCallback = netCallback;
    pthread_mutex_unlock(&gBleNetworkCbMutex);
    int ret = 0;
#ifdef __TIZEN__
    if (netCallback)
    {
        ret = bt_adapter_set_state_changed_cb(CALEDeviceStateChangedCb, NULL);
        if(ret != 0)
        {
            OCLog(ERROR, CALEADAPTER_TAG, "bt_adapter_set_state_changed_cb failed!");
        }
    }
    else
    {
        ret = bt_adapter_unset_state_changed_cb();
        if(ret != 0)
        {
            OCLog(ERROR, CALEADAPTER_TAG, "bt_adapter_set_state_changed_cb failed!");
        }
    }
#endif //#ifdef __TIZEN__
    OCLog(DEBUG, CALEADAPTER_TAG, "OUT");
    return CA_STATUS_OK;
}

#ifdef __TIZEN__

int CALEDeviceStateChangedCb(int result, bt_adapter_state_e adapter_state, void *user_data)
{
    OCLog(DEBUG, CALEADAPTER_TAG, "IN");

    bt_adapter_state_e btAdaptorState = BT_ADAPTER_DISABLED;
    if (BT_ADAPTER_ENABLED == adapter_state)
    {
        btAdaptorState = BT_ADAPTER_ENABLED;
    }

    CALocalConnectivityt_t localEndpoint;

    pthread_mutex_lock(&gBleLocalAddressMutex);
    strncpy(localEndpoint.addressInfo.BT.btMacAddress, gLocalBLEAddress, strlen(gLocalBLEAddress));
    pthread_mutex_unlock(&gBleLocalAddressMutex);

    pthread_mutex_lock(&gBleNetworkCbMutex);
    if(NULL != gNetworkCallback)
    {
        gNetworkCallback(&localEndpoint, adapter_state);
    }
    else
    {
        OCLog(ERROR, CALEADAPTER_TAG, "gNetworkCallback is NULL");
    }
    pthread_mutex_unlock(&gBleNetworkCbMutex);
    OCLog(DEBUG, CALEADAPTER_TAG, "OUT");
    return CA_STATUS_OK;
}
#endif //#ifdef OIC_TIZEN
