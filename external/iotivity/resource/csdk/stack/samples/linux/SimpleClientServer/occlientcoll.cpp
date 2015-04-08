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
#include <iostream>
#include <sstream>
#include "logger.h"
const char *getResult(OCStackResult result);
std::string getIPAddrTBServer(OCClientResponse * clientResponse);
std::string getPortTBServer(OCClientResponse * clientResponse);
std::string getQueryStrForGetPut(unsigned  const char * responsePayload);

#define TAG PCF("occlient")
#define DEFAULT_CONTEXT_VALUE 0x99
#ifndef MAX_LENGTH_IPv4_ADDR
#define MAX_LENGTH_IPv4_ADDR 16
#endif

typedef enum {
    TEST_INVALID = 0,
    TEST_GET_DEFAULT,
    TEST_GET_BATCH,
    TEST_GET_LINK_LIST,
    TEST_PUT_DEFAULT,
    TEST_PUT_BATCH,
    TEST_PUT_LINK_LIST,
    TEST_UNKNOWN_RESOURCE_GET_DEFAULT,
    TEST_UNKNOWN_RESOURCE_GET_BATCH,
    TEST_UNKNOWN_RESOURCE_GET_LINK_LIST,
    MAX_TESTS
} CLIENT_TEST;

unsigned static int TEST = TEST_INVALID;

typedef struct
{
    unsigned char text[30];
    CLIENT_TEST test;
} testToTextMap;

testToTextMap queryInterface[] = {
        {"invalid", TEST_INVALID},
        {"?if=oc.mi.def", TEST_GET_DEFAULT},
        {"?if=oc.mi.b", TEST_GET_BATCH},
        {"?if=oc.mi.ll", TEST_GET_LINK_LIST},
        {"?if=oc.mi.def", TEST_UNKNOWN_RESOURCE_GET_DEFAULT},
        {"?if=oc.mi.b", TEST_UNKNOWN_RESOURCE_GET_BATCH},
        {"?if=oc.mi.ll", TEST_UNKNOWN_RESOURCE_GET_LINK_LIST},
        {"?if=oc.mi.def", TEST_PUT_DEFAULT},
        {"?if=oc.mi.b", TEST_PUT_BATCH},
        {"?if=oc.mi.ll", TEST_PUT_LINK_LIST},
};

static std::string putPayload = "{\"state\":\"off\",\"power\":\"0\"}";

// The handle for the observe registration
OCDoHandle gObserveDoHandle;
// After this crosses a threshold client deregisters for further observations
int gNumObserveNotifies = 1;

int gQuitFlag = 0;
/* SIGINT handler: set gQuitFlag to 1 for graceful termination */
void handleSigInt(int signum) {
    if (signum == SIGINT) {
        gQuitFlag = 1;
    }
}

// Forward Declaration
OCStackApplicationResult getReqCB(void* ctx, OCDoHandle handle, OCClientResponse * clientResponse);
int InitGetRequestToUnavailableResource(OCClientResponse * clientResponse);
int InitObserveRequest(OCClientResponse * clientResponse);
int InitPutRequest(OCClientResponse * clientResponse);
int InitGetRequest(OCClientResponse * clientResponse);
int InitDiscovery();

void PrintUsage()
{
    OC_LOG(INFO, TAG, "Usage : occlientcoll -t <Test Case>");
    OC_LOG(INFO, TAG, "Test Case 1 : Discover Resources && Initiate GET Request on an"\
            "available resource using default interface.");
    OC_LOG(INFO, TAG, "Test Case 2 : Discover Resources && Initiate GET Request on an"\
                 "available resource using batch interface.");
    OC_LOG(INFO, TAG, "Test Case 3 : Discover Resources && Initiate GET Request on an"\
                 "available resource using link list interface.");
    OC_LOG(INFO, TAG, "Test Case 4 : Discover Resources && Initiate GET & PUT Request on an"\
                 "available resource using default interface.");
    OC_LOG(INFO, TAG, "Test Case 5 : Discover Resources && Initiate GET & PUT Request on an"\
                 "available resource using batch interface.");
    OC_LOG(INFO, TAG, "Test Case 6 : Discover Resources && Initiate GET & PUT Request on an"\
                 "available resource using link list interface.");
    OC_LOG(INFO, TAG, "Test Case 7 : Discover Resources && Initiate GET Request on an"\
                 "unavailable resource using default interface.");
    OC_LOG(INFO, TAG, "Test Case 8 : Discover Resources && Initiate GET Request on an"\
                 "unavailable resource using batch interface.");
    OC_LOG(INFO, TAG, "Test Case 9 : Discover Resources && Initiate GET Request on an"\
                 "unavailable resource using link list interface.");
}

OCStackApplicationResult putReqCB(void* ctx, OCDoHandle handle, OCClientResponse * clientResponse) {
    if(clientResponse == NULL)
    {
        OC_LOG(INFO, TAG, "The clientResponse is NULL");
        return   OC_STACK_DELETE_TRANSACTION;
    }
    if(ctx == (void*)DEFAULT_CONTEXT_VALUE) {
        OC_LOG_V(INFO, TAG, "Callback Context for PUT query recvd successfully");
        OC_LOG_V(INFO, TAG, "JSON = %s =============> Discovered", clientResponse->resJSONPayload);
    }

    return OC_STACK_KEEP_TRANSACTION;
}

OCStackApplicationResult getReqCB(void* ctx, OCDoHandle handle, OCClientResponse * clientResponse) {
    OC_LOG_V(INFO, TAG, "StackResult: %s",
            getResult(clientResponse->result));
    if(ctx == (void*)DEFAULT_CONTEXT_VALUE) {
        OC_LOG_V(INFO, TAG, "SEQUENCE NUMBER: %d", clientResponse->sequenceNumber);
        if(clientResponse->sequenceNumber == 0) {
            OC_LOG_V(INFO, TAG, "Callback Context for GET query recvd successfully");
            OC_LOG_V(INFO, TAG, "Fnd' Rsrc': %s", clientResponse->resJSONPayload);
        }
        else {
            OC_LOG_V(INFO, TAG, "Callback Context for Get recvd successfully %d", gNumObserveNotifies);
            OC_LOG_V(INFO, TAG, "Fnd' Rsrc': %s", clientResponse->resJSONPayload);
            gNumObserveNotifies++;
            if (gNumObserveNotifies == 3)
            {
                if (OCCancel (gObserveDoHandle, OC_LOW_QOS, NULL, 0) != OC_STACK_OK){
                    OC_LOG(ERROR, TAG, "Observe cancel error");
                }
            }
        }
    }
    if(TEST == TEST_PUT_DEFAULT || TEST == TEST_PUT_BATCH || TEST == TEST_PUT_LINK_LIST)
    {
        InitPutRequest(clientResponse);
    }
    return OC_STACK_KEEP_TRANSACTION;
}


// This is a function called back when a device is discovered
OCStackApplicationResult discoveryReqCB(void* ctx, OCDoHandle handle,
        OCClientResponse * clientResponse) {
    uint8_t remoteIpAddr[4];
    uint16_t remotePortNu;

    OC_LOG(INFO, TAG,
            "Entering discoveryReqCB (Application Layer CB)");
    OC_LOG_V(INFO, TAG, "StackResult: %s",
            getResult(clientResponse->result));

    if (ctx == (void*) DEFAULT_CONTEXT_VALUE) {
        OC_LOG_V(INFO, TAG, "Callback Context recvd successfully");
    }

    OCDevAddrToIPv4Addr((OCDevAddr *) clientResponse->addr, remoteIpAddr,
            remoteIpAddr + 1, remoteIpAddr + 2, remoteIpAddr + 3);
    OCDevAddrToPort((OCDevAddr *) clientResponse->addr, &remotePortNu);

    OC_LOG_V(INFO, TAG,
            "Device =============> Discovered %s @ %d.%d.%d.%d:%d",
            clientResponse->resJSONPayload, remoteIpAddr[0], remoteIpAddr[1],
            remoteIpAddr[2], remoteIpAddr[3], remotePortNu);

    if(TEST == TEST_UNKNOWN_RESOURCE_GET_DEFAULT || TEST == TEST_UNKNOWN_RESOURCE_GET_BATCH ||\
            TEST == TEST_UNKNOWN_RESOURCE_GET_LINK_LIST)
    {
        InitGetRequestToUnavailableResource(clientResponse);
    }
    else
    {
        InitGetRequest(clientResponse);
    }
    return OC_STACK_KEEP_TRANSACTION;
}


int InitGetRequestToUnavailableResource(OCClientResponse * clientResponse)
{
    OCStackResult ret;
    OCCallbackData cbData;
    OCDoHandle handle;
    std::ostringstream getQuery;
    getQuery << "coap://" << getIPAddrTBServer(clientResponse) << ":" << getPortTBServer(clientResponse) << "/SomeUnknownResource";
    cbData.cb = getReqCB;
    cbData.context = (void*)DEFAULT_CONTEXT_VALUE;
    cbData.cd = NULL;

    ret = OCDoResource(&handle, OC_REST_GET, getQuery.str().c_str(), 0, 0, OC_LOW_QOS,
            &cbData, NULL, 0);
    if (ret != OC_STACK_OK)
    {
        OC_LOG(ERROR, TAG, "OCStack resource error");
    }
    return ret;
}


int InitObserveRequest(OCClientResponse * clientResponse)
{
    OCStackResult ret;
    OCCallbackData cbData;
    OCDoHandle handle;
    std::ostringstream obsReg;
    obsReg << "coap://" << getIPAddrTBServer(clientResponse) << ":" << getPortTBServer(clientResponse) << getQueryStrForGetPut(clientResponse->resJSONPayload);
    cbData.cb = getReqCB;
    cbData.context = (void*)DEFAULT_CONTEXT_VALUE;
    cbData.cd = NULL;
    OC_LOG_V(INFO, TAG, "OBSERVE payload from client = %s ", putPayload.c_str());

    ret = OCDoResource(&handle, OC_REST_OBSERVE, obsReg.str().c_str(), 0, 0, OC_LOW_QOS,
            &cbData, NULL, 0);
    if (ret != OC_STACK_OK)
    {
        OC_LOG(ERROR, TAG, "OCStack resource error");
    }
    else
    {
        gObserveDoHandle = handle;
    }
    return ret;
}


int InitPutRequest(OCClientResponse * clientResponse)
{
    OCStackResult ret;
    OCCallbackData cbData;
    OCDoHandle handle;
    //* Make a PUT query*/
    std::ostringstream getQuery;
    getQuery << "coap://" << getIPAddrTBServer(clientResponse) << ":" << getPortTBServer(clientResponse) <<
    "/a/room" << queryInterface[TEST].text;
    cbData.cb = putReqCB;
    cbData.context = (void*)DEFAULT_CONTEXT_VALUE;
    cbData.cd = NULL;
    OC_LOG_V(INFO, TAG, "PUT payload from client = %s ", putPayload.c_str());

    ret = OCDoResource(&handle, OC_REST_PUT, getQuery.str().c_str(), 0, putPayload.c_str(),
            OC_LOW_QOS, &cbData, NULL, 0);
    if (ret != OC_STACK_OK)
    {
        OC_LOG(ERROR, TAG, "OCStack resource error");
    }
    return ret;
}


int InitGetRequest(OCClientResponse * clientResponse)
{
    OCStackResult ret;
    OCCallbackData cbData;
    OCDoHandle handle;

    uint8_t remoteIpAddr[4];
    uint16_t remotePortNu;

    OCDevAddrToIPv4Addr((OCDevAddr *) clientResponse->addr, remoteIpAddr,
            remoteIpAddr + 1, remoteIpAddr + 2, remoteIpAddr + 3);
    OCDevAddrToPort((OCDevAddr *) clientResponse->addr, &remotePortNu);

    //* Make a GET query*/
    std::ostringstream getQuery;
    getQuery << "coap://" << getIPAddrTBServer(clientResponse) << ":" << getPortTBServer(clientResponse) <<
    "/a/room" << queryInterface[TEST].text;

    std::cout << "Get Query: " << getQuery.str() << std::endl;

    cbData.cb = getReqCB;
    cbData.context = (void*)DEFAULT_CONTEXT_VALUE;
    cbData.cd = NULL;
    ret = OCDoResource(&handle, OC_REST_GET, getQuery.str().c_str(), 0, 0, OC_LOW_QOS,
            &cbData, NULL, 0);
    if (ret != OC_STACK_OK)
    {
        OC_LOG(ERROR, TAG, "OCStack resource error");
    }
    return ret;
}

int InitDiscovery()
{
    OCStackResult ret;
    OCCallbackData cbData;
    OCDoHandle handle;
    /* Start a discovery query*/
    char szQueryUri[64] = { 0 };

    strcpy(szQueryUri, OC_WELL_KNOWN_QUERY);

    cbData.cb = discoveryReqCB;
    cbData.context = (void*)DEFAULT_CONTEXT_VALUE;
    cbData.cd = NULL;
    ret = OCDoResource(&handle, OC_REST_GET, szQueryUri, 0, 0, OC_LOW_QOS,
            &cbData, NULL, 0);
    if (ret != OC_STACK_OK)
    {
        OC_LOG(ERROR, TAG, "OCStack resource error");
    }
    return ret;
}

int main(int argc, char* argv[]) {
    uint8_t addr[20] = {0};
    uint8_t* paddr = NULL;
    uint16_t port = USE_RANDOM_PORT;
    uint8_t ifname[] = "eth0";
    int opt;

    while ((opt = getopt(argc, argv, "t:")) != -1)
    {
        switch(opt)
        {
        case 't':
            TEST = atoi(optarg);
            break;
        default:
            PrintUsage();
            return -1;
        }
    }
    if(TEST <= TEST_INVALID || TEST >= MAX_TESTS){
        PrintUsage();
        return -1;
    }

    /*Get Ip address on defined interface and initialize coap on it with random port number
     * this port number will be used as a source port in all coap communications*/
    if ( OCGetInterfaceAddress(ifname, sizeof(ifname), AF_INET, addr,
                               sizeof(addr)) == ERR_SUCCESS)
    {
        OC_LOG_V(INFO, TAG, "Starting occlient on address %s",addr);
        paddr = addr;
    }

    /* Initialize OCStack*/
    if (OCInit((char *) paddr, port, OC_CLIENT) != OC_STACK_OK) {
        OC_LOG(ERROR, TAG, "OCStack init error");
        return 0;
    }

    InitDiscovery();

    // Break from loop with Ctrl+C
    OC_LOG(INFO, TAG, "Entering occlient main loop...");
    signal(SIGINT, handleSigInt);
    while (!gQuitFlag) {

        if (OCProcess() != OC_STACK_OK) {
            OC_LOG(ERROR, TAG, "OCStack process error");
            return 0;
        }

        sleep(2);
    }
    OC_LOG(INFO, TAG, "Exiting occlient main loop...");

    if (OCStop() != OC_STACK_OK) {
        OC_LOG(ERROR, TAG, "OCStack stop error");
    }

    return 0;
}

std::string getIPAddrTBServer(OCClientResponse * clientResponse) {
    if(!clientResponse) return "";
    if(!clientResponse->addr) return "";
    uint8_t a, b, c, d = 0;
    if(0 != OCDevAddrToIPv4Addr(clientResponse->addr, &a, &b, &c, &d) ) return "";

    char ipaddr[16] = {'\0'};
    snprintf(ipaddr,  sizeof(ipaddr), "%d.%d.%d.%d", a,b,c,d); // ostringstream not working correctly here, hence snprintf
    //printf("IP address string of the TB server = %s\n", *out_ipaddr);
    return std::string (ipaddr);
}


std::string getPortTBServer(OCClientResponse * clientResponse){
    if(!clientResponse) return "";
    if(!clientResponse->addr) return "";
    uint16_t p = 0;
    if(0 != OCDevAddrToPort(clientResponse->addr, &p) ) return "";
    std::ostringstream ss;
    ss << p;
    return ss.str();
}

std::string getQueryStrForGetPut(unsigned  const char * responsePayload){

    std::string jsonPayload(reinterpret_cast<char*>(const_cast<unsigned char*>(responsePayload)));

    return "/a/room";
}
