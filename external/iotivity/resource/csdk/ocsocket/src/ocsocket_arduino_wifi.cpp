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

#include <logger.h>
#include <ocsocket.h>
#include <Arduino.h>
#include <IPAddress.h>
#include <WiFi.h>
#include <utility/server_drv.h>
#include <utility/wifi_drv.h>

/// Module Name
#define MOD_NAME PCF("ocsocket")

/// Macro to verify the validity of input argument
#define VERIFY_NON_NULL(arg) { if (!arg) {OC_LOG_V(FATAL, MOD_NAME, "%s is NULL", #arg); \
         return ERR_INVALID_INPUT;} }

/// Length of the IP address decimal notation string
#define IPNAMESIZE (16)

/// This is the max buffer size between Arduino and WiFi Shield
#define ARDUINO_WIFI_SPI_RECV_BUFFERSIZE (64)

// Start offsets based on end of received data buffer
#define WIFI_RECBUF_IPADDR_OFFSET  (6)
#define WIFI_RECBUF_PORT_OFFSET    (2)

#define WIFI_RECBUF_IPADDR_SIZE    (WIFI_RECBUF_IPADDR_OFFSET - WIFI_RECBUF_PORT_OFFSET)
#define WIFI_RECBUF_PORT_SIZE      (WIFI_RECBUF_PORT_OFFSET - 0)
#define WIFI_RECBUF_FOOTER_SIZE    (WIFI_RECBUF_IPADDR_SIZE + WIFI_RECBUF_PORT_SIZE)


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
    WiFiClass WiFi;

    VERIFY_NON_NULL(addr);
    if (addrLen < IPNAMESIZE) {
        OC_LOG(FATAL, MOD_NAME, PCF("OCGetInterfaceAddress: addrLen MUST be at least 16"));
        return ERR_INVALID_INPUT;
    }

    if (addrType != AF_INET) {
        return ERR_INVALID_INPUT;
    }

    IPAddress ip = WiFi.localIP();
    snprintf((char *)addr, addrLen, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);

    OC_LOG_BUFFER(INFO, MOD_NAME, addr, addrLen);

    return ERR_SUCCESS;
}

/// Retrieves a empty socket and bind it for UDP with the input port
int32_t OCInitUDP(OCDevAddr* ipAddr, int32_t* sockfd, OC_SOCKET_OPTION sockoption)
{
    ArduinoAddr* ardAddr = (ArduinoAddr*)ipAddr;
    uint8_t sock;

    VERIFY_NON_NULL(ardAddr);
    VERIFY_NON_NULL(sockfd);

    OC_LOG(DEBUG, MOD_NAME, PCF("OCInitUDP Begin"));
    //Is any socket available to work with ?
    *sockfd = -1;

    sock = WiFiClass::getSocket();
    if (sock != NO_SOCKET_AVAIL)
    {
        ServerDrv::startServer(ardAddr->port, sock, UDP_MODE);
        WiFiClass::_server_port[sock] = ardAddr->port;
        *sockfd = (int32_t)sock;
    }

    if (*sockfd == -1)
    {
        return ERR_UNKNOWN;
    }

    OC_LOG(DEBUG, MOD_NAME, PCF("OCInitUDP End"));
    return ERR_SUCCESS;
}



/// Currently WiFi shield does NOT support multicast.
int32_t OCInitUDPMulticast(OCDevAddr* ipMcastMacAddr, int32_t* sockfd)
{
    return OCInitUDP(ipMcastMacAddr, sockfd, OC_SOCKET_REUSEADDR);
}


/// Send data to requested end-point using UDP socket
int32_t OCSendTo(int32_t sockfd, const uint8_t* buf, uint32_t bufLen, uint32_t flags,
            OCDevAddr * ipAddr)
{
    int32_t ret = 0;
    ArduinoAddr* ardAddr = (ArduinoAddr*)ipAddr;
    uint32_t ip;
    uint16_t rem, send;

    VERIFY_NON_NULL(buf);
    VERIFY_NON_NULL(ardAddr);
    OC_LOG(DEBUG, MOD_NAME, PCF("OCSendTo Begin"));

    if (sockfd >= MAX_SOCK_NUM)
    {
        OC_LOG(ERROR, MOD_NAME, PCF("Invalid sockfd"));
        return -1;
    }

    memcpy((uint8_t*)&ip, (uint8_t*)&(ardAddr->a), sizeof(ip));
    ServerDrv::startClient(ip, ardAddr->port, (uint8_t)sockfd, UDP_MODE);

    rem = bufLen;
    do
    {
        send = (rem > ARDUINO_WIFI_SPI_RECV_BUFFERSIZE ) ? ARDUINO_WIFI_SPI_RECV_BUFFERSIZE : rem;
        if (!ServerDrv::insertDataBuf((uint8_t)sockfd, buf, (uint16_t)send))
        {
            OC_LOG(ERROR, MOD_NAME, PCF("insertDataBuf error"));
            ret = -1;
            break;
        }
        rem = rem - send;
        buf = buf + send;
    }while(rem > 0);

    if (ret != -1)
    {
        if (!ServerDrv::sendUdpData((uint8_t)sockfd))
        {
            OC_LOG(ERROR, MOD_NAME, PCF("sendUdpData error"));
            ret = -1;
        }
        else
        {
            ret = bufLen;
        }
    }
    OC_LOG(DEBUG, MOD_NAME, PCF("OCSendTo End"));
    return ret;
}


/// Retrieve any available data from UDP socket. This is a non-blocking call.
int32_t OCRecvFrom(int32_t sockfd, uint8_t* buf, uint32_t bufLen, uint32_t flags,
            OCDevAddr * ipAddr)
{

    uint16_t recvLen;
    uint16_t size = 0;
    ArduinoAddr* ardAddr = (ArduinoAddr*)ipAddr;

    VERIFY_NON_NULL(buf);
    VERIFY_NON_NULL(ardAddr);

    OC_LOG(DEBUG, MOD_NAME, PCF("OCRecvFrom Begin"));
    if (sockfd >= MAX_SOCK_NUM)
    {
        OC_LOG(ERROR, MOD_NAME, PCF("Invalid sockfd"));
        return -1;
    }

    recvLen = (int32_t)ServerDrv::availData((uint8_t)sockfd);
    if (recvLen == 0)
    {
        return recvLen;
    }

    // Make sure buf is large enough for received data
    if ((uint32_t)recvLen > bufLen)
    {
        OC_LOG(ERROR, MOD_NAME, PCF("Receive buffer too small"));
        return -1;
    }

    if (!ServerDrv::getDataBuf((uint8_t)sockfd, buf, &size))
    {
        OC_LOG(ERROR, MOD_NAME, PCF("getDataBuf error"));
        return -1;
    }

    // Read IP Address and Port from end of receive buffer
    memcpy(&(ardAddr->a), &buf[size - WIFI_RECBUF_IPADDR_OFFSET], WIFI_RECBUF_IPADDR_SIZE);
    // Change the endianness of the port number
    *((uint8_t*)&(ardAddr->port)) = buf[size - (WIFI_RECBUF_PORT_OFFSET-1)];
    *((uint8_t*)&(ardAddr->port) + 1) = buf[size - (WIFI_RECBUF_PORT_OFFSET)];

    size -= WIFI_RECBUF_FOOTER_SIZE;

    ardAddr->size =  sizeof(ArduinoAddr) - sizeof(ardAddr->size);
    OC_LOG(DEBUG, MOD_NAME, PCF("OCRecvFrom End"));
    return (int32_t)size;
}


/// Close the socket and release all system resources.
int32_t OCClose(int32_t sockfd)
{
    if (sockfd >= MAX_SOCK_NUM)
    {
        OC_LOG(ERROR, MOD_NAME, PCF("Invalid sockfd"));
        return -1;
    }
    ServerDrv::stopClient(sockfd);

    WiFiClass::_server_port[sockfd] = 0;
    WiFiClass::_state[sockfd] = NA_STATE;

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
