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
 * @file cacommon.h
 * @brief This file contains the common data structures between Resource , CA and adapters
 */
#ifndef __CA_COMMON_H_
#define __CA_COMMON_H_

#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 @brief IP Address Length
 */
#define CA_IPADDR_SIZE 16

/**
 @brief Mac Address Length for BT
 */
#define CA_MACADDR_SIZE 18

/**
 @brief Max Header Options data length
 */
#define CA_MAX_HEADER_OPTION_DATA_LENGTH 16

/**
 @brief CAPayload_t
 */
typedef char* CAPayload_t;

/**
 @brief CAURI_t
 */
typedef char* CAURI_t;

/**
 @brief CAToken_t
 */
typedef char* CAToken_t;

/**
 @brief CABool_t
 */
typedef enum
{
    CA_FALSE = 0, CA_TRUE
} CABool_t;

/**
 @brief CAConnectivityType_t
 */
typedef enum
{
    CA_ETHERNET = (1 << 0), CA_WIFI = (1 << 1), CA_EDR = (1 << 2), CA_LE = (1 << 3)
} CAConnectivityType_t;

/**
 @brief CANetworkStatus_t
 */
typedef enum
{
    CA_INTERFACE_UP, CA_INTERFACE_DOWN
} CANetworkStatus_t;

/**
 @brief  Address of the local or remote endpoint
 */
typedef union
{
    /**
     @brief BT Information
     */
    struct
    {
        /** @brief Bluettoth Mac Address **/
        char btMacAddress[CA_MACADDR_SIZE];
    } BT;
    /**
     @brief IP Information
     */
    struct
    {
        /** Ip address of the interface**/
        char ipAddress[CA_IPADDR_SIZE];
        /** port information**/
        uint32_t port;
    } IP;
} CAAddress_t;

/**
 @brief CAQualityOfService
 */
typedef enum
{
    CA_LOW_QOS = 0, CA_MEDIUM_QOS, CA_HIGH_QOS, CA_NA_QOS // No Quality is defined, let the stack decide
} CAQualityOfService_t;

/**
 @brief CAMethod_t
 */
typedef enum
{
    CA_GET = 1, CA_POST, CA_PUT, CA_DELETE
} CAMethod_t;

/**
 @brief RemoteEndpoint information for connectivities
 */
typedef struct
{
    /** Resource URI information **/
    CAURI_t resourceUri;
    /** Remote Endpoint address **/
    CAAddress_t addressInfo;
    /** Connectivity of the endpoint**/
    CAConnectivityType_t connectivityType;
} CARemoteEndpoint_t;

/**
 @brief Local Connectivity information
 */
typedef struct
{
    /** address of the interface  **/
    CAAddress_t addressInfo;
    /** Connectivity type that localconnectivity avaialble **/
    CAConnectivityType_t type;
} CALocalConnectivityt_t;

/**
 @brief Enums for CA return values
 */
typedef enum
{
    /* Success status code - START HERE */
    CA_STATUS_OK = 0,
    CA_STATUS_INVALID_PARAM,
    CA_DESTINATION_NOT_REACHABLE,
    CA_SEND_FAILED,
    CA_RECEVIE_FAILED,
    CA_MEMORY_ALLOC_FAILED,
    CA_REQUEST_TIMEOUT,
    CA_DESTINATION_DISCONNECTED,
    CA_STATUS_FAILED,
    CA_NOT_SUPPORTED
/* Error status code - END HERE */
} CAResult_t;

/**
 @brief Enums for CA Response  values
 */
typedef enum
{
    /* Success status code - START HERE */
    CA_CREATED = 201,
    CA_DELETED = 202,
    CA_VALID = 203,
    CA_CONTENT = 205,
    CA_BAD_REQ = 400,
    CA_BAD_OPT = 402,
    CA_NOT_FOUND = 404
/* Error status code - END HERE */
} CAResponseResult_t;

/**
 @brief Transport Protocol IDs
 */
typedef enum
{
    CA_INVALID_ID = (1 << 0), CA_COAP_ID = (1 << 1)
} CATransportProtocolID_t;

/**
 * @brief Header options structure to be filled
 *
 * This structure is used to hold header information.
 */
typedef struct
{
    /** The protocol ID this option applies to**/
    CATransportProtocolID_t protocolID;
    /** The header option ID which will be added to communication packets**/
    uint16_t optionID;
    /** its length   191**/
    uint16_t optionLength;
    /** optional data values**/
    uint8_t optionData[CA_MAX_HEADER_OPTION_DATA_LENGTH];
} CAHeaderOption_t;

/**
 * @brief Request Information to be sent
 *
 * This structure is used to hold request information
 */

/**
 * @brief Base Information received
 *
 * This structure is used to hold request & response base information
 */
typedef struct
{
    /**Qos for the request **/
    CAQualityOfService_t qos;
    /** Token for CA**/
    CAToken_t token;
    /** Header Options for the request **/
    CAHeaderOption_t * options;
    /** Number of Header options**/
    uint8_t numOptions;
    /** payload of the request **/
    CAPayload_t payload;
} CAInfo_t;

typedef struct
{
    /** Name of the Method Allowed **/
    CAMethod_t method;
    /** Base Information **/
    CAInfo_t info;
} CARequestInfo_t;

/**
 * @brief Response Information received
 *
 * This structure is used to hold response information
 */
typedef struct
{
    /**Response Result by Resource Model**/
    CAResponseResult_t result;
    /**Base Information **/
    CAInfo_t info;
} CAResponseInfo_t;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif //#ifndef __CA_COMMON_H_
