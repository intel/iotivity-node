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

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <fcntl.h>
#include <net/if.h>
#include <errno.h>

#ifdef __ANDROID__
#include <sys/ioctl.h>
#else
#include <ifaddrs.h>
#endif

#include <logger.h>
#include <ocsocket.h>

/// Module Name
#define MOD_NAME ("ocsocket")

/// Macro to verify the validity of input argument
#define VERIFY_NON_NULL(arg) { if (!arg) {OC_LOG(FATAL, MOD_NAME, #arg " is NULL"); return ERR_INVALID_INPUT;} }

/// Builds a socket interface address using IP address and port number
int32_t OCBuildIPv4Address(uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint16_t port, OCDevAddr *ipAddr)
{
    struct sockaddr_in *sa;
    uint32_t ip = a;

    VERIFY_NON_NULL(ipAddr);
    memset(ipAddr, 0, sizeof(OCDevAddr));

    ip <<= 8;
    ip |= b;
    ip <<= 8;
    ip |= c;
    ip <<= 8;
    ip |= d;

    ipAddr->size = sizeof(struct sockaddr_in);
    sa = (struct sockaddr_in*)ipAddr->addr;
    sa->sin_family = AF_INET;
    sa->sin_addr.s_addr = htonl(ip);
    sa->sin_port = htons(port);

    return ERR_SUCCESS;
}

#ifdef __ANDROID__
/// Retrieves the IP address assigned to specified wireless interface
int32_t OCGetInterfaceAddress(uint8_t* ifName, uint32_t ifNameLen, uint16_t addrType,
             uint8_t *addr,  uint32_t addrLen)
{
    int32_t ret = ERR_UNKNOWN;
    int32_t sfd = 0xFFFFFFFF;
    struct ifreq ifr;

    VERIFY_NON_NULL(addr);
    VERIFY_NON_NULL(ifName);
    if (ifNameLen > (IFNAMSIZ - 1) ) {
        return ERR_INVALID_INPUT;
    }
    if (addrType != AF_INET) {
        return ERR_INVALID_INPUT;
    }

    sfd = socket(addrType, SOCK_DGRAM, 0);
    if (sfd < 0) {
        OC_LOG_V(FATAL, MOD_NAME, "socket API ret val %d", sfd);
        goto exit;
    }

    ifr.ifr_addr.sa_family = addrType;

    strncpy(ifr.ifr_name, (const char*)ifName, ifNameLen);

    if (ioctl(sfd, SIOCGIFADDR, &ifr) != 0) {
        OC_LOG(FATAL, MOD_NAME, "ioctl call failed");
        goto exit;
    }

    strncpy((char *)addr,
            inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr),
            addrLen);
    ret = ERR_SUCCESS;

exit:
    if (sfd >= 0) {
        close(sfd);
    }
    return ret;
}

#else
/// Retrieves the IP address assigned to specified wireless interface
int32_t OCGetInterfaceAddress(uint8_t* ifName, uint32_t ifNameLen, uint16_t addrType,
             uint8_t *addr,  uint32_t addrLen)
{
    struct ifaddrs *myaddrs = NULL, *ifa = NULL;
    int32_t ret = ERR_UNKNOWN;

    VERIFY_NON_NULL(addr);

    if (addrType != AF_INET) {
        return ERR_INVALID_INPUT;
    }

    if(getifaddrs(&myaddrs) != 0) {
        goto exit;
    }

    for (ifa = myaddrs; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL)
            continue;
        if (!(ifa->ifa_flags & IFF_UP))
            continue;
        if (!(ifa->ifa_flags & IFF_RUNNING))
            continue;

        if (ifName && ifa->ifa_name) {
           if(strncmp((const char*)ifName, ifa->ifa_name, ifNameLen) != 0)
            continue;
        }

        switch (ifa->ifa_addr->sa_family)
        {
            case AF_INET:
                {
                    struct sockaddr_in *s4 = (struct sockaddr_in *)ifa->ifa_addr;
                    if(inet_ntop(AF_INET, &(s4->sin_addr), (char *)addr, addrLen))
                        ret = ERR_SUCCESS;
                    goto exit;
                }

            default:
                continue;
        }
    }

exit:
    if (myaddrs) {
        freeifaddrs(myaddrs);
    }
    return ret;
}
#endif //__ANDROID__

/// Creates a BSD socket and binds it specified port for UDP
int32_t OCInitUDP(OCDevAddr* ipAddr, int32_t *sockfd, OC_SOCKET_OPTION sockoption)
{
    int32_t ret = ERR_UNKNOWN;
    int32_t sfd = 0xFFFFFFFF;
    int set_option_on = 1;

    VERIFY_NON_NULL(ipAddr);
    VERIFY_NON_NULL(sockfd);

    OC_LOG_V(DEBUG, MOD_NAME, "%s Begin", __func__ );
    //Create a datagram socket on which to recv/send.
    sfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sfd < 0) {
        OC_LOG_V(FATAL, MOD_NAME, "socket API ret val %d", sfd);
        goto exit;
    }

    if(OC_SOCKET_REUSEADDR == sockoption)
    {
        if ((ret = setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, (char*) &set_option_on,
                    sizeof(set_option_on))) < 0) {
            OC_LOG_V(FATAL, MOD_NAME, "setsockopt API failed with errno %s",
                    strerror(errno));
            goto exit;
        }
    }

    if ((ret = bind(sfd, (struct sockaddr*)ipAddr->addr, ipAddr->size)) < 0) {
        OC_LOG_V(FATAL, MOD_NAME, "bind API failed with errno %s", strerror(errno));
        goto exit;
    }

    *sockfd = sfd;
    ret = ERR_SUCCESS;

exit:
    if ((ret != ERR_SUCCESS) && (sfd >= 0)) {
        close(sfd);
    }

    OC_LOG_V(DEBUG, MOD_NAME, "%s End", __func__ );
    return ret;
}



/// Creates a BSD socket and binds the specified port for UDP multicast.
int32_t OCInitUDPMulticast(OCDevAddr* ipmcastaddr, int32_t* sockfd)
{
    int32_t ret = ERR_UNKNOWN;
    int32_t sfd = 0xFFFFFFFF;
    char loopch=1;
    int set_option_on = 1;

    VERIFY_NON_NULL(ipmcastaddr);
    VERIFY_NON_NULL(sockfd);

    OC_LOG_V(DEBUG, MOD_NAME, "%s Begin", __func__ );
    //Create a datagram socket on which to recv/send.
    sfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sfd < 0) {
        OC_LOG_V(FATAL, MOD_NAME, "socket API ret val %d", sfd);
        goto exit;

    }

    if ((ret = setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, (char*) &set_option_on,
                sizeof(set_option_on))) < 0) {
        OC_LOG_V(FATAL, MOD_NAME, "setsockopt API for SO_REUSEADDR failed with errno %s", strerror(errno));
        goto exit;
    }

    // bind to multicast port
    struct sockaddr_in sa = {0};
    struct sockaddr_in *sin;

    sin = (struct sockaddr_in *)(ipmcastaddr->addr);
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = sin->sin_addr.s_addr;
    sa.sin_port = sin->sin_port;

    if ((ret = bind(sfd, (struct sockaddr*)&sa, sizeof(sa))) < 0) {
        OC_LOG_V(FATAL, MOD_NAME, "bind API failed with errno %s", strerror(errno));
        goto exit;
    }

    // add membership to receiving socket
    struct ip_mreq mreq = {{0}};
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    mreq.imr_multiaddr.s_addr = sin->sin_addr.s_addr;
    if ((ret = setsockopt(sfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *) &mreq, sizeof(mreq))) < 0) {
        OC_LOG_V(FATAL, MOD_NAME, "setsockopt API for IP_ADD_MEMBERSHIP failed with errno %s", strerror(errno));
        goto exit;
    }

    //Enable loopback so we can receive our own datagrams.
    if ((ret = setsockopt(sfd, IPPROTO_IP, IP_MULTICAST_LOOP,
                (char *)&loopch, sizeof(loopch))) < 0) {
        OC_LOG_V(FATAL, MOD_NAME, "setsockopt API for IP_MULTICAST_LOOP failed with errno %s", strerror(errno));
        goto exit;
    }

    ret = ERR_SUCCESS;

exit:
    if (ret == ERR_SUCCESS) {
        *sockfd = sfd;
    } else {
        close(sfd);
    }

    OC_LOG_V(DEBUG, MOD_NAME, "%s End", __func__ );
    return ret;
}



/// Send data to requested end-point using UDP socket
int32_t OCSendTo(int32_t sockfd, const uint8_t* buf, uint32_t bufLen, uint32_t flags,
            OCDevAddr * ipAddr)
{
    int32_t ret;

    VERIFY_NON_NULL(buf);
    VERIFY_NON_NULL(ipAddr);

    OC_LOG_V(DEBUG, MOD_NAME, "%s Begin", __func__ );
    ret = sendto(sockfd, buf, bufLen, flags,
            (struct sockaddr*)ipAddr->addr, ipAddr->size);

    OC_LOG_V(DEBUG, MOD_NAME, "%s End", __func__ );
    return ret;
}


/// Retrieve any available data from UDP socket. This is a non-blocking call.
int32_t OCRecvFrom(int32_t sockfd, uint8_t* buf, uint32_t bufLen, uint32_t flags,
            OCDevAddr * ipAddr)
{
    int32_t ret = 0;

    VERIFY_NON_NULL(buf);
    VERIFY_NON_NULL(ipAddr);

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 5000;
    fd_set reads;

    OC_LOG_V(DEBUG, MOD_NAME, "%s Begin", __func__ );

    FD_ZERO(&reads);
    FD_SET(sockfd, &reads);
    ret = select(sockfd + 1, &reads, NULL, NULL, &timeout);
    if( ret < 0) {
        OC_LOG(FATAL, MOD_NAME, "select API failed");
        return ret;
    }
    if (!FD_ISSET(sockfd, &reads)) {
        OC_LOG(DEBUG, MOD_NAME, "No data to read");
        return ERR_SUCCESS;
    }

    // Read available data.
    ret = recvfrom(sockfd, buf, bufLen, flags,
            (struct sockaddr*)ipAddr->addr, (socklen_t*)&(ipAddr->size));
    if (ret < 1) {
        OC_LOG(FATAL, MOD_NAME, "OCRecvFrom ERR");
    }
    OC_LOG_V(DEBUG, MOD_NAME, "%s End", __func__ );
    return ret;
}


/// Close the socket and release all system resources.
int32_t OCClose(int32_t sockfd)
{
    return (close(sockfd));
}

//convert OCDevAddr to String
int32_t OCDevAddrToString(OCDevAddr* addr, char* stringAddress)
{
    uint8_t a;
    uint8_t b;
    uint8_t c;
    uint8_t d;

    if(OCDevAddrToIPv4Addr(addr, &a, &b, &c, &d) == 0)
    {
        if (!stringAddress)
        {
            return ERR_INVALID_INPUT;
        }

        snprintf(stringAddress, DEV_ADDR_SIZE_MAX, "%u.%u.%u.%u",
                a, b, c, d);
        return ERR_SUCCESS;
    }
    else
    {
        return ERR_INVALID_INPUT;
    }
}

/// Retrieve the IPv4 address embedded inside OCDev address data structure
int32_t OCDevAddrToIPv4Addr(OCDevAddr *ipAddr, uint8_t *a, uint8_t *b,
            uint8_t *c, uint8_t *d )
{
    struct sockaddr_in *sa;
    uint32_t ip;

    if ( !ipAddr || !a || !b || !c || !d ) {
        OC_LOG(FATAL, MOD_NAME, "Invalid argument");
        return ERR_INVALID_INPUT;
    }

    sa = (struct sockaddr_in*)ipAddr->addr;
    ip = ntohl(sa->sin_addr.s_addr);
    *d = *((uint8_t*)&ip + 0);
    *c = *((uint8_t*)&ip + 1);
    *b = *((uint8_t*)&ip + 2);
    *a = *((uint8_t*)&ip + 3);

    return ERR_SUCCESS;
}


/// Retrieve the IPv4 address embedded inside OCDev address data structure
int32_t OCDevAddrToPort(OCDevAddr *ipAddr, uint16_t *port)
{
    struct sockaddr_in *sa;
    if ( !ipAddr || !port ) {
        OC_LOG(FATAL, MOD_NAME, "Invalid argument");
        return ERR_INVALID_INPUT;
    }

    sa = (struct sockaddr_in*)ipAddr->addr;
    *port = ntohs(sa->sin_port);

    return ERR_SUCCESS;
}

/// Retrieve the port to which socket is bound
int32_t OCGetSocketInfo(int32_t sockfd, uint16_t *port)
{
    int32_t ret = ERR_SUCCESS;

    struct sockaddr_in sa;
    socklen_t salen = sizeof(sa);
    if (getsockname(sockfd, (struct sockaddr*)&sa, &salen) == 0) {
        *port = ntohs(sa.sin_port);
    } else {
        OC_LOG_V(FATAL, MOD_NAME, "getsockname API failed with errno \
            %s", strerror(errno));
        ret = ERR_UNKNOWN;
    }
    return ret;
}
