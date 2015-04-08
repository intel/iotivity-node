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

/**
 * @file caadapterinterface.h
 * @brief This file contains the APIs for adapters to be implemented
 */
#ifndef __CA_ADAPTER_INTERFACE_H_
#define __CA_ADAPTER_INTERFACE_H_

#include "cacommon.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief Starting connectivity adapters and each adapter have transport specific behavior.
 *  Transport Specific Behavior:
 *   WIFI/ETH connectivity Starts unicast server on  all available IPs and defined port number as per specification.
 *   EDR will not start any specific servers.
 *   LE will not start any specific servers.
 * @return CA_STATUS_OK or ERROR CODES ( CAResult_t error codes in cacommon.h)
 */
typedef CAResult_t (*CAAdapterStart)();

/**
 * @brief Starting listening server for receiving multicast search requests
 * Transport Specific Behavior:
 *   WIFI/ETH Starts multicast server on  all available IPs and defined port number and as per specification.
 *   EDR  Starts RFCOMM Server with prefixed UUID as per specification.
 *   LE Start GATT Server with prefixed UUID and Characteristics as per OIC Specification.
 * @return CA_STATUS_OK or ERROR CODES ( CAResult_t error codes in cacommon.h)
 */
typedef CAResult_t (*CAAdapterStartListeningServer)();

/**
 * @brief for starting discovery servers for receiving multicast advertisements
 * Transport Specific Behavior:
 *   WIFI/ETH Starts multicast server on all available IPs and defined port number as per OIC Specification.
 *   EDR Starts RFCOMM Server with prefixed UUID as per OIC Specification.
 *   LE Starts GATT Server with prefixed UUID and Characteristics as per OIC Specification.
 * @return CA_STATUS_OK or ERROR CODES ( CAResult_t error codes in cacommon.h)
 */
typedef CAResult_t (*CAAdapterStartDiscoveryServer)();

/**
 * @brief Sends data to the endpoint using the adapter connectivity.
 * Note: length must be > 0.
 * @param   endpoint    [IN]    Remote Endpoint information (like ipaddress , port, reference uri and connectivity type) to
 *                              which the unicast data has to be sent.
 * @param   data        [IN]    Data which required to be sent.
 * @param   dataLen     [IN]    Size of data to be sent.
 * @return - The number of bytes sent on the network. Return value equal to zero indicates error.
 */
typedef uint32_t (*CAAdapterSendUnitcastData)(const CARemoteEndpoint_t* endpoint, void* data,
        uint32_t dataLen);

/**
 * @brief Sends Multicast data to the endpoint using the adapter connectivity.
 * Note: length must be > 0.
 * @param   data        [IN]    Data which required to be sent.
 * @param   dataLen     [IN]    Size of data to be sent.
 * @return - The number of bytes sent on the network. Return value equal to zero indicates error.
 */
typedef uint32_t (*CAAdapterSendMulticastData)(void* data, uint32_t dataLen);

/**
 * @brief Starts notification server on adapters.
 * @return CA_STATUS_OK or ERROR CODES ( CAResult_t error codes in cacommon.h)
 */
typedef CAResult_t (*CAAdapterStartNotificationRecvServer)();

/**
 * @brief Send notification information to the given endpoint.
 * Note: length must be > 0.
 * @param   endpoint    [IN]    Remote Endpoint information (like ipaddress , port, reference uri and connectivity type) to
 *                              which the unicast data has to be sent.
 * @param   data        [IN]    Data which required to be sent.
 * @param   dataLen     [IN]    Size of data to be sent.
 * @return - The number of bytes sent on the network. Return value equal to zero indicates error.
 */
typedef uint32_t (*CAAdapterSendNotification)(const CARemoteEndpoint_t* endpoint, void* data,
        uint32_t dataLen);

/**
 * @brief Get Network Information
 * @param   info        [OUT]   Local connectivity information structures
 * @param   size        [OUT]   Number of local connectivity structures.
 * @return CA_STATUS_OK or ERROR CODES ( CAResult_t error codes in cacommon.h)
 */
typedef CAResult_t (*CAAdapterGetNetworkInfo)(CALocalConnectivityt_t** info, uint32_t* size);

/**
 * @brief Read Synchronous API callback.
 * @return CA_STATUS_OK or ERROR CODES ( CAResult_t error codes in cacommon.h)
 */
typedef CAResult_t (*CAAdapterReadData)();

/**
 * @brief Stopping the adapters and close socket connections
 * Transport Specific Behavior:
 *   WIFI/ETH Stops all listening servers and close sockets.
 *   EDR Stops all RFCOMM servers and close sockets.
 *   LE Stops all GATT servers and close sockets.
 * @return CA_STATUS_OK or ERROR CODES ( CAResult_t error codes in cacommon.h)
 */
typedef CAResult_t (*CAAdapterStop)();

/**
 * @brief Terminate the connectivity adapter.Configuration information will be deleted from further use
 */
typedef void (*CAAdapterTerminate)();

/**
 * @brief Connectivity handler information for adapter
 */
typedef struct
{
    /** Start Transport specific functions*/
    CAAdapterStart startAdapter;
    /** Listening Server function address*/
    CAAdapterStartListeningServer startListenServer;
    /** Discovery Server function address **/
    CAAdapterStartDiscoveryServer startDiscoverServer;
    /** Unicast data function address**/
    CAAdapterSendUnitcastData sendData;
    /** Multicast data function address**/
    CAAdapterSendMulticastData sendDataToAll;
    /** Notify server function address**/
    CAAdapterStartNotificationRecvServer startNotifyServer;
    /** Send Notification function address**/
    CAAdapterSendNotification sendNotification;
    /** Get Networking information  **/
    CAAdapterGetNetworkInfo GetnetInfo;
    /** Read Data function address**/
    CAAdapterReadData readData;
    /** Stop Transport specific functions*/
    CAAdapterStop stopAdapter;
    /** Terminate function address stored in this pointer**/
    CAAdapterTerminate terminate;

} CAConnectivityHandler_t;

/**
 * @brief This will be used during the registration of adapters call backs to the common logic
 * @see CAConnectivityHandler_t , CAConnectivityType_t
 */
typedef void (*CARegisterConnectivityCallback)(CAConnectivityHandler_t handler,
        CAConnectivityType_t cType);

/**
 * @brief This will be used during the recive of network requests and response.
 * @see SendUnitcastData(), SendMulticastData()
 */
typedef void (*CANetworkPacketReceivedCallback)(CARemoteEndpoint_t* endPoint, void* data,
        uint32_t dataLen);

/**
 * @brief This will be used to intimate network changes to the connectivity common logic layer
 * @see SendUnitcastData(), SendMulticastData()
 */
typedef void (*CANetworkChangeCallback)(CALocalConnectivityt_t* info, CANetworkStatus_t status);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  // __CA_ADAPTER_INTERFACE_H_
