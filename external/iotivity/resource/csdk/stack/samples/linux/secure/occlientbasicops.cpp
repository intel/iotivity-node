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
#include <iostream>
#include <sstream>
#include "ocstack.h"
#include "logger.h"
#include "occlientbasicops.h"
#include "cJSON.h"
#include "common.h"

#define TAG "occlientbasicops"
static int UNICAST_DISCOVERY = 0;
static int TEST_CASE = 0;
static const char * TEST_APP_UNICAST_DISCOVERY_QUERY = "coap://0.0.0.0:5683/oc/core";
static std::string putPayload = "{\"state\":\"off\",\"power\":10}";
static std::string coapServerIP;
static std::string coapServerPort;
static std::string coapServerResource;
static int coapSecureResource;

//File containing Client's Identity and the PSK credentials
//of other devices which the client trusts
//This can be generated using 'gen_sec_bin' application
static char CRED_FILE[] = "client_cred.bin";


int gQuitFlag = 0;

/* SIGINT handler: set gQuitFlag to 1 for graceful termination */
void handleSigInt(int signum)
{
    if (signum == SIGINT)
    {
        gQuitFlag = 1;
    }
}

static void PrintUsage()
{
    OC_LOG(INFO, TAG, "Usage : occlient -u <0|1> -t <1|2|3>");
    OC_LOG(INFO, TAG, "-u <0|1> : Perform multicast/unicast discovery of resources");
    OC_LOG(INFO, TAG, "-t 1 : Discover Resources");
    OC_LOG(INFO, TAG, "-t 2 : Discover Resources and"
            " Initiate Nonconfirmable Get/Put/Post Requests");
    OC_LOG(INFO, TAG, "-t 3 : Discover Resources and Initiate Confirmable Get/Put/Post Requests");
}

OCStackResult InvokeOCDoResource(std::ostringstream &query,
        OCMethod method, OCQualityOfService qos,
        OCClientResponseHandler cb, OCHeaderOption * options, uint8_t numOptions)
{
    OCStackResult ret;
    OCCallbackData cbData;
    OCDoHandle handle;

    cbData.cb = cb;
    cbData.context = NULL;
    cbData.cd = NULL;

    ret = OCDoResource(&handle, method, query.str().c_str(), 0,
            (method == OC_REST_PUT || method == OC_REST_POST) ? putPayload.c_str() : NULL,
            qos, &cbData, options, numOptions);

    if (ret != OC_STACK_OK)
    {
        OC_LOG_V(ERROR, TAG, "OCDoResource returns error %d with method %d", ret, method);
    }

    return ret;
}

OCStackApplicationResult putReqCB(void* ctx, OCDoHandle handle, OCClientResponse * clientResponse)
{
    OC_LOG(INFO, TAG, "Callback Context for PUT recvd successfully");

    if(clientResponse)
    {
        OC_LOG_V(INFO, TAG, "StackResult: %s",  getResult(clientResponse->result));
        OC_LOG_V(INFO, TAG, "JSON = %s =============> Put Response", clientResponse->resJSONPayload);
    }
    return OC_STACK_DELETE_TRANSACTION;
}

OCStackApplicationResult postReqCB(void *ctx, OCDoHandle handle, OCClientResponse *clientResponse)
{
    OC_LOG(INFO, TAG, "Callback Context for POST recvd successfully");

    if(clientResponse)
    {
        OC_LOG_V(INFO, TAG, "StackResult: %s",  getResult(clientResponse->result));
        OC_LOG_V(INFO, TAG, "JSON = %s =============> Post Response",
                clientResponse->resJSONPayload);
    }
    return OC_STACK_DELETE_TRANSACTION;
}

OCStackApplicationResult getReqCB(void* ctx, OCDoHandle handle, OCClientResponse * clientResponse)
{
    OC_LOG(INFO, TAG, "Callback Context for GET query recvd successfully");

    if(clientResponse)
    {
        OC_LOG_V(INFO, TAG, "StackResult: %s",  getResult(clientResponse->result));
        OC_LOG_V(INFO, TAG, "SEQUENCE NUMBER: %d", clientResponse->sequenceNumber);
        OC_LOG_V(INFO, TAG, "JSON = %s =============> Get Response",
                clientResponse->resJSONPayload);
    }
    return OC_STACK_DELETE_TRANSACTION;
}

// This is a function called back when a device is discovered
OCStackApplicationResult discoveryReqCB(void* ctx, OCDoHandle handle,
        OCClientResponse * clientResponse)
{
    uint8_t remoteIpAddr[4];
    uint16_t remotePortNu;

    OC_LOG(INFO, TAG, "Callback Context for DISCOVER query recvd successfully");

    if (clientResponse)
    {
        OC_LOG_V(INFO, TAG, "StackResult: %s", getResult(clientResponse->result));

        OCDevAddrToIPv4Addr((OCDevAddr *) clientResponse->addr, remoteIpAddr,
                remoteIpAddr + 1, remoteIpAddr + 2, remoteIpAddr + 3);
        OCDevAddrToPort((OCDevAddr *) clientResponse->addr, &remotePortNu);

        OC_LOG_V(INFO, TAG,
                "Device =============> Discovered %s @ %d.%d.%d.%d:%d",
                clientResponse->resJSONPayload, remoteIpAddr[0], remoteIpAddr[1],
                remoteIpAddr[2], remoteIpAddr[3], remotePortNu);

        if (parseClientResponse(clientResponse) != -1)
        {
            switch(TEST_CASE)
            {
                case TEST_NON_CON_OP:
                    InitGetRequest(OC_LOW_QOS);
                    InitPutRequest();
                    //InitPostRequest(OC_LOW_QOS);
                    break;
                case TEST_CON_OP:
                    InitGetRequest(OC_HIGH_QOS);
                    InitPutRequest();
                    //InitPostRequest(OC_HIGH_QOS);
                    break;
            }
        }
    }

    return (UNICAST_DISCOVERY) ? OC_STACK_DELETE_TRANSACTION : OC_STACK_KEEP_TRANSACTION ;

}

int InitPutRequest()
{
    OC_LOG_V(INFO, TAG, "\n\nExecuting %s", __func__);
    std::ostringstream query;
    query << (coapSecureResource ? "coaps://" : "coap://") << coapServerIP
        << ":" << coapServerPort << coapServerResource;
    return (InvokeOCDoResource(query, OC_REST_PUT, OC_LOW_QOS, putReqCB, NULL, 0));
}

int InitPostRequest(OCQualityOfService qos)
{
    OCStackResult result;
    OC_LOG_V(INFO, TAG, "\n\nExecuting %s", __func__);
    std::ostringstream query;
    query << (coapSecureResource ? "coaps://" : "coap://") << coapServerIP
        << ":" << coapServerPort << coapServerResource;

    // First POST operation (to create an LED instance)
    result = InvokeOCDoResource(query, OC_REST_POST,
            ((qos == OC_HIGH_QOS) ? OC_HIGH_QOS: OC_LOW_QOS),
            postReqCB, NULL, 0);
    if (OC_STACK_OK != result)
    {
        // Error can happen if for example, network connectivity is down
        OC_LOG(INFO, TAG, "First POST call did not succeed");
    }

    // Second POST operation (to create an LED instance)
    result = InvokeOCDoResource(query, OC_REST_POST,
            ((qos == OC_HIGH_QOS) ? OC_HIGH_QOS: OC_LOW_QOS),
            postReqCB, NULL, 0);
    if (OC_STACK_OK != result)
    {
        OC_LOG(INFO, TAG, "Second POST call did not succeed");
    }

    // This POST operation will update the original resourced /a/led
    return (InvokeOCDoResource(query, OC_REST_POST,
                ((qos == OC_HIGH_QOS) ? OC_HIGH_QOS: OC_LOW_QOS),
                postReqCB, NULL, 0));
}

int InitGetRequest(OCQualityOfService qos)
{
    OC_LOG_V(INFO, TAG, "\n\nExecuting %s", __func__);
    std::ostringstream query;
    query << (coapSecureResource ? "coaps://" : "coap://") << coapServerIP
        << ":" << coapServerPort << coapServerResource;

    return (InvokeOCDoResource(query, OC_REST_GET, (qos == OC_HIGH_QOS)?
            OC_HIGH_QOS:OC_LOW_QOS, getReqCB, NULL, 0));
}

int InitDiscovery()
{
    OCStackResult ret;
    OCCallbackData cbData;
    OCDoHandle handle;
    /* Start a discovery query*/
    char szQueryUri[64] = { 0 };
    if (UNICAST_DISCOVERY)
    {
        strcpy(szQueryUri, TEST_APP_UNICAST_DISCOVERY_QUERY);
    }
    else
    {
        strcpy(szQueryUri, OC_WELL_KNOWN_QUERY);
    }
    cbData.cb = discoveryReqCB;
    cbData.context = NULL;
    cbData.cd = NULL;
    ret = OCDoResource(&handle, OC_REST_GET, szQueryUri, 0, 0, OC_LOW_QOS, &cbData, NULL, 0);
    if (ret != OC_STACK_OK)
    {
        OC_LOG(ERROR, TAG, "OCStack resource error");
    }
    return ret;
}

int main(int argc, char* argv[])
{
    uint8_t addr[20] = {0};
    uint8_t* paddr = NULL;
    uint16_t port = USE_RANDOM_PORT;
    uint8_t ifname[] = "eth0";
    int opt;
    struct timespec timeout;

    while ((opt = getopt(argc, argv, "u:t:")) != -1)
    {
        switch(opt)
        {
            case 'u':
                UNICAST_DISCOVERY = atoi(optarg);
                break;
            case 't':
                TEST_CASE = atoi(optarg);
                break;
            default:
                PrintUsage();
                return -1;
        }
    }

    if ((UNICAST_DISCOVERY != 0 && UNICAST_DISCOVERY != 1) ||
            (TEST_CASE < TEST_DISCOVER_REQ || TEST_CASE >= MAX_TESTS) )
    {
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
    if (OCInit((char *) paddr, port, OC_CLIENT) != OC_STACK_OK)
    {
        OC_LOG(ERROR, TAG, "OCStack init error");
        return 0;
    }

    /*
     * Read DTLS PSK credentials from persistent storage and
     * set in the OC stack.
     */
    if (SetCredentials(CRED_FILE) != OC_STACK_OK)
    {
        OC_LOG(ERROR, TAG, "SetCredentials failed");
        return 0;
    }

    InitDiscovery();

    timeout.tv_sec  = 0;
    timeout.tv_nsec = 100000000L;

    // Break from loop with Ctrl+C
    OC_LOG(INFO, TAG, "Entering occlient main loop...");
    signal(SIGINT, handleSigInt);
    while (!gQuitFlag)
    {
        if (OCProcess() != OC_STACK_OK)
        {
            OC_LOG(ERROR, TAG, "OCStack process error");
            return 0;
        }

        nanosleep(&timeout, NULL);
    }
    OC_LOG(INFO, TAG, "Exiting occlient main loop...");

    if (OCStop() != OC_STACK_OK)
    {
        OC_LOG(ERROR, TAG, "OCStack stop error");
    }

    return 0;
}

std::string getIPAddrTBServer(OCClientResponse * clientResponse)
{
    if(!clientResponse) return "";
    if(!clientResponse->addr) return "";
    uint8_t a, b, c, d = 0;
    if(0 != OCDevAddrToIPv4Addr(clientResponse->addr, &a, &b, &c, &d) ) return "";

    char ipaddr[16] = {'\0'};
    // ostringstream not working correctly here, hence snprintf
    snprintf(ipaddr,  sizeof(ipaddr), "%d.%d.%d.%d", a,b,c,d);
    return std::string (ipaddr);
}


std::string getPortTBServer(OCClientResponse * clientResponse)
{
    if(!clientResponse) return "";
    if(!clientResponse->addr) return "";
    uint16_t p = 0;
    if(0 != OCDevAddrToPort(clientResponse->addr, &p) ) return "";
    std::ostringstream ss;
    ss << p;
    return ss.str();
}

int parseClientResponse(OCClientResponse * clientResponse)
{
    int port = -1;
    cJSON * root = NULL;
    cJSON * oc = NULL;

    // Initialize all global variables
    coapServerResource.clear();
    coapServerPort.clear();
    coapServerIP.clear();
    coapSecureResource = 0;

    root = cJSON_Parse((char *)(clientResponse->resJSONPayload));
    if (!root)
    {
        return -1;
    }

    oc = cJSON_GetObjectItem(root,"oc");
    if (!oc)
    {
        return -1;
    }

    if (oc->type == cJSON_Array)
    {
        if (cJSON_GetArraySize(oc) > 0)
        {
            cJSON * resource = cJSON_GetArrayItem(oc, 0);
            if (cJSON_GetObjectItem(resource, "href"))
            {
                coapServerResource.assign(cJSON_GetObjectItem(resource, "href")->valuestring);
            }
            else
            {
                coapServerResource = "";
            }
            OC_LOG_V(INFO, TAG, "Uri -- %s", coapServerResource.c_str());

            cJSON * prop = cJSON_GetObjectItem(resource,"prop");
            if (prop)
            {
                // If this is a secure resource, the info about the port at which the
                // resource is hosted on server is embedded inside discovery JSON response
                if (cJSON_GetObjectItem(prop, "sec"))
                {
                    if ((cJSON_GetObjectItem(prop, "sec")->valueint) == 1)
                    {
                        coapSecureResource = 1;
                    }
                }
                OC_LOG_V(INFO, TAG, "Secure -- %s", coapSecureResource == 1 ? "YES" : "NO");
                if (cJSON_GetObjectItem(prop, "port"))
                {
                    port = cJSON_GetObjectItem(prop, "port")->valueint;
                    OC_LOG_V(INFO, TAG, "Hosting Server Port (embedded inside JSON) -- %u", port);

                    std::ostringstream ss;
                    ss << port;
                    coapServerPort = ss.str();
                }
            }
        }
    }
    cJSON_Delete(root);

    coapServerIP = getIPAddrTBServer(clientResponse);
    if (port == -1)
    {
        coapServerPort = getPortTBServer(clientResponse);
        OC_LOG_V(INFO, TAG, "Hosting Server Port -- %s", coapServerPort.c_str());
    }
    return 0;
}
