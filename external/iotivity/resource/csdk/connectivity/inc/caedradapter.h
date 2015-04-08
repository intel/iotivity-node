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
 * @file caedradapter.h
 * @brief This file contains the APIs for EDR adapters to be implemented
 */
#ifndef __CA_EDRADAPTER_H_
#define __CA_EDRADAPTER_H_

/**
 * BT Interface AP
 **/
#include "cacommon.h"
#include "caadapterinterface.h"

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef BLUETOOTH_ADAPTER_TAG
#define BLUETOOTH_ADAPTER_TAG "CA_EDR"
#endif //BLUETOOTH_ADAPTER_TAG
/**
 * @brief Initialize EDR connectivity interface.
 * @param registerCallback [IN] To register EDR interfaces to Connectivity Abstraction Layer
 * @param reqRespCallback [IN] sending responses and discovery messages from unicast , multicast servers
 * @param netCallback [IN] Intimate the network additions to Connectivity Abstraction Layer.
 * @return CA_STATUS_OK or ERROR CODES ( CAResult_t error codes in cacommon.h)
 */
CAResult_t CAInitializeEDR(CARegisterConnectivityCallback registerCallback,
        CANetworkPacketReceivedCallback reqRespCallback, CANetworkChangeCallback netCallback);

/**
 * @brief Starting EDR connectivity adapters .As its peer to peer it doesnot require to start any servers
 * @return CA_STATUS_OK or ERROR CODES ( CAResult_t error codes in cacommon.h)
 */
CAResult_t CAStartEDR();

/**
 * @brief Starting listening server for receiving multicast search requests
 * Transport Specific Behavior:
 *   EDR  Starts RFCOMM Server with prefixed UUID as per specification.
 * @return CA_STATUS_OK or ERROR CODES ( CAResult_t error codes in cacommon.h)
 */
CAResult_t CAStartEDRListeningServer();

/**
 * @brief for starting discovery servers for receiving multicast advertisements
 * Transport Specific Behavior:
 *   EDR Starts RFCOMM server with prefixed UUID as per OIC Specification.
 * @return CA_STATUS_OK or ERROR CODES ( CAResult_t error codes in cacommon.h)
 */
CAResult_t CAStartEDRDiscoveryServer();

/**
 * @brief Sends data to the endpoint using the adapter connectivity.
 * Note: length must be > 0.
 * @param   endpoint    [IN]    Remote Endpoint information (like ipaddress , port, reference uri and connectivity type) to
 *                              which the unicast data has to be sent.
 * @param   data        [IN]    Data which required to be sent.
 * @param   dataLen     [IN]    Size of data to be sent.
 * @return - The number of bytes sent on the network. Return value equal to zero indicates error.
 */
uint32_t CASendEDRUnicastData(const CARemoteEndpoint_t* endpoint, void* data, uint32_t dataLen);

/**
 * @brief Sends Multicast data to the endpoint using the EDR connectivity.
 * Note: length must be > 0.
 * @param   data        [IN]    Data which required to be sent.
 * @param   dataLen     [IN]    Size of data to be sent.
 * @return - The number of bytes sent on the network. Return value equal to zero indicates error.
 */
uint32_t CASendEDRMulticastData(void* data, uint32_t dataLen);

/**
 * @brief Starts notification server on EDR adapters.
 * @return CA_STATUS_OK or ERROR CODES ( CAResult_t error codes in cacommon.h)
 */
CAResult_t CAStartEDRNotifyServer();

/**
 * @brief Send notification information.
 * Note: length must be > 0.
 * @param   endpoint    [IN]    Remote Endpoint information (like ipaddress , port, reference uri and connectivity type) to
 *                              which the unicast data has to be sent.
 * @param   data        [IN]    Data which required to be sent.
 * @param   dataLen     [IN]    Size of data to be sent.
 * @return - The number of bytes sent on the network. Return value equal to zero indicates error.
 */
uint32_t CASendEDRNotification(const CARemoteEndpoint_t* endpoint, void* data, uint32_t dataLen);

/**
 * @brief Get EDR Connectivity network information
 * @param   info        [OUT]   Local connectivity information structures
 * @param   size        [OUT]   Number of local connectivity structures.
 * @return CA_STATUS_OK or ERROR CODES ( CAResult_t error codes in cacommon.h)
 */
CAResult_t CAGetEDRInterfaceInformation(CALocalConnectivityt_t** info, uint32_t* size);

/**
 * @brief Read Synchronous API callback.
 * @return CA_STATUS_OK or ERROR CODES ( CAResult_t error codes in cacommon.h)
 */
CAResult_t CAReadEDRData();

/**
 * @brief Stopping the adapters and close socket connections
 *   EDR Stops all RFCOMM servers and close sockets.
 * @return CA_STATUS_OK or ERROR CODES ( CAResult_t error codes in cacommon.h)
 */
CAResult_t CAStopEDR();

/**
 * @brief Terminate the EDR connectivity adapter.
 * Configuration information will be deleted from further use
 */
void CATerminateEDR();

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  //__CA_EDRADAPTER_H_
