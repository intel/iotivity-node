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


#include <gtest/gtest.h>
#include <ocsocket.h>
#include <logger.h>

#define MOD_NAME ("ocsocket_test")

#define TEST_PORT_NUM (8888)

unsigned char buf1[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0b, 0x5f, 0x67, 0x6f, 0x6f, 0x67,
  0x6c, 0x65, 0x63, 0x61, 0x73, 0x74, 0x04, 0x5f, 0x74, 0x63, 0x70, 0x05,
  0x6c, 0x6f, 0x63, 0x61, 0x6c, 0x00, 0x00, 0x0c, 0x00, 0x01
};
unsigned int buf1_len = sizeof(buf1);


unsigned char buf2[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x0b, 0x5f, 0x67, 0x6f,
  0x6f, 0x67, 0x6c, 0x65, 0x63, 0x61, 0x73, 0x74,
  0x04, 0x5f, 0x74, 0x63, 0x70, 0x05, 0x6c, 0x6f,
  0x63, 0x61, 0x6c, 0x00, 0x00, 0x0c, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x0b, 0x5f, 0x67, 0x6f,
  0x6f, 0x67, 0x6c, 0x65, 0x63, 0x61, 0x73, 0x74,
  0x04, 0x5f, 0x74, 0x63, 0x70, 0x05, 0x6c, 0x6f,
  0x63, 0x61, 0x6c, 0x00, 0x00, 0x0c, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x0b, 0x5f, 0x67, 0x6f,
  0x6f, 0x67, 0x6c, 0x65, 0x63, 0x61, 0x73, 0x74,
  0x04, 0x5f, 0x74, 0x63, 0x70, 0x05, 0x6c, 0x6f,
  0x63, 0x61, 0x6c, 0x00, 0x00, 0x0c, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x0b, 0x5f, 0x67, 0x6f,
  0x6f, 0x67, 0x6c, 0x65, 0x63, 0x61, 0x73, 0x74,
  0x04, 0x5f, 0x74, 0x63, 0x70, 0x05, 0x6c, 0x6f,
  0x63, 0x61, 0x6c, 0x00, 0x00, 0x0c, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x0b, 0x5f, 0x67, 0x6f,
  0x6f, 0x67, 0x6c, 0x65, 0x63, 0x61, 0x73, 0x74,
  0x04, 0x5f, 0x74, 0x63, 0x70, 0x05, 0x6c, 0x6f,
  0x63, 0x61, 0x6c, 0x00, 0x00, 0x0c, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x0b, 0x5f, 0x67, 0x6f,
  0x6f, 0x67, 0x6c, 0x65, 0x63, 0x61, 0x73, 0x74,
  0x04, 0x5f, 0x74, 0x63, 0x70, 0x05, 0x6c, 0x6f,
  0x63, 0x61, 0x6c, 0x00, 0x00, 0x0c, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x0b, 0x5f, 0x67, 0x6f,
  0x6f, 0x67, 0x6c, 0x65, 0x63, 0x61, 0x73, 0x74,
  0x04, 0x5f, 0x74, 0x63, 0x70, 0x05, 0x6c, 0x6f,
  0x63, 0x61, 0x6c, 0x00, 0x00, 0x0c, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x0b, 0x5f, 0x67, 0x6f,
  0x6f, 0x67, 0x6c, 0x65, 0x63, 0x61, 0x73, 0x74,
  0x04, 0x5f, 0x74, 0x63, 0x70, 0x05, 0x6c, 0x6f,
  0x63, 0x61, 0x6c, 0x00, 0x00, 0x0c, 0x00, 0x01,
};
unsigned int buf2_len = sizeof(buf2);

TEST(BuildIPv4, Positive) {
    OCDevAddr ipaddr;
    EXPECT_EQ(ERR_SUCCESS, OCBuildIPv4Address(224,0,0,251,5353, &ipaddr));
}


TEST(BuildIPv4, InvalidInput) {
    EXPECT_EQ(ERR_INVALID_INPUT, OCBuildIPv4Address(24,24,24,24,2424, NULL));
    EXPECT_EQ(ERR_INVALID_INPUT, OCBuildIPv4Address(-24,24,-24,24,2424, NULL));
}

TEST(DevAddrToIPv4Addr, Positive) {
    OCDevAddr ipaddr;
    uint8_t  a,b,c,d;
    uint16_t port;
    OCBuildIPv4Address(1,2,3,4,5353, &ipaddr);
    EXPECT_EQ(ERR_SUCCESS, OCDevAddrToIPv4Addr(&ipaddr, &a, &b, &c, &d ));
    EXPECT_TRUE((a == 1) && (b == 2) && (c == 3) && (d ==4));
    EXPECT_EQ(ERR_SUCCESS, OCDevAddrToPort(&ipaddr, &port ));
    EXPECT_TRUE(port == 5353);
}


TEST(DevAddrToIPv4Addr, InvalidInput) {
    OCDevAddr ipaddr;
    uint8_t  a,b,c,d;
    uint16_t port;
    OCBuildIPv4Address(1,2,3,4,5353, &ipaddr);
    EXPECT_EQ(ERR_INVALID_INPUT, OCDevAddrToIPv4Addr(NULL, &a, &b, &c, &d ));
    EXPECT_EQ(ERR_INVALID_INPUT, OCDevAddrToIPv4Addr(&ipaddr, NULL, &b, &c, &d ));
    EXPECT_EQ(ERR_INVALID_INPUT, OCDevAddrToIPv4Addr(NULL, NULL, &b, &c, &d ));

    EXPECT_EQ(ERR_INVALID_INPUT, OCDevAddrToPort(NULL, &port ));
    EXPECT_EQ(ERR_INVALID_INPUT, OCDevAddrToPort(&ipaddr, NULL ));
    EXPECT_EQ(ERR_INVALID_INPUT, OCDevAddrToPort(NULL, NULL ));
}




TEST(GetInterfaceAddress, Positive) {
    uint8_t addr[20];
    uint8_t ifname[] = "eth0";
    EXPECT_EQ(ERR_SUCCESS, OCGetInterfaceAddress( ifname, sizeof(ifname), AF_INET,  addr, sizeof(addr)));
    printf("IPv4 Address: %s\n", addr);
    EXPECT_EQ(ERR_SUCCESS, OCGetInterfaceAddress( NULL, 0,  AF_INET, addr, sizeof(addr)));
    printf("IPv4 Address: %s\n", addr);
}

TEST(GetInterfaceAddress, Negative) {
    uint8_t addr[20];
    uint8_t ifname[] = "ethxx";
    EXPECT_EQ(ERR_UNKNOWN, OCGetInterfaceAddress( ifname, sizeof(ifname),  AF_INET, addr, sizeof(addr)));
    EXPECT_EQ(ERR_INVALID_INPUT, OCGetInterfaceAddress( ifname, sizeof(ifname),  AF_INET, NULL, sizeof(addr)));
    EXPECT_EQ(ERR_UNKNOWN, OCGetInterfaceAddress( ifname, sizeof(ifname),  AF_INET, addr, 0));
    EXPECT_EQ(ERR_INVALID_INPUT, OCGetInterfaceAddress( ifname, sizeof(ifname),  AF_INET6, addr, sizeof(addr)));
}

TEST(InitUDP, Positive) {
    OCDevAddr ipaddr;
    int32_t  sockfd;
    uint8_t addr[20];
    uint8_t ifname[] = "eth0";
    uint8_t a,b,c,d;

    OCBuildIPv4Address(0,0,0,0, 0, &ipaddr);
    EXPECT_EQ(ERR_SUCCESS, OCInitUDP(&ipaddr, &sockfd));
    OCClose(sockfd);

    OCBuildIPv4Address(0,0,0,0, 5678, &ipaddr);
    EXPECT_EQ(ERR_SUCCESS, OCInitUDP(&ipaddr, &sockfd));
    OCClose(sockfd);

    OCGetInterfaceAddress( ifname, sizeof(ifname),  AF_INET, addr, sizeof(addr));
    sscanf((const char*)addr, "%d.%d.%d.%d", (int*)&a, (int*)&b, (int*)&c, (int*)&d);
    OCBuildIPv4Address(a,b,c,d, TEST_PORT_NUM, &ipaddr);
    EXPECT_EQ(ERR_SUCCESS, OCInitUDP(&ipaddr, &sockfd));
    OCClose(sockfd);
}


TEST(InitUDP, Negative) {
    OCDevAddr ipaddr;
    int32_t  sockfd;

    OCBuildIPv4Address(0,0,0,0, 0, &ipaddr);
    EXPECT_EQ(ERR_INVALID_INPUT, OCInitUDP(NULL, &sockfd));

    EXPECT_EQ(ERR_INVALID_INPUT, OCInitUDP(&ipaddr, NULL));
}



TEST(InitUDPMulticast, Positive) {
    OCDevAddr ipaddr1;
    int32_t sfd;

    OCBuildIPv4Address(224, 0, 0, 251, 5353, &ipaddr1); //address to which MEMBERSHIP needs to be added
    EXPECT_EQ(ERR_SUCCESS, OCInitUDPMulticast(&ipaddr1, &sfd));
    OCClose(sfd);
}


TEST(InitUDPMulticast, Negative) {
    OCDevAddr ipaddr1;
    int32_t sfd;

    OCBuildIPv4Address(224, 0, 0, 251, 5353, &ipaddr1); //address to which MEMBERSHIP needs to be added
    EXPECT_EQ(ERR_INVALID_INPUT, OCInitUDPMulticast(NULL, &sfd));
    EXPECT_EQ(ERR_INVALID_INPUT, OCInitUDPMulticast(&ipaddr1, NULL));
}


TEST(SendToRecvfromUnicast, Positive) {
    OCDevAddr ipaddr1, ipaddr2, ipaddr3;
    int32_t  ssfd, rsfd;
    uint8_t addr[20];
    uint8_t ifname[] = "eth0";
    uint8_t a,b,c,d;
    uint8_t tmp1[512];

    //Create sending socket
    OCBuildIPv4Address(0,0,0,0, 0, &ipaddr1);
    OCInitUDP(&ipaddr1, &ssfd);

    //Create receiving socket...i.e. bind to the specific port
    OCBuildIPv4Address(0,0,0,0, TEST_PORT_NUM, &ipaddr2);
    OCInitUDP(&ipaddr2, &rsfd);

    //Since this is a Unit test, we will attempt to send message to ourself at a specific port
    OCGetInterfaceAddress( ifname, sizeof(ifname), AF_INET, addr, sizeof(addr));
    sscanf((const char*)addr, "%d.%d.%d.%d", (int*)&a, (int*)&b, (int*)&c, (int*)&d);
    OCBuildIPv4Address(a,b,c,d, TEST_PORT_NUM, &ipaddr2);

    //Test 1 -- Send 40 bytes
    //Send the packet to ipaddr2(myself:TEST_PORT_NUM)
    EXPECT_EQ(buf1_len, OCSendTo(ssfd, buf1, buf1_len, 0, &ipaddr2));
    //Receive the packet
    ipaddr3.size = sizeof(ipaddr3.addr);
    EXPECT_EQ(buf1_len, OCRecvFrom(rsfd, tmp1, sizeof(tmp1), 0, &ipaddr3));
    //Compare the received buffer with send buffer
    EXPECT_EQ(ERR_SUCCESS, memcmp(tmp1, buf1, buf1_len));

    //Test 2 -- Send 1 byte
    //Send the packet to ipaddr2(myself:TEST_PORT_NUM)
    EXPECT_EQ( 1, OCSendTo(ssfd, buf1, 1, 0, &ipaddr2));
    //Receive the packet
    ipaddr3.size = sizeof(ipaddr3.addr);
    EXPECT_EQ( 1, OCRecvFrom(rsfd, tmp1, sizeof(tmp1), 0, &ipaddr3));
    //Compare the received buffer with send buffer
    EXPECT_EQ(ERR_SUCCESS, memcmp(tmp1, buf1, 1));

    //Test 3 -- Send 320 byte
    //Send the packet to ipaddr2(myself:TEST_PORT_NUM)
    EXPECT_EQ(buf2_len, OCSendTo(ssfd, buf2, buf2_len, 0, &ipaddr2));
    //Receive the packet
    ipaddr3.size = sizeof(ipaddr3.addr);
    EXPECT_EQ(buf2_len, OCRecvFrom(rsfd, tmp1, sizeof(tmp1), 0, &ipaddr3));
    //Compare the received buffer with send buffer
    EXPECT_EQ(ERR_SUCCESS, memcmp(tmp1, buf2, buf2_len));

    OCClose(ssfd);
    OCClose(rsfd);
}


TEST(SendToRecvfromMulticast, Positive) {
    OCDevAddr ipaddr1, ipaddr2, ipaddr3, ipaddr4;
    int32_t  ssfd;
    uint8_t tmp1[512];

    //Create sending socket
    OCBuildIPv4Address(0,0,0,0, 5353, &ipaddr1);
    OCBuildIPv4Address(224, 0, 0, 251, 5353, &ipaddr2); //address to which MEMBERSHIP needs to be added
    OCInitUDPMulticast(&ipaddr2, &ssfd);

    //build the multicast address to which we need to send the datagram
    OCBuildIPv4Address(224, 0, 0, 251, 5353, &ipaddr3);

    //Test 1 -- Send 40 bytes
    EXPECT_EQ(buf1_len, OCSendTo(ssfd, buf1, buf1_len, 0, &ipaddr3));
    //Receive the packet
    ipaddr4.size = sizeof(ipaddr4.addr);
    EXPECT_EQ(buf1_len, OCRecvFrom(ssfd, tmp1, sizeof(tmp1), 0, &ipaddr4));
    //Compare the received buffer with send buffer
    EXPECT_EQ(ERR_SUCCESS, memcmp(tmp1, buf1, buf1_len));

    //Test 2 -- Send 1 byte
    EXPECT_EQ( 1, OCSendTo(ssfd, buf1, 1, 0, &ipaddr3));
    //Receive the packet
    ipaddr4.size = sizeof(ipaddr4.addr);
    EXPECT_EQ( 1, OCRecvFrom(ssfd, tmp1, sizeof(tmp1), 0, &ipaddr4));
    //Compare the received buffer with send buffer
    EXPECT_EQ(ERR_SUCCESS, memcmp(tmp1, buf1, 1));

    //Test 3 -- Send 320 byte
    EXPECT_EQ(buf2_len, OCSendTo(ssfd, buf2, buf2_len, 0, &ipaddr3));
    //Receive the packet
    ipaddr4.size = sizeof(ipaddr4.addr);
    EXPECT_EQ(buf2_len, OCRecvFrom(ssfd, tmp1, sizeof(tmp1), 0, &ipaddr3));
    //Compare the received buffer with send buffer
    EXPECT_EQ(ERR_SUCCESS, memcmp(tmp1, buf2, buf2_len));

    OCClose(ssfd);
}

TEST(GetSocketInfo, Positive) {
    OCDevAddr ipaddr;
    int32_t  sockfd;
    uint8_t addr[20];
    uint8_t ifname[] = "eth0";
    uint16_t port;
    uint8_t a,b,c,d;

    OCBuildIPv4Address(0,0,0,0, 0, &ipaddr);
    EXPECT_EQ(ERR_SUCCESS, OCInitUDP(&ipaddr, &sockfd));
    EXPECT_EQ(ERR_SUCCESS, OCGetSocketInfo(sockfd, &port));
    OC_LOG_V(DEBUG, MOD_NAME, "Port %d", port);
    OCClose(sockfd);

    OCBuildIPv4Address(0,0,0,0, 5678, &ipaddr);
    EXPECT_EQ(ERR_SUCCESS, OCInitUDP(&ipaddr, &sockfd));
    EXPECT_EQ(ERR_SUCCESS, OCGetSocketInfo(sockfd, &port));
    OC_LOG_V(DEBUG, MOD_NAME, "Port %d", port);
    EXPECT_TRUE(port == 5678);
    OCClose(sockfd);

    OCGetInterfaceAddress( ifname, sizeof(ifname),  AF_INET, addr, sizeof(addr));
    sscanf((const char*)addr, "%d.%d.%d.%d", (int*)&a, (int*)&b, (int*)&c, (int*)&d);
    OCBuildIPv4Address(a,b,c,d, TEST_PORT_NUM, &ipaddr);
    EXPECT_EQ(ERR_SUCCESS, OCInitUDP(&ipaddr, &sockfd));
    EXPECT_EQ(ERR_SUCCESS, OCGetSocketInfo(sockfd, &port));
    OC_LOG_V(DEBUG, MOD_NAME, "Port %d", port);
    EXPECT_TRUE(port == TEST_PORT_NUM);
    OCClose(sockfd);
}
