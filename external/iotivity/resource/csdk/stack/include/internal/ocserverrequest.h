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

#ifndef OC_SERVER_REQUEST_H
#define OC_SERVER_REQUEST_H

#include "occoap.h"

/**
 * The signature of the internal call back functions to handle responses from entity handler
 */
typedef OCStackResult (* OCEHResponseHandler)(OCEntityHandlerResponse * ehResponse);
OCStackResult HandleSingleResponse(OCEntityHandlerResponse * ehResponse);
OCStackResult HandleAggregateResponse(OCEntityHandlerResponse * ehResponse);

// following structure will be created in occoap and passed up the stack on the server side
typedef struct OCServerRequest {
    // the REST method retrieved from received request PDU
    OCMethod method;
    // resourceUrl will be filled in occoap using the path options in received request PDU
    unsigned char resourceUrl[MAX_URI_LENGTH];
    // resource query send by client
    unsigned char query[MAX_QUERY_LENGTH];

    // qos is indicating if the request is CON or NON
    OCQualityOfService qos;
    // Observe option field
    uint32_t observationOption;
    OCStackResult observeResult;
    uint8_t numResponses;
    OCEHResponseHandler ehResponseHandler;
    //////////////////////////////////////////////////////////
    // IP address & port of client registered for observe   //These
    OCDevAddr requesterAddr;                                //Members
    // CoAP token for the observe request                   //Might
    OCCoAPToken requestToken;                               //Be
    // The ID of CoAP pdu                                   //Kept in
    uint16_t coapID;                                        //CoAP
    uint8_t delayedResNeeded;
    uint8_t secured;
    //////////////////////////////////////////////////////////
    // An array of the received vendor specific header options
    uint8_t numRcvdVendorSpecificHeaderOptions;
    OCHeaderOption rcvdVendorSpecificHeaderOptions[MAX_HEADER_OPTIONS];
    uint8_t requestComplete;
    struct OCServerRequest * next;
    // Flag indicating slow response
    uint8_t slowFlag;
    uint8_t notificationFlag;
    // reqJSON is retrieved from the payload of the received request PDU
    unsigned char reqJSONPayload[1];
} OCServerRequest;

// following structure will be created in ocstack to aggregate responses (in future: for block transfer)
typedef struct OCServerResponse {
    struct OCServerResponse * next;
    // this is the pointer to server payload data to be transferred
    unsigned char *payload;
    uint16_t remainingPayloadSize;
    OCRequestHandle requestHandle;
} OCServerResponse;

OCServerRequest * GetServerRequestUsingToken (const OCCoAPToken token);

OCServerRequest * GetServerRequestUsingHandle (const OCServerRequest * handle);

OCServerResponse * GetServerResponseUsingHandle (const OCServerRequest * handle);

OCStackResult AddServerRequest (OCServerRequest ** request, uint16_t coapID,
        uint8_t delayedResNeeded, uint8_t secured, uint8_t notificationFlag, OCMethod method,
        uint8_t numRcvdVendorSpecificHeaderOptions, uint32_t observationOption,
        OCQualityOfService qos, unsigned char * query,
        OCHeaderOption * rcvdVendorSpecificHeaderOptions,
        unsigned char * reqJSONPayload, OCCoAPToken * requestToken,
        OCDevAddr * requesterAddr, unsigned char * resourceUrl, uint32_t reqTotalSize);

OCStackResult AddServerResponse (OCServerResponse ** response, OCRequestHandle requestHandle);

// Internal function to create OCEntityHandlerRequest at the server from a received coap pdu
OCStackResult FormOCEntityHandlerRequest(OCEntityHandlerRequest * entityHandlerRequest, OCRequestHandle request,
        OCMethod method, OCResourceHandle resource, unsigned char * queryBuf, unsigned char * bufReqPayload,
        uint8_t numVendorOptions, OCHeaderOption * vendorOptions, OCObserveAction observeAction,
        OCObservationId observeID);

void FindAndDeleteServerRequest(OCServerRequest * serverRequest);

void DeleteServerRequest(OCServerRequest * serverRequest);

void FindAndDeleteServerResponse(OCServerResponse * serverResponse);

void DeleteServerResponse(OCServerResponse * serverResponse);

#endif //OC_SERVER_REQUEST_H
