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

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "occoaphelper.h"
#include "ocstackconfig.h"
#include "logger.h"
#include "coap_time.h"
#include "ocmalloc.h"

//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------
#define TAG    PCF("OCCoAPHelper")
#define VERIFY_NON_NULL(arg) { if (!arg) {OC_LOG_V(FATAL, TAG, "%s is NULL", #arg); goto exit;} }

//=============================================================================
// Helper Functions
//=============================================================================

OCStackResult isVendorSpecific(uint16_t optionID)
{
    if(optionID >= COAP_VENDOR_OPT_START && optionID <= COAP_MAX_OPT)
    {
        return OC_STACK_OK;
    }
    return OC_STACK_INVALID_OPTION;
}

// Convert OCStack code to CoAP code
uint8_t OCToCoAPResponseCode(OCStackResult result)
{
    uint8_t ret;
    switch(result)
    {
        case OC_STACK_OK :
            ret = COAP_RESPONSE_200;
            break;

        case OC_STACK_RESOURCE_CREATED:
            ret = COAP_RESPONSE_201;
            break;

        case OC_STACK_RESOURCE_DELETED:
            ret = COAP_RESPONSE_202;
            break;

        case OC_STACK_INVALID_QUERY :
            ret = COAP_RESPONSE_400;
            break;

        case OC_STACK_RESOURCE_ERROR:
            return COAP_RESPONSE_403;
            break;

        case OC_STACK_NO_RESOURCE :
            ret = COAP_RESPONSE_404;
            break;

        case OC_STACK_INVALID_METHOD :
            ret = COAP_RESPONSE_405;
            break;

        case OC_STACK_NOTIMPL :
            ret = COAP_RESPONSE_501;
            break;

        default:
            ret = COAP_RESPONSE_500;
    }
    return ret;
}

uint8_t OCToCoAPQoS(OCQualityOfService qos, uint8_t * ipAddr)
{
    if(ipAddr[0] == COAP_WK_IPAddr_0 && ipAddr[1] == COAP_WK_IPAddr_1 &&
            ipAddr[2] == COAP_WK_IPAddr_2 && ipAddr[3] == COAP_WK_IPAddr_3)
    {
        return COAP_MESSAGE_NON;
    }
    switch (qos)
    {
        case OC_HIGH_QOS:
            return COAP_MESSAGE_CON;
            break;
        case OC_MEDIUM_QOS:
        case OC_LOW_QOS:
        case OC_NA_QOS:
        default:
            return COAP_MESSAGE_NON;
            break;
    }
}
// Convert CoAP code to OCStack code
OCStackResult CoAPToOCResponseCode(uint8_t coapCode)
{
    OCStackResult ret;
    int decimal;
    switch(coapCode)
    {
        case COAP_RESPONSE_200 :
            ret = OC_STACK_OK;
            break;

        case COAP_RESPONSE_201 :
            ret = OC_STACK_RESOURCE_CREATED;
            break;

        case COAP_RESPONSE_202 :
            ret = OC_STACK_RESOURCE_DELETED;
            break;

        case COAP_RESPONSE_400 :
            ret = OC_STACK_INVALID_QUERY;
            break;

        case COAP_RESPONSE_403 :
            ret = OC_STACK_RESOURCE_ERROR;
            break;

        case COAP_RESPONSE_404 :
            ret = OC_STACK_NO_RESOURCE;
            break;

        case COAP_RESPONSE_405 :
            ret = OC_STACK_INVALID_METHOD;
            break;

        case COAP_RESPONSE_501 :
            ret = OC_STACK_NOTIMPL;
            break;

        default:
            decimal = ((coapCode >> 5) * 100) + (coapCode & 31);
            if (decimal >= 200 && decimal <= 231)
            {
                ret = OC_STACK_OK;
            }
            else
            {
                ret = OC_STACK_ERROR;
            }
    }
    return ret;
}

// Retrieve Uri and Query from received coap pdu
OCStackResult ParseCoAPPdu(coap_pdu_t * pdu, unsigned char * uriBuf,
        unsigned char * queryBuf, uint32_t * observeOption,
        uint32_t * maxAgeOption,
        uint8_t * numVendorSpecificHeaderOptions,
        OCHeaderOption * vendorSpecificHeaderOptions,
        coap_block_t * block1, coap_block_t * block2,
        uint16_t * size1, uint16_t * size2,
        unsigned char * payload)
{
    coap_opt_filter_t filter;
    coap_opt_iterator_t opt_iter;
    coap_opt_t *option = NULL;
    size_t optLen = 0;
    unsigned char * optVal = NULL;
    size_t uriBufLen = 0;
    size_t queryBufLen = 0;
    unsigned char * payloadLoc = NULL;
    size_t payloadLength = 0;

    coap_option_filter_clear(filter);
    if(uriBuf)
    {
        coap_option_setb(filter, COAP_OPTION_URI_PATH);
    }
    if(queryBuf)
    {
        coap_option_setb(filter, COAP_OPTION_URI_QUERY);
    }
    if(observeOption)
    {
        coap_option_setb(filter, COAP_OPTION_OBSERVE);
    }
    if(maxAgeOption)
    {
        coap_option_setb(filter, COAP_OPTION_MAXAGE);
    }
    if(block1)
    {
        coap_option_setb(filter, COAP_OPTION_BLOCK1);
    }
    if(block2)
    {
        coap_option_setb(filter, COAP_OPTION_BLOCK2);
    }
    if(size1)
    {
        coap_option_setb(filter, COAP_OPTION_SIZE1);
    }
    if(size2)
    {
        coap_option_setb(filter, COAP_OPTION_SIZE2);
    }
    if(vendorSpecificHeaderOptions)
    {
        coap_option_setbVendor(filter);
    }
    if(payload)
    {
        coap_get_data(pdu, &payloadLength, &payloadLoc);
        memcpy(payload, payloadLoc, payloadLength);
    }
    coap_option_iterator_init(pdu, &opt_iter, filter);

    while ((option = coap_option_next(&opt_iter)))
    {
        optLen = COAP_OPT_LENGTH(option);
        optVal = COAP_OPT_VALUE(option);
        switch(opt_iter.type)
        {
        case COAP_OPTION_URI_PATH:
            if (uriBufLen + 1 + optLen < MAX_URI_LENGTH)
            {
                //we still have room in the buffer
                uriBuf[uriBufLen++] = '/';
                memcpy(uriBuf + uriBufLen, optVal, optLen);
                uriBufLen += optLen;
            }
            else
            {
                return OC_STACK_NO_MEMORY;
            }
            break;
        case COAP_OPTION_URI_QUERY:
            if (queryBufLen + 1 + optLen < MAX_QUERY_LENGTH)
            {
                //we still have room in the buffer
                memcpy(queryBuf + queryBufLen, optVal, optLen);
                queryBufLen += optLen;
                queryBuf[queryBufLen++] = '&';
            }
            else
            {
                // TODO: we should check that resources do not have long uri
                // at the resource creation
                return OC_STACK_NO_MEMORY;
            }
            break;
        case COAP_OPTION_OBSERVE:
            memcpy(observeOption, optVal, optLen);
            OC_LOG_V(DEBUG, TAG, "^^^^^^^^^^^^Parsing the observe option %u",
                    *observeOption);
            break;
        case COAP_OPTION_MAXAGE:
            memcpy(maxAgeOption, optVal, optLen);
            OC_LOG_V(DEBUG, TAG, "^^^^^^^^^^^^Parsing the max age option %u",
                    *maxAgeOption);
            break;
        case COAP_OPTION_BLOCK1:
            block1->szx = COAP_OPT_BLOCK_SZX(option);
            block1->num = coap_opt_block_num(option);
            block1->m = 0;
            if(COAP_OPT_BLOCK_MORE(option))
            {
                block1->m = 1;
            }
            OC_LOG_V(DEBUG, TAG, "^^^^^^^^^^^^Parsing block1 %u:%u:%u",
                    block1->num, block1->m, block1->szx);
            break;
        case COAP_OPTION_BLOCK2:
            block2->szx = COAP_OPT_BLOCK_SZX(option);
            block2->num = coap_opt_block_num(option);
            block2->m = 0;
            if(COAP_OPT_BLOCK_MORE(option))
            {
                block2->m = 1;
            }
            OC_LOG_V(DEBUG, TAG, "^^^^^^^^^^^^Parsing block2 %u:%u:%u",
                    block1->num, block1->m, block1->szx);
            break;
        case COAP_OPTION_SIZE1:
            break;
        case COAP_OPTION_SIZE2:
            break;
        default:
            if(*numVendorSpecificHeaderOptions >= MAX_HEADER_OPTIONS ||
                    optLen > MAX_HEADER_OPTION_DATA_LENGTH)
            {
                return OC_STACK_NO_MEMORY;
            }
            vendorSpecificHeaderOptions[*numVendorSpecificHeaderOptions].protocolID = OC_COAP_ID;
            vendorSpecificHeaderOptions[*numVendorSpecificHeaderOptions].optionID = opt_iter.type;
            vendorSpecificHeaderOptions[*numVendorSpecificHeaderOptions].optionLength = optLen;
            memcpy(vendorSpecificHeaderOptions[*numVendorSpecificHeaderOptions].optionData, optVal, optLen);
            OC_LOG_V(DEBUG, TAG, "^^^^^^^^^^^^Parsing vendor specific option %u",
                    vendorSpecificHeaderOptions[*numVendorSpecificHeaderOptions].optionID);
            OC_LOG_BUFFER(DEBUG, TAG, vendorSpecificHeaderOptions[*numVendorSpecificHeaderOptions].optionData,
                    vendorSpecificHeaderOptions[*numVendorSpecificHeaderOptions].optionLength);
            (*numVendorSpecificHeaderOptions)++;
        }
    }

    if(uriBuf)
    {
        uriBuf[uriBufLen] = '\0';
    }
    // delete last '&' in the query
    if(queryBuf)
    {
        queryBuf[queryBufLen?queryBufLen-1:queryBufLen] = '\0';
    }

    OC_LOG_V(DEBUG, TAG, "^^^^^^^^^^^^The final parsed uri is %s", uriBuf);
    OC_LOG_V(DEBUG, TAG, "^^^^^^^^^^^^The final parsed query is %s", queryBuf);
    return OC_STACK_OK;
}

// Retrieve the token from the PDU
void RetrieveOCCoAPToken(const coap_pdu_t * pdu, OCCoAPToken * rcvdToken)
{
    if (pdu && rcvdToken)
    {
        rcvdToken->tokenLength = pdu->hdr->token_length;
        memcpy(rcvdToken->token, pdu->hdr->token,
            rcvdToken->tokenLength);
    }
}

OCStackResult FormOCResponse(OCResponse * * responseLoc, ClientCB * cbNode,
        uint8_t TTL, OCClientResponse * clientResponse)
{
    OCResponse * response = (OCResponse *) OCMalloc(sizeof(OCResponse));
    if (!response)
    {
        return OC_STACK_NO_MEMORY;
    }
    response->cbNode = cbNode;
    response->TTL = TTL;
    response->clientResponse = clientResponse;

    *responseLoc = response;
    return OC_STACK_OK;
}

OCStackResult FormOCClientResponse(OCClientResponse * clientResponse,
        OCStackResult result, OCDevAddr * remote, uint32_t seqNum,
        const unsigned char * resJSONPayload)
{
    clientResponse->sequenceNumber = seqNum;
    clientResponse->result = result;
    clientResponse->addr = remote;
    clientResponse->resJSONPayload = resJSONPayload;

    return OC_STACK_OK;
}

OCStackResult FormOptionList(coap_list_t * * optListLoc, uint8_t * addMediaType,
        uint32_t * addMaxAge, uint32_t * observeOptionPtr,
        uint16_t * addPortNumber, uint8_t uriLength, unsigned char * uri,
        uint8_t queryLength, unsigned char * query,
        OCHeaderOption * vendorSpecificHeaderOptions,
        uint8_t numVendorSpecificHeaderOptions)
{
    coap_list_t * optNode = NULL;
    int res;
    size_t buflen;
    unsigned char _buf[MAX_URI_QUERY_BUF_SIZE];
    unsigned char *buf = _buf;

    if(addMediaType)
    {
        optNode = CreateNewOptionNode(COAP_OPTION_CONTENT_TYPE,
                sizeof(*addMediaType), addMediaType);
        VERIFY_NON_NULL(optNode);
        coap_insert(optListLoc, optNode, OrderOptions);
    }

    if(addMaxAge)
    {
        optNode = CreateNewOptionNode(COAP_OPTION_MAXAGE,
                sizeof(*addMaxAge), (uint8_t *)addMaxAge);
        VERIFY_NON_NULL(optNode);
        coap_insert(optListLoc, optNode, OrderOptions);
    }

    if(observeOptionPtr)
    {
        optNode = CreateNewOptionNode(COAP_OPTION_OBSERVE,
                sizeof(*observeOptionPtr), (uint8_t *)observeOptionPtr);

        VERIFY_NON_NULL(optNode);
        coap_insert(optListLoc, optNode, OrderOptions);
    }
    if(addPortNumber && *addPortNumber != COAP_DEFAULT_PORT)
    {
        optNode = CreateNewOptionNode(COAP_OPTION_URI_PORT,
                sizeof(*addPortNumber), (uint8_t *)addPortNumber);
        VERIFY_NON_NULL(optNode);
        coap_insert(optListLoc, optNode, OrderOptions);
    }

    if(uri && uriLength)
    {
        buf = _buf;
        buflen = MAX_URI_QUERY_BUF_SIZE;
        res = coap_split_path(uri, uriLength, buf, &buflen);
        while (res--) {
            optNode = CreateNewOptionNode(COAP_OPTION_URI_PATH,
                    COAP_OPT_LENGTH(buf), COAP_OPT_VALUE(buf));
            VERIFY_NON_NULL(optNode);
            coap_insert(optListLoc, optNode, OrderOptions);
            buf += COAP_OPT_SIZE(buf);
        }
    }

    if(query && queryLength)
    {
        buf = _buf;
        buflen = MAX_URI_QUERY_BUF_SIZE;
        res = coap_split_query(query, queryLength, buf, &buflen);
        while (res--) {
            optNode = CreateNewOptionNode(COAP_OPTION_URI_QUERY,
                    COAP_OPT_LENGTH(buf), COAP_OPT_VALUE(buf));
            VERIFY_NON_NULL(optNode);
            coap_insert(optListLoc, optNode, OrderOptions);
            buf += COAP_OPT_SIZE(buf);
        }
    }

    // make sure that options are valid
    if(vendorSpecificHeaderOptions && numVendorSpecificHeaderOptions)
    {
        uint8_t i = 0;
        for( i = 0; i < numVendorSpecificHeaderOptions; i++)
        {
            if(vendorSpecificHeaderOptions[i].protocolID == OC_COAP_ID)
            {
                if(isVendorSpecific(vendorSpecificHeaderOptions[i].optionID)
                        == OC_STACK_OK &&
                        vendorSpecificHeaderOptions[i].optionLength <=
                        MAX_HEADER_OPTION_DATA_LENGTH)
                {
                    OC_LOG_V(INFO, TAG, " Adding option %d with",
                            vendorSpecificHeaderOptions[i].optionID);
                    OC_LOG_BUFFER(INFO, TAG, vendorSpecificHeaderOptions[i].optionData,
                            vendorSpecificHeaderOptions[i].optionLength);
                    optNode = CreateNewOptionNode(vendorSpecificHeaderOptions[i].optionID,
                            vendorSpecificHeaderOptions[i].optionLength,
                            vendorSpecificHeaderOptions[i].optionData);
                    VERIFY_NON_NULL(optNode);
                    coap_insert(optListLoc, optNode, OrderOptions);
                }
                else
                {
                    coap_delete_list(*optListLoc);
                    return OC_STACK_INVALID_OPTION;
                }
            }
        }
    }

    return OC_STACK_OK;
    exit:
        coap_delete_list(*optListLoc);
        return OC_STACK_NO_MEMORY;
}

//Send a coap pdu
OCStackResult
SendCoAPPdu(coap_context_t * gCoAPCtx, coap_address_t* dst, coap_pdu_t * pdu,
        coap_send_flags_t flag)
{
    coap_tid_t tid = COAP_INVALID_TID;
    OCStackResult res = OC_STACK_COMM_ERROR;
    uint8_t cache = 0;

    if (!(flag & SEND_DELAYED))
    {
        flag = (coap_send_flags_t)( flag |
            ((pdu->hdr->type == COAP_MESSAGE_CON) ? SEND_NOW_CON : SEND_NOW));
    }

    tid = coap_send(gCoAPCtx, dst, pdu, flag, &cache);
    OC_LOG_V(INFO, TAG, "TID %d", tid);
    if(tid != COAP_INVALID_TID)
    {
        OC_LOG(INFO, TAG, PCF("Sending a pdu with Token:"));
        OC_LOG_BUFFER(INFO,TAG, pdu->hdr->token, pdu->hdr->token_length);
        res = OC_STACK_OK;
    }

    if (( (pdu->hdr->type != COAP_MESSAGE_CON) && (!(flag & SEND_DELAYED)) && (!cache))
        || (tid == COAP_INVALID_TID))
    {
        OC_LOG(INFO, TAG, PCF("Deleting PDU"));
        coap_delete_pdu(pdu);
    }
    else
    {
        OC_LOG(INFO, TAG, PCF("Keeping PDU, we will handle the retry/delay of this pdu"));
    }

    return res;
}

//generate a coap message
coap_pdu_t *
GenerateCoAPPdu(uint8_t msgType, uint8_t code, unsigned short id,
        OCCoAPToken * token, unsigned char * payloadJSON,
        coap_list_t *options)
{
    coap_pdu_t *pdu;
    coap_list_t *opt;

    if(token)
    {
        pdu = coap_pdu_init(msgType, code, id, COAP_MAX_PDU_SIZE);
        VERIFY_NON_NULL(pdu);
        pdu->hdr->token_length = token->tokenLength;
         if (!coap_add_token(pdu, token->tokenLength, token->token))
         {
            OC_LOG(FATAL, TAG, PCF("coap_add_token failed"));
        }
    }
    else
    {
        pdu = coap_pdu_init(msgType, code, id, sizeof(coap_pdu_t));
        VERIFY_NON_NULL(pdu);
    }

    for (opt = options; opt; opt = opt->next)
    {
        coap_add_option(pdu, COAP_OPTION_KEY(*(coap_option *) opt->data),
                COAP_OPTION_LENGTH(*(coap_option *) opt->data),
                COAP_OPTION_DATA(*(coap_option *) opt->data));
    }

    if (payloadJSON)
    {
        coap_add_data(pdu, strlen((const char *) payloadJSON) + 1,
                (unsigned char*) payloadJSON);
    }

    // display the pdu for debugging purposes
    coap_show_pdu(pdu);

    // clean up
    coap_delete_list(options);
    return pdu;

    exit:
    coap_delete_list(options);
    return NULL;
}

//a function to help in ordering coap options
int OrderOptions(void *a, void *b)
{
    if (!a || !b)
    {
        return a < b ? -1 : 1;
    }

    if (COAP_OPTION_KEY(*(coap_option *)a)
            < COAP_OPTION_KEY(*(coap_option *)b) )
    {
        return -1;
    }

    return COAP_OPTION_KEY(*(coap_option *)a)
            == COAP_OPTION_KEY(*(coap_option *)b) ;
}

//a function to create a coap option
coap_list_t *
CreateNewOptionNode(unsigned short key, unsigned int length, unsigned char *data)
{
    coap_option *option = NULL;
    coap_list_t *node;

    VERIFY_NON_NULL(data);
    option = (coap_option *)coap_malloc(sizeof(coap_option) + length);
    VERIFY_NON_NULL(option);

    COAP_OPTION_KEY(*option) = key;
    COAP_OPTION_LENGTH(*option) = length;
    memcpy(COAP_OPTION_DATA(*option), data, length);

    /* we can pass NULL here as delete function since option is released automatically  */
    node = coap_new_listnode(option, NULL);

    if (node)
    {
        return node;
    }

exit:
    OC_LOG(ERROR,TAG, PCF("new_option_node: malloc: was not created"));
    coap_free(option);
    return NULL;
}

OCStackResult ReTXCoAPQueue(coap_context_t * ctx, coap_queue_t * queue)
{
    coap_tid_t tid = COAP_INVALID_TID;
    OCStackResult result = OC_STACK_ERROR;
    tid = coap_retransmit( ctx, queue);
    if(tid == COAP_INVALID_TID)
    {
        OC_LOG_V(DEBUG, TAG, "Retransmission Failed TID %d",
                queue->id);
        result = OC_STACK_COMM_ERROR;
    }
    else
    {
        OC_LOG_V(DEBUG, TAG, "Retransmission TID %d, this is attempt %d",
                queue->id, queue->retransmit_cnt);
        result = OC_STACK_OK;
    }
    return result;
}

OCStackResult HandleFailedCommunication(coap_context_t * ctx, coap_queue_t * queue)
{
    //TODO: this function should change to only use OCStackFeedBack
    OCResponse * response = NULL;
    ClientCB * cbNode = NULL;
    OCClientResponse clientResponse;
    OCCoAPToken token;
    OCStackResult result = OC_STACK_OK;

    RetrieveOCCoAPToken(queue->pdu, &token);

    cbNode = GetClientCB(&token, NULL, NULL);
    if(!cbNode)
    {
        goto observation;
    }
    result = FormOCClientResponse(&clientResponse, OC_STACK_COMM_ERROR,
            (OCDevAddr *) &(queue->remote), 0, NULL);
    if(result != OC_STACK_OK)
    {
        goto observation;
    }
    result = FormOCResponse(&response, cbNode, 0, &clientResponse);
    if(result != OC_STACK_OK)
    {
        goto observation;
    }
    HandleStackResponses(response);

observation:
    result = OCStackFeedBack(&token, OC_OBSERVER_FAILED_COMM);
    if(result == OC_STACK_OK)
    {
        coap_cancel_all_messages(ctx, &queue->remote, token.token, token.tokenLength);
    }
    OCFree(response);
    return result;
}

// a function to handle the send queue in the passed context
void HandleSendQueue(coap_context_t * ctx)
{
    coap_tick_t now;
    coap_queue_t *nextQueue = NULL;

    coap_ticks(&now);
    nextQueue = coap_peek_next( ctx );
    while (nextQueue && nextQueue->t <= now - ctx->sendqueue_basetime)
    {
        nextQueue = coap_pop_next( ctx );
        if((uint8_t)nextQueue->delayedResNeeded)
        {
            OC_LOG_V(DEBUG, TAG, "Sending Delayed response TID %d",
                    nextQueue->id);
            if(SendCoAPPdu(ctx, &nextQueue->remote, nextQueue->pdu,
                 (coap_send_flags_t)(nextQueue->secure ? SEND_SECURE_PORT : 0))
                    == OC_STACK_COMM_ERROR)
            {
                OC_LOG(DEBUG, TAG, PCF("A problem occurred in sending a pdu"));
                HandleFailedCommunication(ctx, nextQueue);
            }
            nextQueue->pdu = NULL;
            coap_delete_node(nextQueue);
        }
        else
        {
            OC_LOG_V(DEBUG, TAG, "Retrying a CON pdu TID %d",nextQueue->id);
            if(ReTXCoAPQueue(ctx, nextQueue) == OC_STACK_COMM_ERROR)
            {
                OC_LOG(DEBUG, TAG, PCF("A problem occurred in retransmitting a pdu"));
                HandleFailedCommunication(ctx, nextQueue);
                coap_delete_node(nextQueue);
            }
        }
        nextQueue = coap_peek_next( ctx );
    }
}
