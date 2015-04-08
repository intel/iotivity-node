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


extern "C" {
    #include "ocstack.h"
    #include "ocstackinternal.h"
    #include "logger.h"
}

#include "gtest/gtest.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>

#include <iostream>
#include <stdint.h>
using namespace std;

#define DEFAULT_CONTEXT_VALUE 0x99
//-----------------------------------------------------------------------------
// Private variables
//-----------------------------------------------------------------------------
static const char TAG[] = "TestHarness";

//-----------------------------------------------------------------------------
// Callback functions
//-----------------------------------------------------------------------------
extern "C"  OCStackApplicationResult asyncDoResourcesCallback(void* ctx, OCDoHandle handle, OCClientResponse * clientResponse) {
    OC_LOG(INFO, TAG, "Entering asyncDoResourcesCallback");

    EXPECT_EQ(OC_STACK_OK, clientResponse->result);

    if(ctx == (void*)DEFAULT_CONTEXT_VALUE) {
        OC_LOG_V(INFO, TAG, "Callback Context recvd successfully");
    }
    OC_LOG_V(INFO, TAG, "result = %d", clientResponse->result);

    return OC_STACK_KEEP_TRANSACTION;
}


//-----------------------------------------------------------------------------
//  Tests
//-----------------------------------------------------------------------------
#if 1
TEST(StackTest, StackInitNullAddr) {
    EXPECT_EQ(OC_STACK_OK, OCInit(0, 5683, OC_SERVER));
    EXPECT_EQ(OC_STACK_OK, OCStop());
}

TEST(StackTest, StackInitNullPort) {
    EXPECT_EQ(OC_STACK_OK, OCInit("127.0.0.1", 0, OC_SERVER));
    EXPECT_EQ(OC_STACK_OK, OCStop());
}

TEST(StackTest, StackInitNullAddrAndPort) {
    EXPECT_EQ(OC_STACK_OK, OCInit(0, 0, OC_SERVER));
    EXPECT_EQ(OC_STACK_OK, OCStop());
}

TEST(StackTest, StackInitInvalidMode) {
    EXPECT_EQ(OC_STACK_ERROR, OCInit(0, 0, (OCMode)10));
    EXPECT_EQ(OC_STACK_ERROR, OCStop());
}

TEST(StackTest, StackStartSuccessClient) {
    EXPECT_EQ(OC_STACK_OK, OCInit("127.0.0.1", 5683, OC_CLIENT));
    EXPECT_EQ(OC_STACK_OK, OCStop());
}

TEST(StackTest, StackStartSuccessServer) {
    EXPECT_EQ(OC_STACK_OK, OCInit("127.0.0.1", 5683, OC_SERVER));
    EXPECT_EQ(OC_STACK_OK, OCStop());
}

TEST(StackTest, StackStartSuccessClientServer) {
    EXPECT_EQ(OC_STACK_OK, OCInit("127.0.0.1", 5683, OC_CLIENT_SERVER));
    EXPECT_EQ(OC_STACK_OK, OCStop());
}

TEST(StackTest, StackStartSuccessiveInits) {
    EXPECT_EQ(OC_STACK_OK, OCInit("127.0.0.1", 5683, OC_SERVER));
    EXPECT_EQ(OC_STACK_OK, OCInit("127.0.0.2", 5683, OC_SERVER));
    EXPECT_EQ(OC_STACK_OK, OCStop());
}

TEST(StackTest, DoResourceDeviceDiscovery) {
    uint8_t addr[20];
    uint16_t port = USE_RANDOM_PORT;
    uint8_t ifname[] = "eth0";
    OCCallbackData cbData;
    OCDoHandle handle;

    /*Get Ip address on defined interface and initialize coap on it with random port number
     * this port number will be used as a source port in all coap communications*/
    OCGetInterfaceAddress(ifname, sizeof(ifname), AF_INET, addr, sizeof(addr));
    OC_LOG_V(INFO, TAG, "Starting DoResourceDeviceDiscovery test on address %s",addr);

    EXPECT_EQ(OC_STACK_OK, OCInit((char *) addr, port, OC_CLIENT));
    /* Start a discovery query*/
    char szQueryUri[64] = { 0 };
    strcpy(szQueryUri, OC_WELL_KNOWN_QUERY);
    cbData.cb = asyncDoResourcesCallback;
    cbData.context = (void*)DEFAULT_CONTEXT_VALUE;
    cbData.cd = NULL;

    EXPECT_EQ(OC_STACK_OK, OCDoResource(&handle, OC_REST_GET, szQueryUri, 0, 0, OC_LOW_QOS, &cbData, NULL, 0));
    //EXPECT_EQ(OC_STACK_OK, OCUpdateResources(SERVICE_URI));
    EXPECT_EQ(OC_STACK_OK, OCStop());
}

TEST(StackTest, StackStopWithoutInit) {
    EXPECT_EQ(OC_STACK_OK, OCInit("127.0.0.1", 5683, OC_CLIENT));
    EXPECT_EQ(OC_STACK_OK, OCStop());
    EXPECT_EQ(OC_STACK_ERROR, OCStop());
}

TEST(StackTest, UpdateResourceNullURI) {
    uint8_t addr[20];
    uint16_t port = USE_RANDOM_PORT;
    uint8_t ifname[] = "eth0";
    OCCallbackData cbData;
    OCDoHandle handle;

    /*Get Ip address on defined interface and initialize coap on it with random port number
     * this port number will be used as a source port in all coap communications*/
    OCGetInterfaceAddress(ifname, sizeof(ifname), AF_INET, addr, sizeof(addr));
    OC_LOG_V(INFO, TAG, "Starting UpdateResourceNullURI test on address %s",addr);

    EXPECT_EQ(OC_STACK_OK, OCInit((char *) addr, port, OC_CLIENT));
    /* Start a discovery query*/
    char szQueryUri[64] = { 0 };
    strcpy(szQueryUri, OC_WELL_KNOWN_QUERY);
    cbData.cb = asyncDoResourcesCallback;
    cbData.context = (void*)DEFAULT_CONTEXT_VALUE;
    cbData.cd = NULL;
    EXPECT_EQ(OC_STACK_OK, OCDoResource(&handle, OC_REST_GET, szQueryUri, 0, 0, OC_LOW_QOS, &cbData, NULL, 0));
    //EXPECT_EQ(OC_STACK_INVALID_URI, OCUpdateResources(0));
    EXPECT_EQ(OC_STACK_OK, OCStop());
}

TEST(StackTest, CreateResourceBadParams) {
    uint8_t addr[20];
    uint16_t port = USE_RANDOM_PORT;
    uint8_t ifname[] = "eth0";

    /*Get Ip address on defined interface and initialize coap on it with random port number
     * this port number will be used as a source port in all coap communications*/
    OCGetInterfaceAddress(ifname, sizeof(ifname), AF_INET, addr, sizeof(addr));

    OC_LOG_V(INFO, TAG, "Starting ocserver on address %s:%d",addr,port);
    EXPECT_EQ(OC_STACK_OK, OCInit((char *) addr, port, OC_SERVER));

    OCResourceHandle handle;

    EXPECT_EQ(OC_STACK_INVALID_PARAM, OCCreateResource(NULL, //&handle,
                                            "core.led",
                                            "core.rw",
                                            "/a/led",
                                            0,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));

    EXPECT_EQ(OC_STACK_INVALID_PARAM, OCCreateResource(&handle,
                                            NULL, //"core.led",
                                            "core.rw",
                                            "/a/led",
                                            0,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));

    // Property bitmask out of range
    EXPECT_EQ(OC_STACK_INVALID_PARAM, OCCreateResource(&handle,
                                            "core.led",
                                            "core.rw",
                                            "/a/led",
                                            0,
                                            16));//OC_DISCOVERABLE|OC_OBSERVABLE));

    EXPECT_EQ(OC_STACK_OK, OCStop());
}
#endif

TEST(StackTest, CreateResourceSuccess) {
    uint8_t addr[20];
    uint16_t port = USE_RANDOM_PORT;
    uint8_t ifname[] = "eth0";

    /*Get Ip address on defined interface and initialize coap on it with random port number
     * this port number will be used as a source port in all coap communications*/
    OCGetInterfaceAddress(ifname, sizeof(ifname), AF_INET, addr, sizeof(addr));

    OC_LOG_V(INFO, TAG, "Starting ocserver on address %s:%d",addr,port);
    EXPECT_EQ(OC_STACK_OK, OCInit((char *) addr, port, OC_SERVER));

    OCResourceHandle handle;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle,
                                            "core.led",
                                            "core.rw",
                                            "/a/led",
                                            0,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));
    const char *url = OCGetResourceUri(handle);
    EXPECT_STREQ("/a/led", url);

    EXPECT_EQ(OC_STACK_OK, OCStop());
}

TEST(StackTest, CreateResourceFailDuplicateUri) {
    uint8_t addr[20];
    uint16_t port = USE_RANDOM_PORT;
    uint8_t ifname[] = "eth0";

    /*Get Ip address on defined interface and initialize coap on it with random port number
     * this port number will be used as a source port in all coap communications*/
    OCGetInterfaceAddress(ifname, sizeof(ifname), AF_INET, addr, sizeof(addr));

    OC_LOG_V(INFO, TAG, "Starting ocserver on address %s:%d",addr,port);
    EXPECT_EQ(OC_STACK_OK, OCInit((char *) addr, port, OC_SERVER));

    OCResourceHandle handle;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle,
                                            "core.led",
                                            "core.rw",
                                            "/a/led",
                                            0,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));
    const char *url = OCGetResourceUri(handle);
    EXPECT_STREQ("/a/led", url);

    EXPECT_EQ(OC_STACK_INVALID_PARAM, OCCreateResource(&handle,
                                            "core.led",
                                            "core.rw",
                                            "/a/led",
                                            0,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));

    EXPECT_EQ(OC_STACK_OK, OCStop());
}

TEST(StackTest, CreateResourceMultipleResources) {
    uint8_t addr[20];
    uint16_t port = USE_RANDOM_PORT;
    uint8_t ifname[] = "eth0";

    /*Get Ip address on defined interface and initialize coap on it with random port number
     * this port number will be used as a source port in all coap communications*/
    OCGetInterfaceAddress(ifname, sizeof(ifname), AF_INET, addr, sizeof(addr));

    OC_LOG_V(INFO, TAG, "Starting ocserver on address %s:%d",addr,port);
    EXPECT_EQ(OC_STACK_OK, OCInit((char *) addr, port, OC_SERVER));

    OCResourceHandle handle1;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle1,
                                            "core.led",
                                            "core.rw",
                                            "/a/led1",
                                            0,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));

    OCResourceHandle handle2;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle2,
                                            "core.led",
                                            "core.rw",
                                            "/a/led2",
                                            0,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));
    OCResourceHandle handle3;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle3,
                                            "core.led",
                                            "core.rw",
                                            "/a/led3",
                                            0,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));

    const char *url = OCGetResourceUri(handle1);
    EXPECT_STREQ("/a/led1", url);

    url = OCGetResourceUri(handle2);
    EXPECT_STREQ("/a/led2", url);

    url = OCGetResourceUri(handle3);
    EXPECT_STREQ("/a/led3", url);

    EXPECT_EQ(OC_STACK_OK, OCStop());
}

TEST(StackTest, CreateResourceBadResoureType) {
    uint8_t addr[20];
    uint16_t port = USE_RANDOM_PORT;
    uint8_t ifname[] = "eth0";

    /*Get Ip address on defined interface and initialize coap on it with random port number
     * this port number will be used as a source port in all coap communications*/
    OCGetInterfaceAddress(ifname, sizeof(ifname), AF_INET, addr, sizeof(addr));

    OC_LOG_V(INFO, TAG, "Starting ocserver on address %s:%d",addr,port);
    EXPECT_EQ(OC_STACK_OK, OCInit((char *) addr, port, OC_SERVER));

    OCResourceHandle handle;
    EXPECT_EQ(OC_STACK_INVALID_PARAM, OCCreateResource(&handle,
                                            NULL, //"core.led",
                                            "core.rw",
                                            "/a/led",
                                            0,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));

    EXPECT_EQ(OC_STACK_OK, OCStop());
}

TEST(StackTest, CreateResourceGoodResourceType) {
    uint8_t addr[20];
    uint16_t port = USE_RANDOM_PORT;
    uint8_t ifname[] = "eth0";

    /*Get Ip address on defined interface and initialize coap on it with random port number
     * this port number will be used as a source port in all coap communications*/
    OCGetInterfaceAddress(ifname, sizeof(ifname), AF_INET, addr, sizeof(addr));

    OC_LOG_V(INFO, TAG, "Starting ocserver on address %s:%d",addr,port);
    EXPECT_EQ(OC_STACK_OK, OCInit((char *) addr, port, OC_SERVER));

    OCResourceHandle handle;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle,
                                            "core.led",
                                            "core.rw",
                                            "/a/led",
                                            0,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));

    EXPECT_EQ(OC_STACK_OK, OCStop());
}

TEST(StackTest, ResourceTypeName) {
    uint8_t addr[20];
    uint16_t port = USE_RANDOM_PORT;
    uint8_t ifname[] = "eth0";

    /*Get Ip address on defined interface and initialize coap on it with random port number
     * this port number will be used as a source port in all coap communications*/
    OCGetInterfaceAddress(ifname, sizeof(ifname), AF_INET, addr, sizeof(addr));

    OC_LOG_V(INFO, TAG, "Starting ocserver on address %s:%d",addr,port);
    EXPECT_EQ(OC_STACK_OK, OCInit((char *) addr, port, OC_SERVER));

    OCResourceHandle handle;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle,
                                            "core.led",
                                            "core.rw",
                                            "/a/led",
                                            0,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));

    uint8_t numResourceTypes;
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResourceTypes(handle, &numResourceTypes));
    EXPECT_EQ(1, numResourceTypes);
    const char *resourceTypeName = OCGetResourceTypeName(handle, 0);
    EXPECT_STREQ("core.led", resourceTypeName);

    // try getting resource type names with an invalid index
    resourceTypeName = OCGetResourceTypeName(handle, 1);
    EXPECT_STREQ(NULL, resourceTypeName);
    // try getting resource type names with an invalid index
    resourceTypeName = OCGetResourceTypeName(handle, 10);
    EXPECT_STREQ(NULL, resourceTypeName);

    EXPECT_EQ(OC_STACK_OK, OCStop());
}

TEST(StackTest, BindResourceTypeNameBad) {
    uint8_t addr[20];
    uint16_t port = USE_RANDOM_PORT;
    uint8_t ifname[] = "eth0";

    /*Get Ip address on defined interface and initialize coap on it with random port number
     * this port number will be used as a source port in all coap communications*/
    OCGetInterfaceAddress(ifname, sizeof(ifname), AF_INET, addr, sizeof(addr));

    OC_LOG_V(INFO, TAG, "Starting ocserver on address %s:%d",addr,port);
    EXPECT_EQ(OC_STACK_OK, OCInit((char *) addr, port, OC_SERVER));

    OCResourceHandle handle;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle,
                                            "core.led",
                                            "core.rw",
                                            "/a/led",
                                            0,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));

    uint8_t numResourceTypes;
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResourceTypes(handle, &numResourceTypes));
    EXPECT_EQ(1, numResourceTypes);
    const char *resourceTypeName = OCGetResourceTypeName(handle, 0);
    EXPECT_STREQ("core.led", resourceTypeName);

    EXPECT_EQ(OC_STACK_INVALID_PARAM, OCBindResourceTypeToResource(handle, NULL));

    EXPECT_EQ(OC_STACK_OK, OCStop());
}

TEST(StackTest, BindResourceTypeNameGood) {
    uint8_t addr[20];
    uint16_t port = USE_RANDOM_PORT;
    uint8_t ifname[] = "eth0";

    /*Get Ip address on defined interface and initialize coap on it with random port number
     * this port number will be used as a source port in all coap communications*/
    OCGetInterfaceAddress(ifname, sizeof(ifname), AF_INET, addr, sizeof(addr));

    OC_LOG_V(INFO, TAG, "Starting ocserver on address %s:%d",addr,port);
    EXPECT_EQ(OC_STACK_OK, OCInit((char *) addr, port, OC_SERVER));

    OCResourceHandle handle;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle,
                                            "core.led",
                                            "core.rw",
                                            "/a/led",
                                            0,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));

    uint8_t numResourceTypes;
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResourceTypes(handle, &numResourceTypes));
    EXPECT_EQ(1, numResourceTypes);
    const char *resourceTypeName = OCGetResourceTypeName(handle, 0);
    EXPECT_STREQ("core.led", resourceTypeName);

    EXPECT_EQ(OC_STACK_OK, OCBindResourceTypeToResource(handle, "core.brightled"));
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResourceTypes(handle, &numResourceTypes));
    EXPECT_EQ(2, numResourceTypes);
    resourceTypeName = OCGetResourceTypeName(handle, 1);
    EXPECT_STREQ("core.brightled", resourceTypeName);

    EXPECT_EQ(OC_STACK_OK, OCBindResourceTypeToResource(handle, "core.reallybrightled"));
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResourceTypes(handle, &numResourceTypes));
    EXPECT_EQ(3, numResourceTypes);
    resourceTypeName = OCGetResourceTypeName(handle, 2);
    EXPECT_STREQ("core.reallybrightled", resourceTypeName);

    EXPECT_EQ(OC_STACK_OK, OCStop());
}

TEST(StackTest, ResourceTypeAttrRepresentation) {
    uint8_t addr[20];
    uint16_t port = USE_RANDOM_PORT;
    uint8_t ifname[] = "eth0";

    /*Get Ip address on defined interface and initialize coap on it with random port number
     * this port number will be used as a source port in all coap communications*/
    OCGetInterfaceAddress(ifname, sizeof(ifname), AF_INET, addr, sizeof(addr));

    OC_LOG_V(INFO, TAG, "Starting ocserver on address %s:%d",addr,port);
    EXPECT_EQ(OC_STACK_OK, OCInit((char *) addr, port, OC_SERVER));

    OCResourceHandle handle;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle,
                                            "core.led",
                                            "core.rw",
                                            "/a/led",
                                            0,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));

    uint8_t numResourceTypes;
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResourceTypes(handle, &numResourceTypes));
    EXPECT_EQ(1, numResourceTypes);

    EXPECT_EQ(OC_STACK_OK, OCStop());
}

TEST(StackTest, BindResourceTypeAttribRepGood) {
    uint8_t addr[20];
    uint16_t port = USE_RANDOM_PORT;
    uint8_t ifname[] = "eth0";

    /*Get Ip address on defined interface and initialize coap on it with random port number
     * this port number will be used as a source port in all coap communications*/
    OCGetInterfaceAddress(ifname, sizeof(ifname), AF_INET, addr, sizeof(addr));

    OC_LOG_V(INFO, TAG, "Starting ocserver on address %s:%d",addr,port);
    EXPECT_EQ(OC_STACK_OK, OCInit((char *) addr, port, OC_SERVER));

    OCResourceHandle handle;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle,
                                            "core.led",
                                            "core.rw",
                                            "/a/led",
                                            0,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));

    uint8_t numResourceTypes;
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResourceTypes(handle, &numResourceTypes));
    EXPECT_EQ(1, numResourceTypes);

    EXPECT_EQ(OC_STACK_OK, OCBindResourceTypeToResource(handle, "core.brightled"));
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResourceTypes(handle, &numResourceTypes));
    EXPECT_EQ(2, numResourceTypes);

    EXPECT_EQ(OC_STACK_OK, OCBindResourceTypeToResource(handle, "core.reallybrightled"));
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResourceTypes(handle, &numResourceTypes));
    EXPECT_EQ(3, numResourceTypes);

    EXPECT_EQ(OC_STACK_OK, OCStop());
}

TEST(StackTest, ResourceTypeInterface) {
    uint8_t addr[20];
    uint16_t port = USE_RANDOM_PORT;
    uint8_t ifname[] = "eth0";

    /*Get Ip address on defined interface and initialize coap on it with random port number
     * this port number will be used as a source port in all coap communications*/
    OCGetInterfaceAddress(ifname, sizeof(ifname), AF_INET, addr, sizeof(addr));

    OC_LOG_V(INFO, TAG, "Starting ocserver on address %s:%d",addr,port);
    EXPECT_EQ(OC_STACK_OK, OCInit((char *) addr, port, OC_SERVER));

    OCResourceHandle handle;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle,
                                            "core.led",
                                            "core.rw",
                                            "/a/led",
                                            0,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));

    uint8_t numResourceInterfaces;
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResourceInterfaces(handle, &numResourceInterfaces));
    EXPECT_EQ(1, numResourceInterfaces);
    const char *resourceInterfaceName = OCGetResourceInterfaceName(handle, 0);
    EXPECT_STREQ("core.rw", resourceInterfaceName);

    // try getting resource interface names with an invalid index
    resourceInterfaceName = OCGetResourceInterfaceName(handle, 1);
    EXPECT_STREQ(NULL, resourceInterfaceName);
    // try getting resource interface names with an invalid index
    resourceInterfaceName = OCGetResourceInterfaceName(handle, 10);
    EXPECT_STREQ(NULL, resourceInterfaceName);

    EXPECT_EQ(OC_STACK_OK, OCStop());
}

TEST(StackTest, BindResourceInterfaceNameBad) {
    uint8_t addr[20];
    uint16_t port = USE_RANDOM_PORT;
    uint8_t ifname[] = "eth0";

    /*Get Ip address on defined interface and initialize coap on it with random port number
     * this port number will be used as a source port in all coap communications*/
    OCGetInterfaceAddress(ifname, sizeof(ifname), AF_INET, addr, sizeof(addr));

    OC_LOG_V(INFO, TAG, "Starting ocserver on address %s:%d",addr,port);
    EXPECT_EQ(OC_STACK_OK, OCInit((char *) addr, port, OC_SERVER));

    OCResourceHandle handle;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle,
                                            "core.led",
                                            "core.rw",
                                            "/a/led",
                                            0,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));

    uint8_t numResourceInterfaces;
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResourceInterfaces(handle, &numResourceInterfaces));
    EXPECT_EQ(1, numResourceInterfaces);
    const char *resourceInterfaceName = OCGetResourceInterfaceName(handle, 0);
    EXPECT_STREQ("core.rw", resourceInterfaceName);

    EXPECT_EQ(OC_STACK_INVALID_PARAM, OCBindResourceInterfaceToResource(handle, NULL));

    EXPECT_EQ(OC_STACK_OK, OCStop());
}

TEST(StackTest, BindResourceInterfaceNameGood) {
    uint8_t addr[20];
    uint16_t port = USE_RANDOM_PORT;
    uint8_t ifname[] = "eth0";

    /*Get Ip address on defined interface and initialize coap on it with random port number
     * this port number will be used as a source port in all coap communications*/
    OCGetInterfaceAddress(ifname, sizeof(ifname), AF_INET, addr, sizeof(addr));

    OC_LOG_V(INFO, TAG, "Starting ocserver on address %s:%d",addr,port);
    EXPECT_EQ(OC_STACK_OK, OCInit((char *) addr, port, OC_SERVER));

    OCResourceHandle handle;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle,
                                            "core.led",
                                            "core.rw",
                                            "/a/led",
                                            0,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));

    uint8_t numResourceInterfaces;
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResourceInterfaces(handle, &numResourceInterfaces));
    EXPECT_EQ(1, numResourceInterfaces);
    const char *resourceInterfaceName = OCGetResourceInterfaceName(handle, 0);
    EXPECT_STREQ("core.rw", resourceInterfaceName);

    EXPECT_EQ(OC_STACK_OK, OCBindResourceInterfaceToResource(handle, "core.r"));

    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResourceInterfaces(handle, &numResourceInterfaces));
    EXPECT_EQ(2, numResourceInterfaces);
    resourceInterfaceName = OCGetResourceInterfaceName(handle, 1);
    EXPECT_STREQ("core.r", resourceInterfaceName);

    EXPECT_EQ(OC_STACK_OK, OCStop());
}

TEST(StackTest, ResourceTypeInterfaceMethods) {
    uint8_t addr[20];
    uint16_t port = USE_RANDOM_PORT;
    uint8_t ifname[] = "eth0";

    /*Get Ip address on defined interface and initialize coap on it with random port number
     * this port number will be used as a source port in all coap communications*/
    OCGetInterfaceAddress(ifname, sizeof(ifname), AF_INET, addr, sizeof(addr));

    OC_LOG_V(INFO, TAG, "Starting ocserver on address %s:%d",addr,port);
    EXPECT_EQ(OC_STACK_OK, OCInit((char *) addr, port, OC_SERVER));

    OCResourceHandle handle;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle,
                                            "core.led",
                                            "core.rw",
                                            "/a/led",
                                            0,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));

    uint8_t numResourceInterfaces;
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResourceInterfaces(handle, &numResourceInterfaces));
    EXPECT_EQ(1, numResourceInterfaces);

    EXPECT_EQ(OC_STACK_OK, OCStop());
}

TEST(StackTest, BindResourceInterfaceMethodsBad) {
    uint8_t addr[20];
    uint16_t port = USE_RANDOM_PORT;
    uint8_t ifname[] = "eth0";

    /*Get Ip address on defined interface and initialize coap on it with random port number
     * this port number will be used as a source port in all coap communications*/
    OCGetInterfaceAddress(ifname, sizeof(ifname), AF_INET, addr, sizeof(addr));

    OC_LOG_V(INFO, TAG, "Starting ocserver on address %s:%d",addr,port);
    EXPECT_EQ(OC_STACK_OK, OCInit((char *) addr, port, OC_SERVER));

    OCResourceHandle handle;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle,
                                            "core.led",
                                            "core.rw",
                                            "/a/led",
                                            0,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));

    uint8_t numResourceInterfaces;
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResourceInterfaces(handle, &numResourceInterfaces));
    EXPECT_EQ(1, numResourceInterfaces);

    EXPECT_EQ(OC_STACK_INVALID_PARAM, OCBindResourceInterfaceToResource(handle, 0));

    EXPECT_EQ(OC_STACK_OK, OCStop());
}

TEST(StackTest, BindResourceInterfaceMethodsGood) {
    uint8_t addr[20];
    uint16_t port = USE_RANDOM_PORT;
    uint8_t ifname[] = "eth0";

    /*Get Ip address on defined interface and initialize coap on it with random port number
     * this port number will be used as a source port in all coap communications*/
    OCGetInterfaceAddress(ifname, sizeof(ifname), AF_INET, addr, sizeof(addr));

    OC_LOG_V(INFO, TAG, "Starting ocserver on address %s:%d",addr,port);
    EXPECT_EQ(OC_STACK_OK, OCInit((char *) addr, port, OC_SERVER));

    OCResourceHandle handle;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle,
                                            "core.led",
                                            "core.rw",
                                            "/a/led",
                                            0,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));

    uint8_t numResourceInterfaces;
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResourceInterfaces(handle, &numResourceInterfaces));
    EXPECT_EQ(1, numResourceInterfaces);

    EXPECT_EQ(OC_STACK_OK, OCBindResourceInterfaceToResource(handle, "core.r"));

    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResourceInterfaces(handle, &numResourceInterfaces));
    EXPECT_EQ(2, numResourceInterfaces);

    EXPECT_EQ(OC_STACK_OK, OCStop());
}

TEST(StackTest, BindContainedResourceBad) {
    uint8_t addr[20];
    uint16_t port = USE_RANDOM_PORT;
    uint8_t ifname[] = "eth0";

    /*Get Ip address on defined interface and initialize coap on it with random port number
     * this port number will be used as a source port in all coap communications*/
    OCGetInterfaceAddress(ifname, sizeof(ifname), AF_INET, addr, sizeof(addr));

    OC_LOG_V(INFO, TAG, "Starting ocserver on address %s:%d",addr,port);
    EXPECT_EQ(OC_STACK_OK, OCInit((char *) addr, port, OC_SERVER));

    OCResourceHandle containerHandle;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&containerHandle,
                                            "core.led",
                                            "core.rw",
                                            "/a/kitchen",
                                            0,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));

    OCResourceHandle handle0;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle0,
                                            "core.led",
                                            "core.rw",
                                            "/a/led",
                                            0,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));

    EXPECT_EQ(OC_STACK_INVALID_PARAM, OCBindResource(containerHandle, containerHandle));

    EXPECT_EQ(OC_STACK_ERROR, OCBindResource((OCResourceHandle) 0, handle0));

    EXPECT_EQ(OC_STACK_OK, OCStop());
}

TEST(StackTest, BindContainedResourceGood) {
    uint8_t addr[20];
    uint16_t port = USE_RANDOM_PORT;
    uint8_t ifname[] = "eth0";

    /*Get Ip address on defined interface and initialize coap on it with random port number
     * this port number will be used as a source port in all coap communications*/
    OCGetInterfaceAddress(ifname, sizeof(ifname), AF_INET, addr, sizeof(addr));

    OC_LOG_V(INFO, TAG, "Starting ocserver on address %s:%d",addr,port);
    EXPECT_EQ(OC_STACK_OK, OCInit((char *) addr, port, OC_SERVER));

    uint8_t numResources = 0;
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResources(&numResources));
    EXPECT_EQ(0, numResources);

    OCResourceHandle containerHandle;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&containerHandle,
                                            "core.led",
                                            "core.rw",
                                            "/a/kitchen",
                                            0,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResources(&numResources));
    EXPECT_EQ(1, numResources);

    OCResourceHandle handle0;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle0,
                                            "core.led",
                                            "core.rw",
                                            "/a/led0",
                                            0,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResources(&numResources));
    EXPECT_EQ(2, numResources);

    OCResourceHandle handle1;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle1,
                                            "core.led",
                                            "core.rw",
                                            "/a/led1",
                                            0,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResources(&numResources));
    EXPECT_EQ(3, numResources);

    OCResourceHandle handle2;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle2,
                                            "core.led",
                                            "core.rw",
                                            "/a/led2",
                                            0,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResources(&numResources));
    EXPECT_EQ(4, numResources);

    OCResourceHandle handle3;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle3,
                                            "core.led",
                                            "core.rw",
                                            "/a/led3",
                                            0,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResources(&numResources));
    EXPECT_EQ(5, numResources);

    OCResourceHandle handle4;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle4,
                                            "core.led",
                                            "core.rw",
                                            "/a/led4",
                                            0,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResources(&numResources));
    EXPECT_EQ(6, numResources);

    OCResourceHandle handle5;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle5,
                                            "core.led",
                                            "core.rw",
                                            "/a/led5",
                                            0,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResources(&numResources));
    EXPECT_EQ(7, numResources);


    EXPECT_EQ(OC_STACK_OK, OCBindResource(containerHandle, handle0));
    EXPECT_EQ(OC_STACK_OK, OCBindResource(containerHandle, handle1));
    EXPECT_EQ(OC_STACK_OK, OCBindResource(containerHandle, handle2));
    EXPECT_EQ(OC_STACK_OK, OCBindResource(containerHandle, handle3));
    EXPECT_EQ(OC_STACK_OK, OCBindResource(containerHandle, handle4));
    EXPECT_EQ(OC_STACK_ERROR, OCBindResource(containerHandle, handle5));

    EXPECT_EQ(handle0, OCGetResourceHandleFromCollection(containerHandle, 0));
    EXPECT_EQ(handle1, OCGetResourceHandleFromCollection(containerHandle, 1));
    EXPECT_EQ(handle2, OCGetResourceHandleFromCollection(containerHandle, 2));
    EXPECT_EQ(handle3, OCGetResourceHandleFromCollection(containerHandle, 3));
    EXPECT_EQ(handle4, OCGetResourceHandleFromCollection(containerHandle, 4));

    EXPECT_EQ(NULL, OCGetResourceHandleFromCollection(containerHandle, 5));

    EXPECT_EQ(OC_STACK_OK, OCStop());
}

TEST(StackTest, GetResourceByIndex) {
    uint8_t addr[20];
    uint16_t port = USE_RANDOM_PORT;
    uint8_t ifname[] = "eth0";

    /*Get Ip address on defined interface and initialize coap on it with random port number
     * this port number will be used as a source port in all coap communications*/
    OCGetInterfaceAddress(ifname, sizeof(ifname), AF_INET, addr, sizeof(addr));

    OC_LOG_V(INFO, TAG, "Starting ocserver on address %s:%d",addr,port);
    EXPECT_EQ(OC_STACK_OK, OCInit((char *) addr, port, OC_SERVER));

    uint8_t numResources = 0;
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResources(&numResources));
    EXPECT_EQ(0, numResources);

    OCResourceHandle containerHandle;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&containerHandle,
                                            "core.led",
                                            "core.rw",
                                            "/a/kitchen",
                                            0,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResources(&numResources));
    EXPECT_EQ(1, numResources);

    OCResourceHandle handle0;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle0,
                                            "core.led",
                                            "core.rw",
                                            "/a/led0",
                                            0,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResources(&numResources));
    EXPECT_EQ(2, numResources);

    OCResourceHandle handle1;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle1,
                                            "core.led",
                                            "core.rw",
                                            "/a/led1",
                                            0,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResources(&numResources));
    EXPECT_EQ(3, numResources);

    OCResourceHandle handle2;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle2,
                                            "core.led",
                                            "core.rw",
                                            "/a/led2",
                                            0,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResources(&numResources));
    EXPECT_EQ(4, numResources);

    OCResourceHandle handle3;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle3,
                                            "core.led",
                                            "core.rw",
                                            "/a/led3",
                                            0,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResources(&numResources));
    EXPECT_EQ(5, numResources);

    OCResourceHandle handle4;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle4,
                                            "core.led",
                                            "core.rw",
                                            "/a/led4",
                                            0,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResources(&numResources));
    EXPECT_EQ(6, numResources);

    OCResourceHandle handle5;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle5,
                                            "core.led",
                                            "core.rw",
                                            "/a/led5",
                                            0,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResources(&numResources));
    EXPECT_EQ(7, numResources);

    EXPECT_EQ(containerHandle, OCGetResourceHandle(0));
    EXPECT_EQ(handle0, OCGetResourceHandle(1));
    EXPECT_EQ(handle1, OCGetResourceHandle(2));
    EXPECT_EQ(handle2, OCGetResourceHandle(3));
    EXPECT_EQ(handle3, OCGetResourceHandle(4));
    EXPECT_EQ(handle4, OCGetResourceHandle(5));
    EXPECT_EQ(handle5, OCGetResourceHandle(6));

    EXPECT_EQ(OC_STACK_OK, OCStop());
}

TEST(StackTest, BindEntityHandlerBad) {
    uint8_t addr[20];
    uint16_t port = USE_RANDOM_PORT;
    uint8_t ifname[] = "eth0";

    /*Get Ip address on defined interface and initialize coap on it with random port number
     * this port number will be used as a source port in all coap communications*/
    OCGetInterfaceAddress(ifname, sizeof(ifname), AF_INET, addr, sizeof(addr));

    OC_LOG_V(INFO, TAG, "Starting ocserver on address %s:%d",addr,port);
    EXPECT_EQ(OC_STACK_OK, OCInit((char *) addr, port, OC_SERVER));

    OCResourceHandle handle;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle,
                                            "core.led",
                                            "core.rw",
                                            "/a/led",
                                            0,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));

    EXPECT_EQ(OC_STACK_INVALID_PARAM, OCBindResourceHandler(handle, NULL));
    EXPECT_EQ(OC_STACK_INVALID_PARAM, OCBindResourceHandler(NULL, NULL));

    EXPECT_EQ(OC_STACK_OK, OCStop());
}


OCEntityHandlerResult entityHandler(OCEntityHandlerFlag flag, OCEntityHandlerRequest * entityHandlerRequest) {
    OC_LOG(INFO, TAG, "Entering entityHandler");

    return OC_EH_OK;
}

TEST(StackTest, BindEntityHandlerGood) {
    uint8_t addr[20];
    uint16_t port = USE_RANDOM_PORT;
    uint8_t ifname[] = "eth0";

    /*Get Ip address on defined interface and initialize coap on it with random port number
     * this port number will be used as a source port in all coap communications*/
    OCGetInterfaceAddress(ifname, sizeof(ifname), AF_INET, addr, sizeof(addr));

    OC_LOG_V(INFO, TAG, "Starting ocserver on address %s:%d",addr,port);
    EXPECT_EQ(OC_STACK_OK, OCInit((char *) addr, port, OC_SERVER));

    OCResourceHandle handle;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle,
                                            "core.led",
                                            "core.rw",
                                            "/a/led",
                                            0,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));

    OCEntityHandler myHandler = entityHandler;

    EXPECT_EQ(OC_STACK_OK, OCBindResourceHandler(handle, myHandler));

    EXPECT_EQ(myHandler, OCGetResourceHandler(handle));

    EXPECT_EQ(OC_STACK_OK, OCStop());
}

TEST(StackTest, DeleteHeadResource) {
    uint8_t addr[20];
    uint16_t port = USE_RANDOM_PORT;
    uint8_t ifname[] = "eth0";

    /*Get Ip address on defined interface and initialize coap on it with random port number
     * this port number will be used as a source port in all coap communications*/
    OCGetInterfaceAddress(ifname, sizeof(ifname), AF_INET, addr, sizeof(addr));

    OC_LOG_V(INFO, TAG, "Starting ocserver on address %s:%d",addr,port);
    EXPECT_EQ(OC_STACK_OK, OCInit((char *) addr, port, OC_SERVER));

    uint8_t numResources = 0;
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResources(&numResources));
    EXPECT_EQ(0, numResources);

    OCResourceHandle handle0;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle0,
                                            "core.led",
                                            "core.rw",
                                            "/a/led0",
                                            0,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResources(&numResources));
    EXPECT_EQ(1, numResources);

    EXPECT_EQ(OC_STACK_OK, OCDeleteResource(handle0));
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResources(&numResources));
    EXPECT_EQ(0, numResources);

    EXPECT_EQ(OC_STACK_OK, OCStop());
}

TEST(StackTest, DeleteHeadResource2) {
    uint8_t addr[20];
    uint16_t port = USE_RANDOM_PORT;
    uint8_t ifname[] = "eth0";

    /*Get Ip address on defined interface and initialize coap on it with random port number
     * this port number will be used as a source port in all coap communications*/
    OCGetInterfaceAddress(ifname, sizeof(ifname), AF_INET, addr, sizeof(addr));

    OC_LOG_V(INFO, TAG, "Starting ocserver on address %s:%d",addr,port);
    EXPECT_EQ(OC_STACK_OK, OCInit((char *) addr, port, OC_SERVER));

    uint8_t numResources = 0;
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResources(&numResources));
    EXPECT_EQ(0, numResources);

    OCResourceHandle handle0;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle0,
                                            "core.led",
                                            "core.rw",
                                            "/a/led0",
                                            0,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResources(&numResources));
    EXPECT_EQ(1, numResources);

    OCResourceHandle handle1;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle1,
                                            "core.led",
                                            "core.rw",
                                            "/a/led1",
                                            0,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResources(&numResources));
    EXPECT_EQ(2, numResources);

    EXPECT_EQ(OC_STACK_OK, OCDeleteResource(handle0));
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResources(&numResources));
    EXPECT_EQ(1, numResources);

    EXPECT_EQ(handle1, OCGetResourceHandle(0));

    EXPECT_EQ(OC_STACK_OK, OCStop());
}


TEST(StackTest, DeleteLastResource) {
    uint8_t addr[20];
    uint16_t port = USE_RANDOM_PORT;
    uint8_t ifname[] = "eth0";

    /*Get Ip address on defined interface and initialize coap on it with random port number
     * this port number will be used as a source port in all coap communications*/
    OCGetInterfaceAddress(ifname, sizeof(ifname), AF_INET, addr, sizeof(addr));

    OC_LOG_V(INFO, TAG, "Starting ocserver on address %s:%d",addr,port);
    EXPECT_EQ(OC_STACK_OK, OCInit((char *) addr, port, OC_SERVER));

    uint8_t numResources = 0;
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResources(&numResources));
    EXPECT_EQ(0, numResources);

    OCResourceHandle handle0;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle0,
                                            "core.led",
                                            "core.rw",
                                            "/a/led0",
                                            0,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResources(&numResources));
    EXPECT_EQ(1, numResources);

    OCResourceHandle handle1;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle1,
                                            "core.led",
                                            "core.rw",
                                            "/a/led1",
                                            0,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResources(&numResources));
    EXPECT_EQ(2, numResources);

    EXPECT_EQ(OC_STACK_OK, OCDeleteResource(handle1));
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResources(&numResources));
    EXPECT_EQ(1, numResources);

    EXPECT_EQ(handle0, OCGetResourceHandle(0));

    OCResourceHandle handle2;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle2,
                                            "core.led",
                                            "core.rw",
                                            "/a/led2",
                                            0,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResources(&numResources));
    EXPECT_EQ(2, numResources);

    EXPECT_EQ(OC_STACK_OK, OCStop());
}

TEST(StackTest, DeleteMiddleResource) {
    uint8_t addr[20];
    uint16_t port = USE_RANDOM_PORT;
    uint8_t ifname[] = "eth0";

    /*Get Ip address on defined interface and initialize coap on it with random port number
     * this port number will be used as a source port in all coap communications*/
    OCGetInterfaceAddress(ifname, sizeof(ifname), AF_INET, addr, sizeof(addr));

    OC_LOG_V(INFO, TAG, "Starting ocserver on address %s:%d",addr,port);
    EXPECT_EQ(OC_STACK_OK, OCInit((char *) addr, port, OC_SERVER));

    uint8_t numResources = 0;
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResources(&numResources));
    EXPECT_EQ(0, numResources);

    OCResourceHandle handle0;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle0,
                                            "core.led",
                                            "core.rw",
                                            "/a/led0",
                                            0,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResources(&numResources));
    EXPECT_EQ(1, numResources);

    OCResourceHandle handle1;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle1,
                                            "core.led",
                                            "core.rw",
                                            "/a/led1",
                                            0,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResources(&numResources));
    EXPECT_EQ(2, numResources);

    OCResourceHandle handle2;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle2,
                                            "core.led",
                                            "core.rw",
                                            "/a/led2",
                                            0,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResources(&numResources));
    EXPECT_EQ(3, numResources);

    EXPECT_EQ(OC_STACK_OK, OCDeleteResource(handle1));
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResources(&numResources));
    EXPECT_EQ(2, numResources);

    EXPECT_EQ(handle0, OCGetResourceHandle(0));
    EXPECT_EQ(handle2, OCGetResourceHandle(1));

    // Make sure the resource elements are still correct
    uint8_t numResourceInterfaces;
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResourceInterfaces(handle2, &numResourceInterfaces));
    EXPECT_EQ(1, numResourceInterfaces);
    const char *resourceInterfaceName = OCGetResourceInterfaceName(handle2, 0);
    EXPECT_STREQ("core.rw", resourceInterfaceName);

    EXPECT_EQ(OC_STACK_OK, OCStop());
}

TEST(StackTest, GetResourceProperties) {
    uint8_t addr[20];
    uint16_t port = USE_RANDOM_PORT;
    uint8_t ifname[] = "eth0";

    /*Get Ip address on defined interface and initialize coap on it with random port number
     * this port number will be used as a source port in all coap communications*/
    OCGetInterfaceAddress(ifname, sizeof(ifname), AF_INET, addr, sizeof(addr));

    OC_LOG_V(INFO, TAG, "Starting ocserver on address %s:%d",addr,port);
    EXPECT_EQ(OC_STACK_OK, OCInit((char *) addr, port, OC_SERVER));

    OCResourceHandle handle;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle,
                                            "core.led",
                                            "core.rw",
                                            "/a/led",
                                            0,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));

    EXPECT_EQ(OC_ACTIVE|OC_DISCOVERABLE|OC_OBSERVABLE, OCGetResourceProperties(handle));
    EXPECT_EQ(OC_STACK_OK, OCDeleteResource(handle));

    EXPECT_EQ(OC_STACK_OK, OCStop());
}

TEST(StackTest, StackTestResourceDiscoverOneResourceBad) {
    uint8_t addr[20];
    uint16_t port = USE_RANDOM_PORT;
    uint8_t ifname[] = "eth0";

    /*Get Ip address on defined interface and initialize coap on it with random port number
     * this port number will be used as a source port in all coap communications*/
    OCGetInterfaceAddress(ifname, sizeof(ifname), AF_INET, addr, sizeof(addr));

    OC_LOG_V(INFO, TAG, "Starting ocserver on address %s:%d",addr,port);
    EXPECT_EQ(OC_STACK_OK, OCInit((char *) addr, port, OC_SERVER));

    OCResourceHandle handle;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle,
                                            "core.led",
                                            "core.rw",
                                            "/a1/led",
                                            0,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));
    const char *url = OCGetResourceUri(handle);
    EXPECT_STREQ("/a1/led", url);

    //EXPECT_EQ(OC_STACK_INVALID_URI, OCHandleServerRequest(&res, uri, query, req, rsp));
    EXPECT_EQ(OC_STACK_OK, OCDeleteResource(handle));
    uint8_t numResources = 0;
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResources(&numResources));
    EXPECT_EQ(0, numResources);

    EXPECT_EQ(OC_STACK_OK, OCStop());
}

TEST(StackTest, StackTestResourceDiscoverOneResource) {
    uint8_t addr[20];
    uint16_t port = USE_RANDOM_PORT;
    uint8_t ifname[] = "eth0";

    /*Get Ip address on defined interface and initialize coap on it with random port number
     * this port number will be used as a source port in all coap communications*/
    OCGetInterfaceAddress(ifname, sizeof(ifname), AF_INET, addr, sizeof(addr));

    OC_LOG_V(INFO, TAG, "Starting ocserver on address %s:%d",addr,port);
    EXPECT_EQ(OC_STACK_OK, OCInit((char *) addr, port, OC_SERVER));

    OCResourceHandle handle;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle,
                                            "core.led",
                                            "core.rw",
                                            "/a/led",
                                            0,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));
    const char *url = OCGetResourceUri(handle);
    EXPECT_STREQ("/a/led", url);

    //EXPECT_EQ(OC_STACK_OK, OCHandleServerRequest(&res, uri, query, req, rsp));
    EXPECT_EQ(OC_STACK_OK, OCDeleteResource(handle));

    EXPECT_EQ(OC_STACK_OK, OCStop());
}

TEST(StackTest, StackTestResourceDiscoverManyResources) {
    uint8_t addr[20];
    uint16_t port = USE_RANDOM_PORT;
    uint8_t ifname[] = "eth0";

    /*Get Ip address on defined interface and initialize coap on it with random port number
     * this port number will be used as a source port in all coap communications*/
    OCGetInterfaceAddress(ifname, sizeof(ifname), AF_INET, addr, sizeof(addr));

    OC_LOG_V(INFO, TAG, "Starting ocserver on address %s:%d",addr,port);
    EXPECT_EQ(OC_STACK_OK, OCInit((char *) addr, port, OC_SERVER));

    OCResourceHandle handle1;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle1,
                                            "core.led",
                                            "core.rw",
                                            "/a/led1",
                                            0,
                                            OC_DISCOVERABLE));
    const char *url = OCGetResourceUri(handle1);
    EXPECT_STREQ("/a/led1", url);

    OCResourceHandle handle2;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle2,
                                            "core.led",
                                            "core.rw",
                                            "/a/led2",
                                            0,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));
    url = OCGetResourceUri(handle2);
    EXPECT_STREQ("/a/led2", url);

    EXPECT_EQ(OC_STACK_OK, OCBindResourceTypeToResource(handle2, "core.brightled"));
    EXPECT_EQ(OC_STACK_OK, OCBindResourceTypeToResource(handle2, "core.colorled"));

    OCResourceHandle handle3;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle3,
                                            "core.led",
                                            "core.rw",
                                            "/a/led3",
                                            0,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));
    url = OCGetResourceUri(handle3);
    EXPECT_STREQ("/a/led3", url);

    EXPECT_EQ(OC_STACK_OK, OCBindResourceInterfaceToResource(handle3, "oc.mi.ll"));
    EXPECT_EQ(OC_STACK_OK, OCBindResourceInterfaceToResource(handle3, "oc.mi.b"));

    OCResourceHandle handle4;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle4,
                                            "core.led",
                                            "core.rw",
                                            "/a/led4",
                                            0,
                                            OC_DISCOVERABLE));
    url = OCGetResourceUri(handle4);
    EXPECT_STREQ("/a/led4", url);

    EXPECT_EQ(OC_STACK_OK, OCBindResourceTypeToResource(handle4, "core.brightled"));
    EXPECT_EQ(OC_STACK_OK, OCBindResourceInterfaceToResource(handle4, "oc.mi.ll"));
    EXPECT_EQ(OC_STACK_OK, OCBindResourceInterfaceToResource(handle4, "oc.mi.b"));

    //EXPECT_EQ(OC_STACK_OK, OCHandleServerRequest(&res, uri, query, req, rsp));

    EXPECT_EQ(OC_STACK_OK, OCStop());
}


#if 0
TEST(StackTest, StackTestResourceDiscoverIfFilteringBad) {
    uint8_t addr[20];
    uint16_t port = USE_RANDOM_PORT;
    uint8_t ifname[] = "eth0";
    char uri[] = "/oc/core";
    char query[] = "if";
    char req[1024] = {};
    char rsp[1024] = {};
    //OCServerRequestResult res;

    //EXPECT_EQ(OC_STACK_INVALID_QUERY, OCHandleServerRequest(&res, uri, query, req, rsp));
}

TEST(StackTest, StackTestResourceDiscoverRtFilteringBad) {
    uint8_t addr[20];
    uint16_t port = USE_RANDOM_PORT;
    uint8_t ifname[] = "eth0";
    char uri[] = "/oc/core";
    char query[] = "rt";
    char req[1024] = {};
    char rsp[1024] = {};
    //OCServerRequestResult res;

    //EXPECT_EQ(OC_STACK_INVALID_QUERY, OCHandleServerRequest(&res, uri, query, req, rsp));
}
TEST(StackTest, StackTestResourceDiscoverIfFiltering) {
    uint8_t addr[20];
    uint16_t port = USE_RANDOM_PORT;
    uint8_t ifname[] = "eth0";
    char uri[] = "/oc/core";
    char query[] = "if=oc.mi.ll";
    char req[1024] = {};
    char rsp[1024] = {};
    //OCServerRequestResult res;

    //EXPECT_EQ(OC_STACK_OK, OCHandleServerRequest(&res, uri, query, req, rsp));
}

TEST(StackTest, StackTestResourceDiscoverRtFiltering) {
    uint8_t addr[20];
    uint16_t port = USE_RANDOM_PORT;
    uint8_t ifname[] = "eth0";
    char uri[] = "/oc/core";
    char query[] = "rt=core.brightled";
    char req[1024] = {};
    char rsp[1024] = {};
    //OCServerRequestResult res;

    //EXPECT_EQ(OC_STACK_OK, OCHandleServerRequest(&res, uri, query, req, rsp));
}
#endif
