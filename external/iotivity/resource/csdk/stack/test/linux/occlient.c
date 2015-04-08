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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <ocstack.h>
#include <logger.h>

#define TAG PCF("occlient")

int gQuitFlag = 0;

/* SIGINT handler: set gQuitFlag to 1 for graceful termination */
void handleSigInt(int signum) {
    if (signum == SIGINT) {
        gQuitFlag = 1;
    }
}

// This is a function called back when a device is discovered
OCStackApplicationResult applicationDiscoverCB(
        OCClientResponse * clientResponse) {
    uint8_t remoteIpAddr[4];
    uint16_t remotePortNu;
    OC_LOG(INFO, TAG, "Entering applicationDiscoverCB (Application Layer CB)");
    OCDevAddrToIPv4Addr((OCDevAddr *) clientResponse->addr, remoteIpAddr,
            remoteIpAddr + 1, remoteIpAddr + 2, remoteIpAddr + 3);
    OCDevAddrToPort((OCDevAddr *) clientResponse->addr, &remotePortNu);
    OC_LOG_V(INFO, TAG, "Device =============> Discovered %s @ %d.%d.%d.%d:%d",clientResponse->resJSONPayload,remoteIpAddr[0], remoteIpAddr[1], remoteIpAddr[2], remoteIpAddr[3], remotePortNu);
    //return OC_STACK_DELETE_TRANSACTION;
    return OC_STACK_KEEP_TRANSACTION;
}

int main() {
    uint8_t addr[20];
    uint16_t port = USE_RANDOM_PORT;
    uint8_t ifname[] = "eth0";
    OCDoHandle handle;

    /*Get Ip address on defined interface and initialize coap on it with random port number
     * this port number will be used as a source port in all coap communications*/
    OCGetInterfaceAddress(ifname, sizeof(ifname), AF_INET, addr, sizeof(addr));
    OC_LOG_V(INFO, TAG, "Starting occlient on address %s",addr);

    /* Initialize OCStack*/
    if (OCInit((char *) addr, port, OC_CLIENT) != OC_STACK_OK) {
        OC_LOG(ERROR, TAG, "OCStack init error");
        return 0;
    }

    /* Start a discovery query*/
    char szQueryUri[64] = { 0 };
    strcpy(szQueryUri, OC_EXPLICIT_DEVICE_DISCOVERY_URI);
    if (OCDoResource(&handle, OC_REST_GET, szQueryUri, 0, 0, OC_LOW_QOS,
            0, 0, 0) != OC_STACK_OK) {
        OC_LOG(ERROR, TAG, "OCStack resource error");
        return 0;
    }

    // Break from loop with Ctrl+C
    OC_LOG(INFO, TAG, "Entering occlient main loop...");
    signal(SIGINT, handleSigInt);
    while (!gQuitFlag) {

        if (OCProcess() != OC_STACK_OK) {
            OC_LOG(ERROR, TAG, "OCStack process error");
            return 0;
        }

        sleep(1);
    }OC_LOG(INFO, TAG, "Exiting occlient main loop...");

    if (OCStop() != OC_STACK_OK) {
        OC_LOG(ERROR, TAG, "OCStack stop error");
    }

    return 0;
}
