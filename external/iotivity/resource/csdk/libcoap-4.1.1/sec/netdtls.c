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

#include "netdtls.h"
#include "dtls.h"
#include "alert.h"
#include "debug.h"
#include "logger.h"
#include "mem.h"
#include "ocsecurityconfig.h"

#define MOD_NAME ("netdtls.c")

#define get_dtls_ctx(coap_ctx) (coap_ctx->coap_dtls_ctx->dtls_ctx)

extern void OCGetDtlsPskCredentials(OCDtlsPskCredsBlob **credInfo);

/**
 * An internal method to invoke tinyDTLS library 'dtls_write' method.
 * Return value from this method will indicate if data was successfully sent
 * to peer OR a new DTLS handshake session was invoked OR some error happened
 * while processing.
 *
 */
static dtls_ret coap_dtls_encrypt_internal(coap_context_t *ctx, OCDevAddr* dst,
            uint8_t *pt, uint16_t ptLen) {
    int ret;
    if (ptLen == 0)
        return DTLS_OK;

    ret = dtls_write(get_dtls_ctx(ctx), (session_t*)dst, pt, ptLen);
    if (ret == 0) {
        // A new DTLS session was initiated by tinyDTLS library
        return DTLS_SESSION_INITIATED;
    }

    if (ret == ptLen) {
        // tinyDTLS library successfully encrypted the data and
        // sent it to the peer.
        return DTLS_OK;
    }

    return DTLS_FAIL;
}

/**
 * An internal method to invoke tinyDTLS library 'dtls_handle_message' method
 * to decrypt packet received on secure port.
 * Return value from this method will indicate if a valid application pdu was
 * decypted OR a DTLS handshake message was received OR some error happened
 * while processing.
 *
 */
static dtls_ret coap_dtls_decrypt_internal(coap_context_t *ctx, OCDevAddr* src,
            uint8_t* ct, int ctLen, uint8_t** pt, int* ptLen) {
    dtls_ret ret = DTLS_FAIL;
    pt_info_t ptinfo;

    ptinfo.pt =NULL;
    ptinfo.ptlen = 0;
    ctx->coap_dtls_ctx->pt_info = &ptinfo;

    if (dtls_handle_message(get_dtls_ctx(ctx), (session_t*)src, ct, ctLen) == 0) {
        ret = DTLS_HS_MSG;
        if (ptinfo.pt && ptinfo.ptlen) {
            *pt = ptinfo.pt;
            *ptLen = ptinfo.ptlen;
            ret = DTLS_OK;
        }
    }
    return ret;
}


/**
 * If tinyDTLS library starts a new DTLS handshake session with a peer, the pdu
 * which was requested by application to encrypt will need to be cached until
 * DTLS session is established. This method caches the pdu in cachedqueue.
 *
 */
static int coap_cache_pdu(coap_context_t *ctx,
        coap_queue_t* existing_node,
        OCDevAddr *dst,
        coap_pdu_t *pdu,
        coap_tid_t tid)
{
    coap_queue_t *node;
    coap_tick_t  now;

    if (!ctx)
        return -1;
    /* Create a new node for caching the PDU in cachedqueue until
     * DTLS session is established with peer.
     */
    node = coap_new_node();
    if (!node) {
        OC_LOG(DEBUG, MOD_NAME, PCF("Unable to allocate memory"));
        return -1;
    }

    memcpy(&node->remote, dst, sizeof(coap_address_t));
    node->pdu = pdu;
    node->id = tid;
    node->secure = 1;

    coap_ticks(&now);
    node->t = now + (COAP_DEFAULT_RESPONSE_TIMEOUT *2) * COAP_TICKS_PER_SECOND;

    if (existing_node) {
        node->timeout = existing_node->timeout;
        node->delayedResNeeded = existing_node->delayedResNeeded;
    }

    // Add the node in cachedqueue list
    // TODO : Do we need to add some limits on how many packets can be cached ?
    if (ctx->coap_dtls_ctx->cachedqueue) {
        coap_queue_t *p = ctx->coap_dtls_ctx->cachedqueue;
        while(p->next != NULL) {
            p = p->next;
        }
        p->next = node;
    } else {
        ctx->coap_dtls_ctx->cachedqueue = node;
    }

    return 0;
}

/**
 * Once a DTLS session is established and cached pdu is send, this pdu needs to
 * be saved in 'sendqueue' if this is a CON pdu for re-transmission purposes.
 *
 */
static void save_cached_con_pdu(coap_context_t *ctx,
        coap_queue_t *node)
{
    coap_tick_t now;

    coap_ticks(&now);
    if (ctx->sendqueue == NULL)
    {
        node->t = node->timeout;
        ctx->sendqueue_basetime = now;
    }
    else
    {
        /* make node->t relative to context->sendqueue_basetime */
        node->t = (now - ctx->sendqueue_basetime) + node->timeout;
    }

    node->delayedResNeeded = 0;
    node->next = NULL;
    coap_insert_node(&ctx->sendqueue, node);
}

/**
 * Once a DTLS session is established, this method is invoked to retrieve any
 * pdu's available in cachedqueue to be sent to the peer.
 *
 */
static coap_queue_t* get_cached_pdu( coap_context_t *ctx,
        const coap_address_t *dst)
{
    coap_queue_t *node, *prev;

    node = ctx->coap_dtls_ctx->cachedqueue;
    prev = NULL;
    while(node) {
        if (coap_address_equals(dst, &node->remote)) {
            //disconnect the node from cachedqueue
            if (node == ctx->coap_dtls_ctx->cachedqueue)
                ctx->coap_dtls_ctx->cachedqueue = node->next;
            else if (node->next == NULL)
                prev->next = NULL;
            else
                prev->next = node->next;

            node->next = NULL;
            return node;
        }
        prev = node;
        node = node->next;
    }
    return NULL;
}

/**
 * Once a DTLS session is established, this method takes care of sending
 * pdu's available in cachedqueue to the peer.
 *
 */
static void coap_send_cached_pdu( coap_context_t *ctx,
        const coap_address_t *dst )
{
    coap_queue_t *node;

    if (!ctx)
        return ;

    for (;(node=get_cached_pdu(ctx, dst));) {
        OC_LOG(DEBUG, MOD_NAME, PCF("Sending cached PDU"));
        OC_LOG_BUFFER(DEBUG, MOD_NAME, (uint8_t*)node->pdu->hdr, node->pdu->length);
        // Send this PDU to DTLS library for encryption
        dtls_ret ret = coap_dtls_encrypt_internal(ctx, (OCDevAddr*)dst,
                (uint8_t*)node->pdu->hdr, node->pdu->length);
        if (ret == DTLS_OK) {
            OC_LOG(DEBUG, MOD_NAME, PCF("coap_send_cached_pdu: successully send cached pdu"));
        } else {
            OC_LOG(DEBUG, MOD_NAME, PCF("coap_send_cached_pdu: sending cached pdu failed."));
            //TODO Notify application that packet send failed.
        }

        /* Add cache node in sendqueue if it is CON pdu,
         * as it may be needed for retransmission
         * else, delete it
         */
        if (node->pdu->hdr->type == COAP_MESSAGE_CON) {
            save_cached_con_pdu(ctx, node);
        } else {
            coap_delete_node(node);
        }
    }
}


/**
 * This is the tinyDTLS 'read' callback.
 * It is invoked by tinyDTLS to provide the decrypted pdu.
 *
 */
static int read_decrypted_payload(dtls_context_t *dtls_ctx,
            session_t *session,
            uint8_t *buf,
            size_t len )
{
    if (!dtls_ctx)
        return -1;

    coap_dtls_context_t* coap_dtls_ctx =
        ((coap_context_t*)dtls_get_app_data(dtls_ctx))->coap_dtls_ctx;

    if (coap_dtls_ctx && coap_dtls_ctx->pt_info) {
        coap_dtls_ctx->pt_info->pt = buf;
        coap_dtls_ctx->pt_info->ptlen = len;
        return len;
    }

    return -1;
}

/**
 * This is the tinyDTLS 'write' callback.
 * It is invoked by tinyDTLS to send encrypted data or handshake message to peer.
 *
 */
static int send_secure_data(dtls_context_t *dtls_ctx,
        session_t *session,
        uint8_t* buf,
        size_t buflen)
{
    if (!dtls_ctx)
        return -1;

    return OCSendTo( ((coap_context_t*)dtls_get_app_data(dtls_ctx))->sockfd_dtls,
            buf, buflen, 0, (OCDevAddr*)session);
}


/**
 * This is the tinyDTLS 'event' callback.
 * It is invoked by tinyDTLS to notify any DTLS events or alerts.
 *
 */
static int handle_secure_event(dtls_context_t *dtls_ctx,
        session_t *session,
        dtls_alert_level_t level,
        unsigned short code)
{
    if (!dtls_ctx)
        return -1;

    OC_LOG_V(DEBUG, MOD_NAME, "level %d, code %u", level, code);

    //Notify stack of any errors/connection state changes to upper layer
    //application
    if (!level && (code == DTLS_EVENT_CONNECTED))
    {
        coap_send_cached_pdu( (coap_context_t*)dtls_get_app_data(dtls_ctx),
            (coap_address_t*)session);
    }
    return 0;
}

/**
 * This is the tinyDTLS 'get_psk_info' callback.
 * It is invoked by tinyDTLS to retrieve identity/credentials.
 * This is currently a test version using stationary keys.
 *
 */
static int get_psk_credentials(dtls_context_t *ctx,
              const session_t *session,
              dtls_credentials_type_t type,
              const unsigned char *desc, size_t desc_len,
              unsigned char *result, size_t result_len)
{
    int ret = -1;
    OCDtlsPskCredsBlob *creds_blob = NULL;

    //Retrieve the credentials blob from security module
    OCGetDtlsPskCredentials(&creds_blob);

    if (!creds_blob)
        return ret;

    if ((type == DTLS_PSK_HINT) || (type == DTLS_PSK_IDENTITY)) {
        if (DTLS_PSK_ID_LEN <= result_len){
            memcpy(result, creds_blob->identity, DTLS_PSK_ID_LEN);
            ret = DTLS_PSK_ID_LEN;
        }
    }

    if ((type == DTLS_PSK_KEY) && (desc) && (desc_len == DTLS_PSK_PSK_LEN)) {
        //Check if we have the credentials for the device with which we
        //are trying to perform a handshake
        for (int i =0; i < creds_blob->num; i++) {
            if (memcmp(desc, creds_blob->creds[i].id, DTLS_PSK_ID_LEN) == 0)
            {
                memcpy(result, creds_blob->creds[i].psk, DTLS_PSK_PSK_LEN);
                ret = DTLS_PSK_PSK_LEN;
            }
        }
    }

    return ret;
}


/**
 * Open secure port and initialize tinyDTLS library.
 *
 * @param ctx - handle to global coap_context_t.
 *
 * @param ipAddr - ip address.
 *
 * @return A value less than zero on error, greater or
 *           equal otherwise.
 */
int coap_dtls_init(coap_context_t *ctx, uint8_t ipAddr[]) {

    int ret = -1;
    coap_dtls_context_t *coap_dtls_ctx = NULL;
    OCDevAddr dev_addr;

    if (!ctx)
        goto exit;

    coap_dtls_ctx =
        (coap_dtls_context_t*)coap_malloc(sizeof(coap_dtls_context_t));

    if (!coap_dtls_ctx)
        goto exit;
    memset(coap_dtls_ctx, 0, sizeof(coap_dtls_ctx));
    ctx->sockfd_dtls = -1;

    OCBuildIPv4Address(ipAddr[0], ipAddr[1], ipAddr[2], ipAddr[3],
                                        COAP_DTLS_DEFAULT_PORT, &dev_addr);
    if (OCInitUDP((OCDevAddr *)&dev_addr, (int32_t *)&(ctx->sockfd_dtls), 0) != ERR_SUCCESS) {
        OCBuildIPv4Address(ipAddr[0], ipAddr[1], ipAddr[2], ipAddr[3],
                                        COAP_DTLS_RANDOM_PORT, &dev_addr);
        if (OCInitUDP((OCDevAddr *)&dev_addr, (int32_t *)&(ctx->sockfd_dtls), 0) != ERR_SUCCESS) {
            goto exit;
        }
    }

    // Initialize clock, crypto and other global vars in tinyDTLS library
    dtls_init();

    coap_dtls_ctx->dtls_ctx = dtls_new_context(ctx);
    if (!coap_dtls_ctx->dtls_ctx)
        goto exit;

    coap_dtls_ctx->callbacks.write = send_secure_data;
    coap_dtls_ctx->callbacks.read  = read_decrypted_payload;
    coap_dtls_ctx->callbacks.event = handle_secure_event;
    coap_dtls_ctx->callbacks.get_psk_info = get_psk_credentials;

    dtls_set_handler(coap_dtls_ctx->dtls_ctx, &(coap_dtls_ctx->callbacks));
    ctx->coap_dtls_ctx = coap_dtls_ctx;
    ret = 0;

exit:
    if (ret == -1) {
        coap_dtls_deinit(ctx);
    }
    return ret;
}


/**
 * Closes secure port and de-inits tinyDTLS library.
 *
 * @param ctx - handle to global coap_context_t.
 *
 */
void coap_dtls_deinit(coap_context_t *ctx) {

    if (!ctx || !ctx->coap_dtls_ctx)
        return;

    coap_dtls_context_t *coap_dtls_ctx = ctx->coap_dtls_ctx;

    coap_delete_all(coap_dtls_ctx->cachedqueue);

    dtls_free_context(coap_dtls_ctx->dtls_ctx);
    coap_dtls_ctx->dtls_ctx = NULL;

    if (ctx->sockfd_dtls != -1)
        OCClose(ctx->sockfd_dtls);

    coap_free(coap_dtls_ctx);
    ctx->coap_dtls_ctx = NULL;
}


/**
 * Performs DTLS encryption of the CoAP PDU. If a
 * DTLS session does not exist yet with the @dst,
 * a DTLS handshake will be started. In case where
 * a new DTLS handshake is started, pdu info is
 * cached to be send when session setup is finished.
 *
 * @param ctx    - handle to global coap_context_t.
 * @param dst    - address of the receiver of the pdu.
 * @param pdu    - pointer to CoAP pdu.
 * @param node   - address of the node holding pdu.
 * @param tid    - tid of the pdu.
 * @param cached - output variable to indicate if pdu
 *                  is cached and inform the caller to
 *                  NOT free the memory holding pdu.
 *
 * @return A value less than zero on error, greater or
 *           equal otherwise.
 */
int coap_dtls_encrypt(coap_context_t *ctx,
            OCDevAddr *dst,
            coap_pdu_t *pdu,
            coap_queue_t **node,
            coap_tid_t tid,
            uint8_t *cache_flag) {
    OC_LOG(DEBUG, MOD_NAME, PCF("coap_dtls_encrypt"));

    if (!dst || !pdu)
        return -1;

    dtls_ret ret = coap_dtls_encrypt_internal( ctx, dst,
            (uint8_t*)pdu->hdr, pdu->length);

    if (ret == DTLS_SESSION_INITIATED) {
        OC_LOG(DEBUG, MOD_NAME, PCF("Initiated new DTLS session"));
        if (cache_flag && coap_cache_pdu(ctx, *node, dst, pdu, tid) == 0) {
            /* Delete the node from sendqueue list as it has been
             * added in cachedqueue list. It will be added
             * again in sendqueue list when DTLS session is established
             */
            if (*node) {
                coap_queue_t* removed_node = NULL;
                coap_remove_from_queue(&(ctx->sendqueue),
                   (*node)->id, &removed_node);
                if (removed_node == *node) {
                    coap_free(*node);
                    *node = NULL;
                    OC_LOG(DEBUG, MOD_NAME, PCF("coap_dtls_encrypt -- Removed correct node"));
                }
            }
            *cache_flag = 1;
        }
        return pdu->length;
    }

    if (ret == DTLS_OK) {
        OC_LOG(DEBUG, MOD_NAME, PCF("Encrypted App PDU and send to peer"));
        return pdu->length;
    }
    return -1;
}


/**
 * Performs DTLS decryption of the CoAP PDU received on
 * secure port. This method performs in-place decryption
 * of the cipher-text buffer. If a DTLS handshake message
 * is received or decryption failure happens, this method
 * returns -1. If a valid CoAP pdu is received, it returns the
 * length of the decrypted pdu.
 *
 * @param ctx    - handle to global coap_context_t.
 * @param src    - address of the sender of the pdu.
 * @param ct     - pointer to the cipher text buffer.
 * @param ctlen  - length of the ciphertext buffer.
 * @param pt     - output variable to store the starting address
 *                  of decrypted plaintext.
 * @param ptlen  - output variable to store the length of
 *                  decrypted plaintext.
 *
 * @return A value less than zero on error, greater or
 *           equal otherwise.
 */
int coap_dtls_decrypt(coap_context_t *ctx,
            OCDevAddr* src,
            uint8_t* ct,
            int ctlen,
            uint8_t** pt,
            int* ptlen) {
    OC_LOG(DEBUG, MOD_NAME, PCF("coap_dtls_decrypt"));

    if (!src || !ct || !pt || !ptlen)
        return -1;

    dtls_ret ret = coap_dtls_decrypt_internal(ctx, src, ct, ctlen,
            pt, ptlen);

    if (ret == DTLS_OK)
        return *ptlen;

    if (ret == DTLS_HS_MSG)
        OC_LOG(DEBUG, MOD_NAME, PCF("coap_dtls_decrypt : Handshake msg recvd "));
    if (ret == DTLS_FAIL)
        OC_LOG(DEBUG, MOD_NAME, PCF("coap_dtls_decrypt : Decryption failure "));

    return -1;
}
