//******************************************************************
///
// Copyright 2014 Intel Mobile Communications GmbH All Rights Reserved.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#ifndef _OCSOCKET_H
#define _OCSOCKET_H


#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**@mainpage
 *
 * This module is a part of Open Communication Thin-Block SDK.
 */


/**@defgroup socket Socket Interface
 *
 * This Socket interface  needs to be implemented for every platform on
 * which CCF TB stack is expected to run. If some functionality is not
 * available on a platform, implement the method by returning error
 * ERR_NOT_IMPLEMENTED.
 */

#define ERR_SUCCESS          (0)
#define ERR_INVALID_INPUT    (-900)
#define ERR_UNKNOWN          (-901)
#define ERR_NOT_IMPLEMENTED  (-903)


/** This would need to be modified for specific platforms and specific
 *  technologies
 */
#define DEV_ADDR_SIZE_MAX (16)

/**
 *IPv4 or IPv6 addresses
 */
#ifndef AF_INET
#define AF_INET (2)
#endif //AF_INET

#ifndef AF_INET6
#define AF_INET6 (10)
#endif //AF_INET6


/**
 * Data structure to encapsulate IPv4/IPv6/Contiki/lwIP device addresses
 *
*/
#pragma pack(push, 1)
typedef struct OCDevAddr {
    uint32_t     size;                    /**< length of the address stored in addr field. */
    uint8_t      addr[DEV_ADDR_SIZE_MAX]; /**< device address. */
}OCDevAddr;
#pragma pack(pop)

//-- OCInitNetworkStack -----------------------------------------------------------
/** @ingroup ocsocket
 *
 * This method is used to perform any platform specific network
 * initialization. Optional to implement.
 *
 * @retval 0 for Success, otherwise some error value
 */
//------------------------------------------------------------------------
int32_t OCInitNetworkStack();

typedef enum
{
    OC_SOCKET_NOOPTION = 0,
    OC_SOCKET_REUSEADDR
} OC_SOCKET_OPTION;

//-- OCInitUDP -----------------------------------------------------------
/** @ingroup ocsocket
 *
 * This method is used to create a new platform specific UDP socket and binds
 * it to the address provided.
 *
 * @param[in] ipAddr
 *              device address with which the new socket will be bind.
 * @param[out] sockfd
 *              reference to the new socket.
 * @param[in] sockoption
 *              specifies which socket option to be used.
 *
 * @retval 0 for Success, otherwise some error value
 */
//------------------------------------------------------------------------
int32_t OCInitUDP(OCDevAddr* ipAddr, int32_t* sockfd, OC_SOCKET_OPTION sockoption);



//-- OCInitUDPMulticast --------------------------------------------------
/** @ingroup ocsocket
 *
 * This method is used to create a new platform specific UDP socket for
 * multicast and and binds it to the address provided.
 *
 * @param[in] ipmcastaddr
 *              multicast address to which IGMP membership be added.
 * @param[out] sockfd
 *              reference to the new socket.
 *
 * @retval 0 for Success, otherwise some error value
 */
//------------------------------------------------------------------------
int32_t OCInitUDPMulticast(OCDevAddr* ipmcastaddr, int32_t* sockfd);



//-- OCSendTo -------------------------------------------------------------
/** @ingroup ocsocket
 *
 * This method is used to transmit a UDP datagram to another endpoint.
 *
 * @param[in] sockfd
 *              socket to be used for sending the datagram.
 * @param[in] buf
 *              datagram buffer.
 * @param[in] bufLen
 *              length of above buffer.
 * @param[in] flags
 *              flags to be used for sending datagram.
 * @param[in] addr
 *              endpoint to which datagram needs to be send.
 *
 * @retval On Success, it returns the number of bytes send, otherwise
 *          some negative value.
 */
//------------------------------------------------------------------------
int32_t OCSendTo(int32_t sockfd, const uint8_t* buf, uint32_t bufLen, uint32_t flags,
            OCDevAddr * addr);


//-- OCRecvFrom ------------------------------------------------------------
/** @ingroup ocsocket
 *
 * This method is used to retrieve a UDP datagram from the socket.
 *
 * @param[in] sockfd
 *              socket to be used for retrieving the datagram.
 * @param[in] buf
 *              datagram buffer.
 * @param[in] bufLen
 *              length of above buffer.
 * @param[in] flags
 *              flags to be used for receiving datagram.
 * @param[out] addr
 *              endpoint from which datagram has been received.
 *
 * @retval On Success, it returns the number of bytes read from the socket,
 *          otherwise some negative value.
 */
//------------------------------------------------------------------------
int32_t OCRecvFrom(int32_t sockfd, uint8_t* buf, uint32_t bufLen, uint32_t flags,
            OCDevAddr * addr);

//-- OCClose ---------------------------------------------------------------
/** @ingroup ocsocket
 *
 * This method is used to close the platform specific socket and release any
 * system resources associated with it.
 *
 * @param[in] sockfd
 *              socket to be closed.
 *
 * @retval 0 for Success, otherwise some error value
 */
//------------------------------------------------------------------------
int32_t OCClose(int32_t sockfd);


//Utility methods
//-- OCBuildIPv4Address -------------------------------------------------
/** @ingroup ocsocket
 *
 * This method is used to create the IPv4 dev_addr structure.
 *
 * @param[in]  a first byte of IPv4 address.
 * @param[in]  b second byte of IPv4 address.
 * @param[in]  c third byte of IPv4 address.
 * @param[in]  d fourth byte of IPv4 address.
 * @param[in]  port port number.
 * @param[out] ipAddr
 *              dev_addr to be filled with above data.
 *
 * @retval 0 for Success, otherwise some error value
 */
//------------------------------------------------------------------------
int32_t OCBuildIPv4Address(uint8_t a, uint8_t b, uint8_t c, uint8_t d,
            uint16_t port, OCDevAddr *ipAddr);


//-- OCGetInterfaceAddress ------------------------------------------------------
/** @ingroup ocsocket
 *
 * This method is used to retrieved the IPv4/IPv6 address of the local interface.
 * If no interface name is provided, this API retrieves the IP address of
 * the default interface.
 *
 * @note currently, only IPv4(AF_INET) is supported for addrType argument.
 *
 * @param[in]  ifName
 *              interface whose address needs to be retrieved.
 * @param[in]  ifNameLen
 *              length of the interface name
 * @param[in]  addrType
 *              IPv4 or IPv6
 * @param[out] addrv4
 *              IPv4 address in a.b.c.d format
 * @param[in]  addrLen
 *              size of the buffer at addrv4. Should be at least 16 bytes for an
 *              IPv4 address.
 *
 * @retval 0 for Success, otherwise some error value
 */
//-------------------------------------------------------------------------------
int32_t OCGetInterfaceAddress(uint8_t* ifName, uint32_t ifNameLen, uint16_t addrType,
             uint8_t *addrv4,  uint32_t addrLen);


//-- OCDevAddrToString ----------------------------------------------------
/** @ingroup ocsocket
 *
 * This method is used to convert the OCDevAddr to string format
 *
 * @param[in]   addr
 *               OCDevAddr address.
 * @param[out]  stringAddress the target string where the address
 *               is to be stored. Memory for this parameter is
 *               allocated by the caller.
 *
 * Note: The length of stringAddress may not exceed DEV_ADDR_SIZE_MAX
 *
 * @retval 0 for Success, otherwise some error value
 */
//------------------------------------------------------------------------
int32_t OCDevAddrToString(OCDevAddr *addr, char *stringAddress);


//-- OCDevAddrToIPv4Addr -------------------------------------------------
/** @ingroup ocsocket
 *
 * This method is used to retrieved the IPv4 address from OCDev address
 * data structure.
 *
 * @param[in]  ipAddr
 *              OCDevAddr address.
 * @param[out]  a first byte of IPv4 address.
 * @param[out]  b second byte of IPv4 address.
 * @param[out]  c third byte of IPv4 address.
 * @param[out]  d fourth byte of IPv4 address.
 *
 * @retval 0 for Success, otherwise some error value
 */
//------------------------------------------------------------------------
int32_t OCDevAddrToIPv4Addr(OCDevAddr *ipAddr, uint8_t *a, uint8_t *b,
            uint8_t *c, uint8_t *d );


//-- OCDevAddrToPort -------------------------------------------------
/** @ingroup ocsocket
 *
 * This method is used to retrieve the port number from OCDev address
 * data structure.
 *
 * @param[in]  ipAddr
 *              OCDevAddr address.
 * @param[out] port
 *              port number
 *
 * @retval 0 for Success, otherwise some error value
 */
//------------------------------------------------------------------------
int32_t OCDevAddrToPort(OCDevAddr *ipAddr, uint16_t *port);


//-- OCGetSocketInfo -----------------------------------------------------
/** @ingroup ocsocket
 *
 * This method is used to retrieve the port number to which the @p sockfd
 * is bound.
 *
 * @param[in]  sockfd
 *              socket whose port needs to be retrieved
 * @param[out] port
 *              port number
 *
 * @retval 0 for Success, otherwise some error value
 */
//------------------------------------------------------------------------
int32_t OCGetSocketInfo(int32_t sockfd, uint16_t *port);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif //_OCSOCKET_H
