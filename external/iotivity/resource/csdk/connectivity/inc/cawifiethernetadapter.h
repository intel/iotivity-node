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
 * @file cawifiethernetadapter.h
 * @brief This file contains the APIs for WiFi/Ethernet Adapter.
 */
#ifndef __CA_WIFI_ETHERNET_ADAPTER_H__
#define __CA_WIFI_ETHERNET_ADAPTER_H__

#include "cacommon.h"
#include "caadapterinterface.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief API to initialize WiFi Interface.
 * @param registerCallback to register WIFI interfaces to things communication
 * @param reqRespCallback for sending responses and discovery messages from unicast , multicast servers
 * @param netCallback to intimate the network additions to things communication
 * @return Error Code
 */
CAResult_t CAInitializeWifi(CARegisterConnectivityCallback registerCallback,
        CANetworkPacketReceivedCallback reqRespCallback, CANetworkChangeCallback netCallback);

/**
 * @brief API to Start WiFi Interface.
 * @return Error Code
 */
CAResult_t CAStartWIFI();

/**
 * @brief starting listening server
 * Transport Specific Behaviour
 *	 IP :- Start Multicast Server on  all available IPs and prefixed port number and as per OIC Specification.
 * @return Error Code CAResult_t.
 */
CAResult_t CAStartWIFIListeningServer();

/**
 * @brief for starting discovery servers.
 * Transport Specific Behaviour
 *	 IP :- Start Multicast Server on all available IPs and prefixed port number as per OIC Specification.
 * @return CAResult_t.
 * CA_STATUS_OK = 0 else ERROR CODES ( CAResult_t error codes in cacommon.h)
 */
CAResult_t CAStartWIFIDiscoveryServer();

/**
 * @brief Send Multicast Data interface API callback.
 * Note: length must be > 0
 * @param endpoint - Endpoint information (like ipaddress , port, reference uri and connectivity type) to which the multicast data has to be sent.
 * @param data - void pointer data which will be sent via client
 * @param dataLen - size of data to be sent
 * @return - Will return the number of bytes sent on the network. Return value equal to zero indicates error.
 */
uint32_t CASendWIFIUnicastData(const CARemoteEndpoint_t* endpoint, void* data, uint32_t dataLen);

/**
 * @brief API to send data to a group of remote addresses.
 * @param endpoint - Endpoint information (like ipaddress , port, reference uri and connectivity type) to which the multicast data has to be sent.
 * @param data - void pointer data which will be sent via client
 * @param dataLen - size of data to be sent
 * @return - size of data sent or -1 on error
 */
uint32_t CASendWIFIMulticastData(void* data, uint32_t dataLen);

/**
 * @brief Start Notification Server interface API callback.
 *
 * @return CAResult_t.
 * CA_STATUS_OK = 0 else ERROR CODES ( CAResult_t error codes in cacommon.h)
 */
CAResult_t CAStartWIFINotifyRecvServers();

/**
 * @brief Send Notification Data interface API callback.
 * Note: length must be > 0
 * @param endpoint - remote endpoint having endpoint details
 *  @param data - data to be sent
 *  @param dataLen - length of data to be sent
 *  @return Will return the number of bytes sent on the network. Return value equal to zero indicates error.
 */
uint32_t CASendWIFINotification(const CARemoteEndpoint_t* endpoint, void* data, uint32_t dataLen);
/**
 * @brief Get Network Information
 *
 * @return CAResult_t.
 * CA_STATUS_OK = 0 else ERROR CODES ( CAResult_t error codes in cacommon.h)
 */
CAResult_t CAGetWIFIInterfaceInformation(CALocalConnectivityt_t** info, uint32_t* size);

/**
 * @brief Read Synchronous API callback.
 * @return Error Code CAResult_t.
 * @return CAResult_t.
 * CA_STATUS_OK = 0 else ERROR CODES ( CAResult_t error codes in cacommon.h)
 */
CAResult_t CAReadWIFIData();

/**
 * @brief API to Stop WiFi Interface.
 * @return CAResult_t
 */
CAResult_t CAStopWIFI();

/**
 * @brief API to terminate WiFi Interface.
 * @return none.
 */
void CATerminateWifi();

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
