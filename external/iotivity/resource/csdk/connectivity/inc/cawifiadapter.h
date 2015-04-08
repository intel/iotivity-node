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
 * @file cawifiadapter.h
 * @brief This file contains the APIs for WiFi Adapter.
 */
#ifndef __CA_WIFI_ADAPTER_H__
#define __CA_WIFI_ADAPTER_H__

#include "cacommon.h"
#include "caadapterinterface.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief API to initialize WiFi Interface.
 * @param registerCallback [IN] To register WIFI interfaces to Connectivity Abstraction Layer
 * @param reqRespCallback [IN] sending responses and discovery messages from unicast , multicast servers
 * @param netCallback [IN] Intimate the network additions to Connectivity Abstraction Layer.
 * @return CA_STATUS_OK or ERROR CODES ( CAResult_t error codes in cacommon.h)
 */
CAResult_t CAInitializeWifi(CARegisterConnectivityCallback registerCallback,
        CANetworkPacketReceivedCallback reqRespCallback, CANetworkChangeCallback netCallback);

/**
 * @brief Start WiFi Interface adapter.
 * @return CA_STATUS_OK or ERROR CODES ( CAResult_t error codes in cacommon.h)
 */
CAResult_t CAStartWIFI();

/**
 * @brief Starting listening server for receiving multicast search requests
 * Transport Specific Behavior:
 *   WIFI Starts Multicast Server on  all available IPs and prefixed port number and as per OIC Specification.
 * @return CA_STATUS_OK or ERROR CODES ( CAResult_t error codes in cacommon.h)
 */
CAResult_t CAStartWIFIListeningServer();

/**
 * @brief for starting discovery servers for receiving multicast advertisements
 * Transport Specific Behavior:
 *   WIFI Starts Start multicast server on all available IPs and prefixed port number as per OIC Specification
 * @return CA_STATUS_OK or ERROR CODES ( CAResult_t error codes in cacommon.h)
 */
CAResult_t CAStartWIFIDiscoveryServer();

/**
 * @brief Sends data to the endpoint using the adapter connectivity.
 * Note: length must be > 0.
 * @param   endpoint    [IN]    Remote Endpoint information (like ipaddress , port, reference uri and connectivity type) to
 *                              which the unicast data has to be sent.
 * @param   data        [IN]    Data which required to be sent.
 * @param   dataLen     [IN]    Size of data to be sent.
 * @return - The number of bytes sent on the network. Return value equal to zero indicates error.
 */
uint32_t CASendWIFIUnicastData(const CARemoteEndpoint_t* endpoint, void* data, uint32_t dataLen);

/**
 * @brief Sends Multicast data to the endpoint using the WIFI connectivity.
 * Note: length must be > 0.
 * @param   data        [IN]    Data which required to be sent.
 * @param   dataLen     [IN]    Size of data to be sent.
 * @return - The number of bytes sent on the network. Return value equal to zero indicates error.
 */
uint32_t CASendWIFIMulticastData(void* data, uint32_t dataLen);

/**
 * @brief Starts notification server on WIFI adapters.
 * @return CA_STATUS_OK or ERROR CODES ( CAResult_t error codes in cacommon.h)
 */
CAResult_t CAStartWIFINotifyRecvServers();

/**
 * @brief Send notification information.
 * Note: length must be > 0.
 * @param   endpoint    [IN]    Remote Endpoint information (like ipaddress , port, reference uri and connectivity type) to
 *                              which the unicast data has to be sent.
 * @param   data        [IN]    Data which required to be sent.
 * @param   dataLen     [IN]    Size of data to be sent.
 * @return - The number of bytes sent on the network. Return value equal to zero indicates error.
 */
uint32_t CASendWIFINotification(const CARemoteEndpoint_t* endpoint, void* data, uint32_t dataLen);
/**
 * @brief Get WIFI Connectivity network information
 * @param   info        [OUT]   Local connectivity information structures
 * @param   size        [OUT]   Number of local connectivity structures.
 * @return CA_STATUS_OK or ERROR CODES ( CAResult_t error codes in cacommon.h)
 */
CAResult_t CAGetWIFIInterfaceInformation(CALocalConnectivityt_t** info, uint32_t* size);

/**
 * @brief Read Synchronous API callback.
 * @return CA_STATUS_OK or ERROR CODES ( CAResult_t error codes in cacommon.h)
 */
CAResult_t CAReadWIFIData();

/**
 * @brief Stopping the adapters and close socket connections
 *   WIFI Stops all multicast and unicast servers and close sockets.
 * @return CA_STATUS_OK or ERROR CODES ( CAResult_t error codes in cacommon.h)
 */
CAResult_t CAStopWIFI();

/**
 * @brief Terminate the WIFI connectivity adapter.
 * Configuration information will be deleted from further use
 */
void CATerminateWIfI();

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif//#ifndef __CA_WIFI_ADAPTER_H__
