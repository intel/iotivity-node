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

#ifndef OC_OBSERVE_H
#define OC_OBSERVE_H

/* In CoAP sequence number is a 24 bit field */
#define MAX_SEQUENCE_NUMBER              (0xFFFFFF)

#define MAX_OBSERVER_FAILED_COMM         (2)
#define MAX_OBSERVER_NON_COUNT           (3)

/* This information is stored for each registerd observer */
typedef struct ResourceObserver {
    // Observation Identifier for request
    OCObservationId observeId;
    // URI of observed resource
    unsigned char *resUri;
    // Query
    unsigned char *query;
    // CoAP token for the observe request
    OCCoAPToken token;
    // Resource handle
    OCResource *resource;
    // IP address & port of client registered for observe
    OCDevAddr *addr;
    // Quality of service of the request
    OCQualityOfService qos;
    // number of times the server failed to reach the observer
    uint8_t failedCommCount;
    // number of times the server sent NON notifications
    uint8_t lowQosCount;
    // force the qos value to CON
    uint8_t forceHighQos;
    // next node in this list
    struct ResourceObserver *next;
} ResourceObserver;

#ifdef WITH_PRESENCE
OCStackResult SendAllObserverNotification (OCMethod method, OCResource *resPtr, uint32_t maxAge,
        OCResourceType *resourceType, OCQualityOfService qos);
#else
OCStackResult SendAllObserverNotification (OCMethod method, OCResource *resPtr, uint32_t maxAge,
        OCQualityOfService qos);
#endif
OCStackResult SendListObserverNotification (OCResource * resource,
        OCObservationId  *obsIdList, uint8_t numberOfIds,
        unsigned char *notificationJSONPayload, uint32_t maxAge,
        OCQualityOfService qos);

void DeleteObserverList();

OCStackResult GenerateObserverId (OCObservationId *observationId);

OCStackResult AddObserver (const char         *resUri,
                           const char         *query,
                           OCObservationId    obsId,
                           OCCoAPToken        *token,
                           OCDevAddr          *addr,
                           OCResource         *resHandle,
                           OCQualityOfService qos);

OCStackResult DeleteObserverUsingToken (OCCoAPToken * token);

ResourceObserver* GetObserverUsingToken (const OCCoAPToken * token);

ResourceObserver* GetObserverUsingId (const OCObservationId observeId);

#endif //OC_OBSERVE_H
