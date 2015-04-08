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
 * @file caethernetadapter.h
 * @brief This file contains the APIs for Ethernet Adapter.
 */
#ifndef __CA_ETHERNET_ADAPTER_H__
#define __CA_ETHERNET_ADAPTER_H__

#include "cacommon.h"
#include "caadapterinterface.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief Initialize Ethernet connectivity interface.
 * @param registerCallback [IN] To register Ethernet interfaces to Connectivity Abstraction Layer
 * @param reqRespCallback [IN] sending responses and discovery messages from unicast , multicast servers
 * @param netCallback [IN] Intimate the network additions to Connectivity Abstraction Layer.
 * @return CA_STATUS_OK or ERROR CODES ( CAResult_t error codes in cacommon.h)
 */
CAResult_t CAInitializeEthernet(CARegisterConnectivityCallback registerCallback,
        CANetworkPacketReceivedCallback reqRespCallback, CANetworkChangeCallback netCallback);

/**
 * @brief Start Ethernet Interface adapter.
 * @return CA_STATUS_OK or ERROR CODES ( CAResult_t error codes in cacommon.h)
 */
CAResult_t CAStartEthernet();

/**
 * @brief Starting listening server for receiving multicast search requests
 * Transport Specific Behavior:
 *   Ethernet Starts Multicast Server on  all available IPs and prefixed port number and as per OIC Specification.
 * @return CA_STATUS_OK or ERROR CODES ( CAResult_t error codes in cacommon.h)
 */
CAResult_t CAStartEthernetListeningServer();

/**
 * @brief for starting discovery servers for receiving multicast advertisements
 * Transport Specific Behavior:
 *  Ethernet Starts Start multicast server on all available IPs and prefixed port number as per OIC Specification
 * @return CA_STATUS_OK or ERROR CODES ( CAResult_t error codes in cacommon.h)
 */
CAResult_t CAStartEthernetDiscoveryServer();

/**
 * @brief Sends data to the endpoint using the adapter connectivity.
 * Note: length must be > 0.
 * @param   endpoint    [IN]    Remote Endpoint information (like ipaddress , port, reference uri and connectivity type) to
 *                              which the unicast data has to be sent.
 * @param   data        [IN]    Data which required to be sent.
 * @param   dataLen     [IN]    Size of data to be sent.
 * @return - The number of bytes sent on the network. Return value equal to zero indicates error.
 */
uint32_t CASendEthernetUnicastData(const CARemoteEndpoint_t* endpoint, void* data,
        uint32_t dataLen);

/**
 * @brief Sends Multicast data to the endpoint using the WIFI connectivity.
 * Note: length must be > 0.
 * @param   data        [IN]    Data which required to be sent.
 * @param   dataLen     [IN]    Size of data to be sent.
 * @return - The number of bytes sent on the network. Return value equal to zero indicates error.
 */
uint32_t CASendEthernetMulticastData(void* data, uint32_t dataLen);

/**
 * @brief Starts notification server on Ethernet adapters.
 * @return CA_STATUS_OK or ERROR CODES ( CAResult_t error codes in cacommon.h)
 */
CAResult_t CAStartEthernetNotifyServer();

/**
 * @brief Send notification information.
 * Note: length must be > 0.
 * @param   endpoint    [IN]    Remote Endpoint information (like ipaddress , port, reference uri and connectivity type) to
 *                              which the unicast data has to be sent.
 * @param   data        [IN]    Data which required to be sent.
 * @param   dataLen     [IN]    Size of data to be sent.
 * @return - The number of bytes sent on the network. Return value equal to zero indicates error.
 */
uint32_t CASendEthernetNotification(const CARemoteEndpoint_t* endpoint, void* data,
        uint32_t dataLen);
/**
 * @brief Get Ethernet Connectivity network information
 * @param   info        [OUT]   Local connectivity information structures
 * @param   size        [OUT]   Number of local connectivity structures.
 * @return CA_STATUS_OK or ERROR CODES ( CAResult_t error codes in cacommon.h)
 */
CAResult_t CAGetEthernetInterfaceInformation(CALocalConnectivityt_t** info, uint32_t* size);

/**
 * @brief Read Synchronous API callback.
 * @return CA_STATUS_OK or ERROR CODES ( CAResult_t error codes in cacommon.h)
 */
CAResult_t CAReadEthernetData();

/**
 * @brief Stopping the adapters and close socket connections
 *   Ethernet Stops all multicast and unicast servers and close sockets.
 * @return CA_STATUS_OK or ERROR CODES ( CAResult_t error codes in cacommon.h)
 */
CAResult_t CAStopEthernet();

/**
 * @brief Terminate the Ethernet connectivity adapter.
 * Configuration information will be deleted from further use
 */
void CATerminateEthernet();

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
