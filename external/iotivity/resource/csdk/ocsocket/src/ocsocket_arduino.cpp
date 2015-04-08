//******************************************************************
//
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

#include <Arduino.h>
#include <Ethernet.h>
#include <socket.h>
#include <w5100.h>
#include <ocsocket.h>
#include <EthernetUdp.h>
#include <IPAddress.h>
#include <logger.h>

/// Ensures the literal string to be stored in Flash memory
#define PCF(str) ((PROGMEM const char *)(F(str)))

/// Module Name
#define MOD_NAME PCF("ocsocket")

/// Macro to verify the validity of input argument
#define VERIFY_NON_NULL(arg) { if (!arg) {OC_LOG_V(FATAL, MOD_NAME, "%s is NULL", #arg); \
         return ERR_INVALID_INPUT;} }

/// Length of the IP address decimal notation string
#define IPNAMESIZE (16)

/// IPv4 address representation for Arduino Ethernet Shield
typedef struct {
    uint32_t size;  /// size of IP address and port bytes
    uint8_t a;
    uint8_t b;
    uint8_t c;
    uint8_t d;
    uint16_t port;
} ArduinoAddr;


/// Builds a socket interface address using IP address and port number
int32_t OCBuildIPv4Address(uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint16_t port, OCDevAddr *ipAddr)
{
    ArduinoAddr* ardAddr = (ArduinoAddr*)ipAddr;

    VERIFY_NON_NULL(ardAddr);

    memset(ardAddr, 0, sizeof(ArduinoAddr));

    ardAddr->size =  sizeof(ArduinoAddr) - sizeof(ardAddr->size);
    ardAddr-> a = a;
    ardAddr-> b = b;
    ardAddr-> c = c;
    ardAddr-> d = d;
    ardAddr-> port = port;

    return ERR_SUCCESS;
}


/// Retrieves the IP address assigned to Arduino Ethernet shield
int32_t OCGetInterfaceAddress(uint8_t* ifName, uint32_t ifNameLen, uint16_t addrType,
             uint8_t *addr,  uint32_t addrLen)
{
    //TODO : Fix this for scenarios when this API is invoked when device is not connected
    uint8_t rawIPAddr[4];
    VERIFY_NON_NULL(addr);
    if (addrLen < IPNAMESIZE) {
        OC_LOG(FATAL, MOD_NAME, PCF("OCGetInterfaceAddress: addrLen MUST be atleast 16"));
        return ERR_INVALID_INPUT;
    }

    if (addrType != AF_INET) {
        return ERR_INVALID_INPUT;
    }
    W5100.getIPAddress(rawIPAddr);
    snprintf((char *)addr, addrLen, "%d.%d.%d.%d", rawIPAddr[0], rawIPAddr[1], rawIPAddr[2], rawIPAddr[3]);

    OC_LOG_BUFFER(INFO, MOD_NAME, addr, addrLen);

    return ERR_SUCCESS;
}

/// Retrieves a empty socket and bind it for UDP with the input port
int32_t OCInitUDP(OCDevAddr* ipAddr, int32_t* sockfd, OC_SOCKET_OPTION sockoption)
{
    uint8_t state;
    ArduinoAddr* ardAddr = (ArduinoAddr*)ipAddr;

    VERIFY_NON_NULL(ardAddr);
    VERIFY_NON_NULL(sockfd);

    OC_LOG(DEBUG, MOD_NAME, PCF("OCInitUDP Begin"));
    //Is any socket available to work with ?
    *sockfd = -1;
    for (int i = 0; i < MAX_SOCK_NUM; i++) {
        state = W5100.readSnSR(i);
        if (state == SnSR::CLOSED || state == SnSR::FIN_WAIT) {
            *sockfd = i;
            break;
        }
    }

    if ( *sockfd == -1) {
        return ERR_UNKNOWN;
    }

    //Create a datagram socket on which to recv/send.
    if (!socket(*sockfd, SnMR::UDP, ardAddr->port, 0)) {
        return ERR_UNKNOWN;
    }

    OC_LOG(DEBUG, MOD_NAME, PCF("OCInitUDP End"));
    return ERR_SUCCESS;
}



/// Retrieves a empty socket and bind it for UDP with the input multicast ip address/port
int32_t OCInitUDPMulticast(OCDevAddr* ipMcastMacAddr, int32_t* sockfd)
{
    uint8_t state;
    uint8_t mcastMacAddr[] = { 0x01, 0x00, 0x5E, 0x00, 0x00, 0x00};
    ArduinoAddr* ardAddr = (ArduinoAddr*)ipMcastMacAddr;

    VERIFY_NON_NULL(ardAddr);
    VERIFY_NON_NULL(sockfd);

    OC_LOG(DEBUG, MOD_NAME, PCF("OCInitUDPMulticast Begin"));
    //Is any socket available to work with ?
    *sockfd = -1;
    for (int i = 0; i < MAX_SOCK_NUM; i++) {
        state = W5100.readSnSR(i);
        if (state == SnSR::CLOSED || state == SnSR::FIN_WAIT) {
            *sockfd = i;
            break;
        }
    }

    if ( *sockfd == -1) {
        return ERR_UNKNOWN;
    }

    //Calculate Multicast MAC address
    mcastMacAddr[3] = ardAddr->b & 0x7F;
    mcastMacAddr[4] = ardAddr->c;
    mcastMacAddr[5] = ardAddr->d;
    W5100.writeSnDIPR(*sockfd, (uint8_t*)&(ardAddr->a));
    W5100.writeSnDHAR(*sockfd, mcastMacAddr);
    W5100.writeSnDPORT(*sockfd, ardAddr->port);

    //Create a datagram socket on which to recv/send.
    if (!socket(*sockfd, SnMR::UDP, ardAddr->port, SnMR::MULTI)) {
        return ERR_UNKNOWN;
    }

    OC_LOG(DEBUG, MOD_NAME, PCF("OCInitUDPMulticast End"));
    return ERR_SUCCESS;
}


/// Send data to requested end-point using UDP socket
int32_t OCSendTo(int32_t sockfd, const uint8_t* buf, uint32_t bufLen, uint32_t flags,
            OCDevAddr * ipAddr)
{
    int32_t ret;
    ArduinoAddr* ardAddr = (ArduinoAddr*)ipAddr;

    VERIFY_NON_NULL(buf);
    VERIFY_NON_NULL(ardAddr);
    OC_LOG(DEBUG, MOD_NAME, PCF("OCSendTo Begin"));
    ret = sendto( sockfd, buf, bufLen, (uint8_t*)&(ardAddr->a), ardAddr->port);
    OC_LOG_V(DEBUG, MOD_NAME, "OCSendTo RetVal %d", ret);
    return ret;
}


/// Retrieve any available data from UDP socket. This is a non-blocking call.
int32_t OCRecvFrom(int32_t sockfd, uint8_t* buf, uint32_t bufLen, uint32_t flags,
            OCDevAddr * ipAddr)
{
    /**Bug : When there are multiple UDP packets in Wiznet buffer, W5100.getRXReceivedSize
     * will not return correct length of the first packet.
     * Fix : Use the patch provided for arduino/libraries/Ethernet/utility/socket.cpp
     */
    int32_t ret = 0;
    uint16_t recvLen;
    ArduinoAddr* ardAddr = (ArduinoAddr*)ipAddr;

    VERIFY_NON_NULL(buf);
    VERIFY_NON_NULL(ardAddr);

    OC_LOG(DEBUG, MOD_NAME, PCF("OCRecvFrom Begin"));
    recvLen = W5100.getRXReceivedSize(sockfd);
    if (recvLen == 0) {
        return recvLen;
    }

    // Read available data.
    ret = recvfrom(sockfd, buf, bufLen, (uint8_t*)&(ardAddr->a), (uint16_t*)&(ardAddr->port));
    ardAddr->size =  sizeof(ArduinoAddr) - sizeof(ardAddr->size);

    OC_LOG(DEBUG, MOD_NAME, PCF("OCRecvFrom End"));
    return ret;
}


/// Close the socket and release all system resources.
int32_t OCClose(int32_t sockfd)
{
    close(sockfd);
    return ERR_SUCCESS;
}


/// Retrieve the IPv4 address embedded inside OCDev address data structure
int32_t OCDevAddrToIPv4Addr(OCDevAddr *ipAddr, uint8_t *a, uint8_t *b,
            uint8_t *c, uint8_t *d )
{
    ArduinoAddr* ardAddr = (ArduinoAddr*)ipAddr;

    if ( !ardAddr || !a || !b || !c || !d ) {
        OC_LOG(FATAL, MOD_NAME, PCF("Invalid argument"));
        return ERR_INVALID_INPUT;
    }

    *a = ardAddr->a;
    *b = ardAddr->b;
    *c = ardAddr->c;
    *d = ardAddr->d;

    return ERR_SUCCESS;
}


/// Retrieve the IPv4 address embedded inside OCDev address data structure
int32_t OCDevAddrToPort(OCDevAddr *ipAddr, uint16_t *port)
{
    ArduinoAddr* ardAddr = (ArduinoAddr*)ipAddr;

    if ( !ardAddr || !port ) {
        OC_LOG(FATAL, MOD_NAME, PCF("Invalid argument"));
        return ERR_INVALID_INPUT;
    }

    *port = ardAddr->port;

    return ERR_SUCCESS;
}

/// Retrieve the port to which socket is bound
int32_t OCGetSocketInfo(int32_t sockfd, uint16_t *port)
{
    return ERR_NOT_IMPLEMENTED;
}
