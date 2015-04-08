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

#include <ocsocket.h>
#include <logger.h>
#include <SPI.h>
#include <WiFi.h>
#include <WiFiUdp.h>

/// WiFi Shield firmware with Intel patches
static const char INTEL_WIFI_SHIELD_FW_VER[] = "1.2.0";

/// IP address of the peer whom I wish to send/recv some data
static uint8_t PEER_IP_ADDR[] = {192, 168, 1, 132};

/// Port number of the peer whom I wish to send/recv some data
#define TEST_PORT_NUM (4097)

/// Max buffer size
#define MAX_BUF_SIZE  (1024)

/// Some test bytes to send to the peer
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

#define MOD_NAME PCF("ocsocket_test")

#define VERIFY_SUCCESS(op, res) { if (op == res) {OC_LOG(DEBUG, MOD_NAME, PCF(#op " SUCCEEDED"));} \
     else {OC_LOG(FATAL, MOD_NAME, PCF(#op "!!!!  FAILED FAILED FAILED !!!!"));} }

/// WiFi network info and credentials
char ssid[] = "mDNSAP";
char pass[] = "letmein9";
int status = WL_IDLE_STATUS;

//Start sample app 'sendrecv' on different Linux box.
void SendReceiveTest()
{
    Serial.println("entering SendReceiveTest...");
    OCDevAddr ipAddr, peerAddr, recvAddr;
    int32_t sfd;
    int32_t recvLen;
    uint8_t buf[MAX_BUF_SIZE];
    uint16_t recvPort;

    VERIFY_SUCCESS(OCBuildIPv4Address( 0, 0, 0, 0, TEST_PORT_NUM, &ipAddr), ERR_SUCCESS);
    VERIFY_SUCCESS(OCBuildIPv4Address(PEER_IP_ADDR[0], PEER_IP_ADDR[1],
                PEER_IP_ADDR[2], PEER_IP_ADDR[3], TEST_PORT_NUM, &peerAddr), ERR_SUCCESS);
    VERIFY_SUCCESS(OCInitUDP( &ipAddr, &sfd), ERR_SUCCESS);
    OC_LOG(DEBUG, MOD_NAME, PCF("Peer Addr :"));
    OC_LOG_BUFFER(INFO, MOD_NAME, peerAddr.addr, peerAddr.size);
    for (int i = 0; i < 300; i++)
    {

        OC_LOG_V(DEBUG, MOD_NAME, "--------------------- i --------------- %d", i);
        VERIFY_SUCCESS(OCSendTo(sfd, TEST_BUF, i + 10, 0, &peerAddr), i+10);

        delay(2000);
        recvLen = OCRecvFrom(sfd, buf, MAX_BUF_SIZE, 0, &recvAddr);
        if (recvLen > 0)
        {
            OC_LOG(DEBUG, MOD_NAME, PCF("Rcvd data from :"));
            OC_LOG_BUFFER(INFO, MOD_NAME, recvAddr.addr, recvAddr.size);
            OCDevAddrToPort(&recvAddr, &recvPort);
            OC_LOG_V(DEBUG, MOD_NAME, "Recv Data from Port %hu", recvPort);
            OC_LOG(DEBUG, MOD_NAME, PCF("Data Length :"));
            OC_LOG_BUFFER(INFO, MOD_NAME, (uint8_t*)&recvLen, sizeof(recvLen));
            OC_LOG(DEBUG, MOD_NAME, PCF("Data :"));
            if (recvLen < 255)
            {
                OC_LOG_BUFFER(INFO, MOD_NAME, buf, recvLen);
            }
            else
            {
                int idx = 0;
                int rem = recvLen;
                do {
                    if (rem > 255)
                    {
                        OC_LOG_BUFFER(INFO, MOD_NAME, buf + idx, 255);
                        rem = rem - 255;
                    }
                    else
                    {
                        OC_LOG_BUFFER(INFO, MOD_NAME, buf + idx, rem);
                        rem = 0;
                    }
                    idx = idx + 255;
                }while(rem > 0);
            }

        }
    }
    VERIFY_SUCCESS(OCClose( sfd), ERR_SUCCESS);
    OC_LOG(DEBUG, MOD_NAME, PCF("WifiTest - Completed"));

}
void setup()
{
    Serial.begin(115200);
    Serial.println("WiFi test starting...");

    // check for the presence of the shield:
    if (WiFi.status() == WL_NO_SHIELD)
    {
      Serial.println("WiFi shield not present");
      // don't continue:
      while(true);
    }

    Serial.print("WiFi version: ");
    Serial.println(WiFi.firmwareVersion());

    if (strcmp(WiFi.firmwareVersion(), INTEL_WIFI_SHIELD_FW_VER) != 0)
    {
        Serial.println("!!!!! Upgrade WiFi Shield Firmware version !!!!!!");
    }

    // attempt to connect to Wifi network:
    while ( status != WL_CONNECTED) {
      Serial.print("Attempting to connect to SSID: ");
      Serial.println(ssid);
      status = WiFi.begin(ssid,pass);

      // wait 10 seconds for connection:
      delay(10000);
    }
    Serial.println("Connected to wifi");

    IPAddress ip = WiFi.localIP();
    Serial.print("IP Address: ");
    Serial.println(ip);
}

void loop() {
    Serial.println("entering loop...");
    SendReceiveTest();
    delay(5000);

    do {
        OC_LOG(DEBUG, MOD_NAME, PCF("All tests Completed"));
        delay(10000);
    } while (1);
}


