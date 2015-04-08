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

#include <SPI.h>
#include <Ethernet.h>
#include <ocsocket.h>
#include <logger.h>

//Mac address of my ethernet shield
static uint8_t ETHERNET_MAC[] = {0x90, 0xA2, 0xDA, 0x0F, 0x2B, 0x72 };
//IP address of the peer whom I wish to send some data
static uint8_t PEER_IP_ADDR[] = {192, 168, 1, 125};
static uint8_t MULTICAST_IP_ADDR[] = {224, 0, 1, 187};

//Set below to 0 to disable multicast testing
//#define MCAST_TESTING_EN 1

#define TEST_NUM_PKTS (20)
#define TEST_PORT_NUM (8888)
#define MAX_BUF_SIZE  (256)

unsigned char TEST_BUF[] = {
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
unsigned int TEST_BUF_LEN = sizeof(TEST_BUF);

#define PCF(str) ((PROGMEM const char *)(F(str)))

#define MOD_NAME PCF("ocsocket_test")

#define VERIFY_SUCCESS(op, res) { if (op == res) {OC_LOG(DEBUG, MOD_NAME, PCF(#op " SUCCEEDED"));} \
     else {OC_LOG(FATAL, MOD_NAME, PCF(#op "!!!!  FAILED FAILED FAILED !!!!"));} }


//OCGetInterfaceAddress tests
void test10() {
    char strAddr[16] = "";
    VERIFY_SUCCESS(OCGetInterfaceAddress(NULL, 0, AF_INET, (uint8_t*)strAddr, 16), ERR_SUCCESS);
    OC_LOG(DEBUG, MOD_NAME, PCF("My IP address :"));
    OC_LOG_BUFFER(INFO, MOD_NAME, (uint8_t*)strAddr, sizeof(strAddr));
    delay(15000);
    VERIFY_SUCCESS(OCGetInterfaceAddress(NULL, 0, AF_INET, NULL, 16), ERR_INVALID_INPUT);
    VERIFY_SUCCESS(OCGetInterfaceAddress(NULL, 0, AF_INET, (uint8_t*)strAddr, 10), ERR_INVALID_INPUT);
    OC_LOG(DEBUG, MOD_NAME, PCF("test10 - Completed"));
}

//OCBuildIPv4Address tests
void test20() {
    OCDevAddr ipAddr;
    VERIFY_SUCCESS(OCBuildIPv4Address( 0, 0, 0, 0, TEST_PORT_NUM, &ipAddr), ERR_SUCCESS);
    VERIFY_SUCCESS(OCBuildIPv4Address( 0, 0, 0, 0, TEST_PORT_NUM, NULL), ERR_INVALID_INPUT);
    OC_LOG(DEBUG, MOD_NAME, PCF("test20 - Completed"));
}


void test30() {

    OCDevAddr ipAddr;
    uint8_t a, b, c, d;
    uint16_t port;
    VERIFY_SUCCESS(OCBuildIPv4Address( 1, 2, 3, 4, TEST_PORT_NUM, &ipAddr), ERR_SUCCESS);
    VERIFY_SUCCESS(OCDevAddrToIPv4Addr( &ipAddr, &a, &b, &c, &d), ERR_SUCCESS);
    if ((a != 1) || (b != 2) || (c !=3) || (d !=4)) {
        OC_LOG(DEBUG, MOD_NAME, PCF("test30 - Failed !!!"));
        return;
    }

    VERIFY_SUCCESS(OCDevAddrToPort( &ipAddr, &port), ERR_SUCCESS);
    if (port != TEST_PORT_NUM) {
        OC_LOG(DEBUG, MOD_NAME, PCF("test30 - Failed !!!"));
        return;
    }
    VERIFY_SUCCESS(OCDevAddrToIPv4Addr( NULL, &a, &b, &c, &d), ERR_INVALID_INPUT);
    VERIFY_SUCCESS(OCDevAddrToPort( &ipAddr, NULL), ERR_INVALID_INPUT);
    OC_LOG(DEBUG, MOD_NAME, PCF("test30 - Completed"));
}

//OCInitUDP tests
void test40() {
    OCDevAddr ipAddr;
    int32_t sfd1, sfd2, sfd3, sfd4, sfd5;

    //We should be able to successfully open 4 sockets.
    VERIFY_SUCCESS(OCBuildIPv4Address( 0, 0, 0, 0, TEST_PORT_NUM, &ipAddr), ERR_SUCCESS);
    VERIFY_SUCCESS(OCInitUDP( &ipAddr, &sfd1), ERR_SUCCESS);

    VERIFY_SUCCESS(OCBuildIPv4Address( 0, 0, 0, 0, TEST_PORT_NUM + 1, &ipAddr), ERR_SUCCESS);
    VERIFY_SUCCESS(OCInitUDP( &ipAddr, &sfd2), ERR_SUCCESS);

    VERIFY_SUCCESS(OCBuildIPv4Address( 0, 0, 0, 0, TEST_PORT_NUM + 2, &ipAddr), ERR_SUCCESS);
    VERIFY_SUCCESS(OCInitUDP( &ipAddr, &sfd3), ERR_SUCCESS);

    VERIFY_SUCCESS(OCBuildIPv4Address( 0, 0, 0, 0, TEST_PORT_NUM + 3, &ipAddr), ERR_SUCCESS);
    VERIFY_SUCCESS(OCInitUDP( &ipAddr, &sfd4), ERR_SUCCESS);

    //5th socket creation call should FAIL
    VERIFY_SUCCESS(OCBuildIPv4Address( 0, 0, 0, 0, TEST_PORT_NUM + 4, &ipAddr), ERR_SUCCESS);
    VERIFY_SUCCESS(OCInitUDP( &ipAddr, &sfd5), ERR_UNKNOWN);

    VERIFY_SUCCESS(OCClose( sfd4), ERR_SUCCESS);
    VERIFY_SUCCESS(OCInitUDP( &ipAddr, &sfd5), ERR_SUCCESS);

    VERIFY_SUCCESS(OCClose( sfd1), ERR_SUCCESS);
    VERIFY_SUCCESS(OCClose( sfd2), ERR_SUCCESS);
    VERIFY_SUCCESS(OCClose( sfd3), ERR_SUCCESS);
    VERIFY_SUCCESS(OCClose( sfd5), ERR_SUCCESS);
    OC_LOG(DEBUG, MOD_NAME, PCF("test40 - Completed"));
}


//OCInitUDPMulticast tests
void test50() {
    OCDevAddr ipAddr;
    int32_t sfd1, sfd2;

    VERIFY_SUCCESS(OCBuildIPv4Address( MULTICAST_IP_ADDR[0], MULTICAST_IP_ADDR[1],
                MULTICAST_IP_ADDR[2], MULTICAST_IP_ADDR[3], TEST_PORT_NUM, &ipAddr), ERR_SUCCESS);
    VERIFY_SUCCESS(OCInitUDPMulticast( &ipAddr, &sfd1), ERR_SUCCESS);

    VERIFY_SUCCESS(OCBuildIPv4Address( 0, 0, 0, 0, TEST_PORT_NUM + 1, &ipAddr), ERR_SUCCESS);
    VERIFY_SUCCESS(OCInitUDP( &ipAddr, &sfd2), ERR_SUCCESS);

    VERIFY_SUCCESS(OCClose( sfd1), ERR_SUCCESS);
    VERIFY_SUCCESS(OCClose( sfd2), ERR_SUCCESS);
    OC_LOG(DEBUG, MOD_NAME, PCF("test50 - Completed"));
}


//OCSendTo -- Unicast tests
void test60() {
    OCDevAddr ipAddr, peerAddr;
    int32_t sfd;

    VERIFY_SUCCESS(OCBuildIPv4Address( 0, 0, 0, 0, TEST_PORT_NUM, &ipAddr), ERR_SUCCESS);
    VERIFY_SUCCESS(OCBuildIPv4Address(PEER_IP_ADDR[0], PEER_IP_ADDR[0],
                PEER_IP_ADDR[0], PEER_IP_ADDR[0], TEST_PORT_NUM, &peerAddr), ERR_SUCCESS);

    VERIFY_SUCCESS(OCInitUDP( &ipAddr, &sfd), ERR_SUCCESS);
    for (int i = 2; i < 300; ) {
        VERIFY_SUCCESS(OCSendTo(sfd, TEST_BUF, i, 0, &peerAddr), i);
        i = i + 12;

        delay(100);
    }
    VERIFY_SUCCESS(OCClose( sfd), ERR_SUCCESS);
    OC_LOG(DEBUG, MOD_NAME, PCF("test60 - Completed"));
}


//OCSendTo -- Multicast tests
void test70() {
    OCDevAddr ipAddrMcast;
    int32_t sfd;

    VERIFY_SUCCESS(OCBuildIPv4Address( MULTICAST_IP_ADDR[0], MULTICAST_IP_ADDR[1],
                MULTICAST_IP_ADDR[2], MULTICAST_IP_ADDR[3], TEST_PORT_NUM, &ipAddrMcast), ERR_SUCCESS);
    VERIFY_SUCCESS(OCInitUDPMulticast( &ipAddrMcast, &sfd), ERR_SUCCESS);
    for (int i = 2; i < 300; ) {
        VERIFY_SUCCESS(OCSendTo(sfd, TEST_BUF, i, 0, &ipAddrMcast), i);
        i = i + 12;

        delay(100);
    }
    VERIFY_SUCCESS(OCClose( sfd), ERR_SUCCESS);
    OC_LOG(DEBUG, MOD_NAME, PCF("test70 - Completed"));
}



//End to End - Unicast Send/Recv test
//Start sample app 'sendrecv' on different Linux box.
void test80() {
    OCDevAddr ipAddr, peerAddr;
    int32_t sfd;
    int32_t recvLen;
    uint8_t buf[MAX_BUF_SIZE];
    uint8_t pktrecv = 0;

    //Unicast
    VERIFY_SUCCESS(OCBuildIPv4Address( 0, 0, 0, 0, TEST_PORT_NUM, &ipAddr), ERR_SUCCESS);
    VERIFY_SUCCESS(OCInitUDP( &ipAddr, &sfd), ERR_SUCCESS);

    do {
        recvLen = OCRecvFrom(sfd, buf, MAX_BUF_SIZE, 0, &peerAddr);
        if (recvLen > 0) {
            pktrecv++;
            OC_LOG(DEBUG, MOD_NAME, PCF("Rcvd data from :"));
            OC_LOG_BUFFER(INFO, MOD_NAME, peerAddr.addr, peerAddr.size);
            OC_LOG(DEBUG, MOD_NAME, PCF("Data Length :"));
            OC_LOG_BUFFER(INFO, MOD_NAME, (uint8_t*)&recvLen, sizeof(recvLen));

            VERIFY_SUCCESS(OCSendTo(sfd, buf, recvLen, 0, &peerAddr), recvLen);
        } else {
            OC_LOG(DEBUG, MOD_NAME, PCF("No data received"));
        }
        delay(500); //delay  secs
    } while (pktrecv < TEST_NUM_PKTS);

    VERIFY_SUCCESS(OCClose(sfd), ERR_SUCCESS);
    OC_LOG(DEBUG, MOD_NAME, PCF("test80 - Completed"));
}


//End to End - Multicast Send/Recv test
//Start sample app 'sendrecv' on different Linux box.
void test90() {
    OCDevAddr ipAddrMcast, peerAddr;
    int32_t sfd;
    int32_t recvLen;
    uint8_t buf[MAX_BUF_SIZE];
    uint8_t pktrecv = 0;

    //Multicast
    VERIFY_SUCCESS(OCBuildIPv4Address( MULTICAST_IP_ADDR[0], MULTICAST_IP_ADDR[1],
                MULTICAST_IP_ADDR[2], MULTICAST_IP_ADDR[3], TEST_PORT_NUM, &ipAddrMcast), ERR_SUCCESS);
    VERIFY_SUCCESS(OCInitUDPMulticast( &ipAddrMcast, &sfd), ERR_SUCCESS);

    do {

        recvLen = OCRecvFrom(sfd, buf, MAX_BUF_SIZE, 0, &peerAddr);
        if (recvLen > 0) {
            pktrecv++;
            OC_LOG(DEBUG, MOD_NAME, PCF("Rcvd data from :"));
            OC_LOG_BUFFER(INFO, MOD_NAME, peerAddr.addr, peerAddr.size);
            OC_LOG(DEBUG, MOD_NAME, PCF("Data Length :"));
            OC_LOG_BUFFER(INFO, MOD_NAME, (uint8_t*)&recvLen, sizeof(recvLen));

            VERIFY_SUCCESS(OCSendTo(sfd, buf, recvLen, 0, &ipAddrMcast), recvLen);
        } else {
            OC_LOG(DEBUG, MOD_NAME, PCF("No data received"));
        }
        delay(500); //delay  secs
    } while (pktrecv < TEST_NUM_PKTS);

    VERIFY_SUCCESS(OCClose(sfd), ERR_SUCCESS);
    OC_LOG(DEBUG, MOD_NAME, PCF("test90 - Completed"));
}



//End to End - 'Simultaneous' Unicast-Multicast Send/Recv test
//This tests if Arduino Wiznet shield can open 2 independent sockets listening on same port:
//  one for unicast traffic and another for multicast traffic.
//Start sample app 'sendrecv' on different Linux box.
void test100() {
    //TBD
    //TBD
    //TBD
}





void setup() {

    Serial.begin(115200);

    Serial.println("Trying to get an IP address using DHCP");
    if (Ethernet.begin(ETHERNET_MAC) == 0) {
        Serial.println("Failed to configure Ethernet using DHCP");
    }
}

void loop() {
    test10();
    test20();
    test30();
    test40();
    test50();
    test60(); //SendTo --Unicast
    test70(); //SendTo --Multicast
    test80(); //End-to-End  --Unicast
    delay(5000);
    test90(); //End-to-End  --Multicast

    do {
        OC_LOG(DEBUG, MOD_NAME, PCF("All tests Completed"));
        delay(10000);
    } while (1);
}


