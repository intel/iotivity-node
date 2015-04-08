//******************************************************************
///
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

#ifndef OCCOAP_H_
#define OCCOAP_H_

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "ocstack.h"
#include "occoaptoken.h"
#include "ocstackinternal.h"
#include "occoaphelper.h"
#include <stdint.h>

//-----------------------------------------------------------------------------
// Function Prototypes
//-----------------------------------------------------------------------------

uint32_t GetTime(float afterSeconds);

/**
 * Initialize the CoAP client or server with the its IPv4 address and CoAP port
 *
 * @param ipAddr
 *     IP Address of host device
 * @param port
 *     Port of host device
 * @param mode
 *     Host device is client, server, or client-server
 *
 * @return
 *   0   - success
 *   TBD - TBD error
 */
OCStackResult OCInitCoAP(const char *address, uint16_t port, OCMode mode);

/**
 * Discover OC resources
 *
 * @param method          - method to perform on the resource
 * @param qos             - CON or NON requests
 * @param token           - pointer to the token data structure
 * @param Uri             - URI of the resource to interact with
 * @param payload         - CoAP PDU payload
 * @param options         - The address of an array containing the vendor specific
 *                          header options to be sent with the request
 * @param numOptions      - The number of header options to be included
 * @return
 *   0   - success
 *   TBD - TBD error
 */
OCStackResult OCDoCoAPResource(OCMethod method, OCQualityOfService qos, OCCoAPToken * token,
        const char *Uri, const char *payload, OCHeaderOption * options, uint8_t numOptions);

/**
 * Send a response to a request.
 *
 * @param response - pointer to OCServerProtocolResponse that contains all request and
 *                   response info necessary to send the response to the client
 * @return
 *     OC_STACK_OK               - No errors; Success
 *     OC_STACK_ERROR            - Error sending response
 */
OCStackResult OCDoCoAPResponse(OCServerProtocolResponse *response);

/**
 * Stop the CoAP client or server processing
 *
 * @return 0 - success, else - TBD error
 */
OCStackResult OCStopCoAP();

/**
 * Called in main loop of CoAP client or server.  Allows low-level CoAP processing of
 * send, receive, timeout, discovery, callbacks, etc.
 *
 * @return 0 - success, else - TBD error
 */
OCStackResult OCProcessCoAP();

/**
 * This method is called to generate a token of MAX_TOKEN_LENGTH.
 * This token is used to co-relate client requests with server responses.
 *
 */
void OCGenerateCoAPToken(OCCoAPToken * token);

/**
 * Retrieve the end-point info where resource is being hosted.
 * Currently, this method only provides the IP port with which the socket
 * is bound. This internal method may be extended in future to retrieve
 * more info (such as IP address, transport technology) about the hosting end-point.
 *
 * @param resPtr    - pointer to the resource
 * @param info      - pointer to 16-bit integer to hold port number
 * @return 0 - success, else - TBD error
 */
OCStackResult OCGetResourceEndPointInfo (OCResource *resPtr, void *info);

#endif /* OCCOAP_H_ */
