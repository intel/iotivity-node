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
 * @file caleadapter.h
 * @brief This file contains the APIs for LE adapters to be implemented
 */
#ifndef _CA_LEADAPTER_H_
#define _CA_LEADAPTER_H_

#include "cacommon.h"
#include "caadapterinterface.h"

/**
 * BLE Interface APIs.
 */
#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief Initialize LE connectivity interface.
 * @param registerCallback [IN] To register LE interfaces to Connectivity Abstraction Layer
 * @param reqRespCallback [IN] sending responses and discovery messages from unicast , multicast servers
 * @param netCallback [IN] Intimate the network additions to Connectivity Abstraction Layer.
 * @return CA_STATUS_OK or ERROR CODES ( CAResult_t error codes in cacommon.h)
 */
CAResult_t CAInitializeLE(CARegisterConnectivityCallback registerCallback,
        CANetworkPacketReceivedCallback reqRespCallback, CANetworkChangeCallback netCallback);
/**
 * @brief Starting LE connectivity adapters .As its peer to peer it doesnot require to start any servers
 * @return CA_STATUS_OK or ERROR CODES ( CAResult_t error codes in cacommon.h)
 */
CAResult_t CAStartLE();

/**
 * @brief Starting listening server for receiving multicast search requests
 * Transport Specific Behavior:
 *   LE  Starts GATT Server with prefixed UUID and Characteristics as per OIC Specification.
 * @return CA_STATUS_OK or ERROR CODES ( CAResult_t error codes in cacommon.h)
 */
CAResult_t CAStartLEListeningServer();

/**
 * @brief for starting discovery servers for receiving multicast advertisements
 * Transport Specific Behavior:
 *   LE  Starts GATT Server with prefixed UUID and Characteristics as per OIC Specification.
 * @return CA_STATUS_OK or ERROR CODES ( CAResult_t error codes in cacommon.h)
 */
CAResult_t CAStartLEDiscoveryServer();

/**
 * @brief Sends data to the endpoint using the adapter connectivity.
 * Note: length must be > 0.
 * @param   endpoint    [IN]    Remote Endpoint information (like ipaddress , port, reference uri and connectivity type) to
 *                              which the unicast data has to be sent.
 * @param   data        [IN]    Data which required to be sent.
 * @param   dataLen     [IN]    Size of data to be sent.
 * @return - The number of bytes sent on the network. Return value equal to zero indicates error.
 */
uint32_t CASendLEUnicastData(const CARemoteEndpoint_t* endpoint, void* data, uint32_t dataLen);

/**
 * @brief Sends Multicast data to the endpoint using the LE connectivity.
 * Note: length must be > 0.
 * @param   data        [IN]    Data which required to be sent.
 * @param   dataLen     [IN]    Size of data to be sent.
 * @return - The number of bytes sent on the network. Return value equal to zero indicates error.
 */
uint32_t CASendLEMulticastData(void* data, uint32_t dataLen);

/**
 * @brief Starts notification server on EDR adapters.
 * @return CA_STATUS_OK or ERROR CODES ( CAResult_t error codes in cacommon.h)
 */
CAResult_t CAStartLENotifyServer();

/**
 * @brief Send notification information.
 * Note: length must be > 0.
 * @param   endpoint    [IN]    Remote Endpoint information (like ipaddress , port, reference uri and connectivity type) to
 *                              which the unicast data has to be sent.
 * @param   data        [IN]    Data which required to be sent.
 * @param   dataLen     [IN]    Size of data to be sent.
 * @return - The number of bytes sent on the network. Return value equal to zero indicates error.
 */
uint32_t CASendLENotification(const CARemoteEndpoint_t* endpoint, void* data, uint32_t dataLen);

/**
 * @brief Get LE Connectivity network information
 * @param   info        [OUT]   Local connectivity information structures
 * @param   size        [OUT]   Number of local connectivity structures.
 * @return CA_STATUS_OK or ERROR CODES ( CAResult_t error codes in cacommon.h)
 */
CAResult_t CAGetLEInterfaceInformation(CALocalConnectivityt_t** info, uint32_t* size);

/**
 * @brief Read Synchronous API callback.
 * @return CA_STATUS_OK or ERROR CODES ( CAResult_t error codes in cacommon.h)
 */
CAResult_t CAReadLEData();

/**
 * @brief Stopping the adapters and close socket connections
 *   LE Stops all GATT servers and close sockets.
 * @return CA_STATUS_OK or ERROR CODES ( CAResult_t error codes in cacommon.h)
 */
CAResult_t CAStopLE();

/**
 * @brief Terminate the LE connectivity adapter.
 * Configuration information will be deleted from further use
 */
void CATerminateLE();

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif //#ifndef _CA_LEADAPTER_H_
