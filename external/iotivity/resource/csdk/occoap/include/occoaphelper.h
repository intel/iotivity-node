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
//

#ifndef OCCOAPHELPER_H_
#define OCCOAPHELPER_H_

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#ifndef WITH_ARDUINO
#include <unistd.h>
#endif
#include <limits.h>
#include <ctype.h>
#include "coap.h"
#include "ocstack.h"
#include "occoaptoken.h"
#include "ocstackinternal.h"

/**
 * The Max Size of the buffer that is used to parse uri and query
 * individually.
 */
#define MAX_URI_QUERY_BUF_SIZE ((MAX_URI_LENGTH) >= (MAX_QUERY_LENGTH) ?\
                            (MAX_URI_LENGTH) : (MAX_QUERY_LENGTH))

#define COAP_WK_IPAddr_0 (224)
#define COAP_WK_IPAddr_1 (0)
#define COAP_WK_IPAddr_2 (1)
#define COAP_WK_IPAddr_3 (187)

// checks if optionID is within vendor specific range
OCStackResult isVendorSpecific(uint16_t optionID);

// Convert OCStack code to CoAP code
uint8_t OCToCoAPResponseCode(OCStackResult result);

//Convert OCQualityOfService to coap NON vs CON
uint8_t OCToCoAPQoS(OCQualityOfService qos, uint8_t * ipAddr);

// Convert CoAP code to OCStack code
OCStackResult CoAPToOCResponseCode(uint8_t coapCode);

// Internal function to generate a coap pdu based on passed parameters
coap_pdu_t *
GenerateCoAPPdu(uint8_t msgType, uint8_t code, unsigned short id,
        OCCoAPToken * token, unsigned char * payloadJSON,
        coap_list_t *options);

// Internal function to send a coap pdu, it also handles NON and CON
OCStackResult
SendCoAPPdu(coap_context_t * gCoAPCtx, coap_address_t* dst, coap_pdu_t * pdu,
        coap_send_flags_t flag);

// Call back function used by libcoap to order option in coap pdu
int OrderOptions(void *a, void *b);

// Internal function to create an option node for coap pdu
coap_list_t *
CreateNewOptionNode(unsigned short key, unsigned int length,
        unsigned char *data);

// Internal function to retrieve Uri and Query from received coap pdu
OCStackResult ParseCoAPPdu(coap_pdu_t * pdu, unsigned char * uriBuf,
        unsigned char * queryBuf, uint32_t * observeOption,
        uint32_t * maxAgeOption,
        uint8_t * numVendorSpecificHeaderOptions,
        OCHeaderOption * vendorSpecificHeaderOptions,
        coap_block_t * block1, coap_block_t * block2,
        uint16_t * size1, uint16_t * size2,
        unsigned char * payload);

// Internal function to retrieve a Token from received coap pdu
void RetrieveOCCoAPToken(const coap_pdu_t * pdu, OCCoAPToken * rcvdToken);

// Internal function to create OCResponse struct at the client from a received coap pdu
OCStackResult FormOCResponse(OCResponse * * responseLoc, ClientCB * cbNode,
        uint8_t TTL, OCClientResponse * clientResponse);

// Internal function to create OCClientResponse struct at the client from a received coap pdu
OCStackResult FormOCClientResponse(OCClientResponse * clientResponse,
        OCStackResult result, OCDevAddr * remote, uint32_t seqNum,
        const unsigned char * resJSONPayload);

// Internal function to handle the queued pdus in the send queue
void HandleSendQueue(coap_context_t * gCoAPCtx);

// Internal function to form the standard response option list
OCStackResult FormOptionList(coap_list_t * * optListLoc, uint8_t * addMediaType,
        uint32_t * addMaxAge, uint32_t * observeOptionPtr,
        uint16_t * addPortNumber, uint8_t uriLength, unsigned char * uri,
        uint8_t queryLength, unsigned char * query, OCHeaderOption * vendorSpecificHeaderOptions,
        uint8_t numVendorSpecificHeaderOptions);

// Internal function to retransmit a queue
OCStackResult ReTXCoAPQueue(coap_context_t * ctx, coap_queue_t * queue);

// Internal function called when sending/retransmission fails
OCStackResult HandleFailedCommunication(coap_context_t * ctx, coap_queue_t * queue);
#endif /* OCCOAPHELPER_H_ */
