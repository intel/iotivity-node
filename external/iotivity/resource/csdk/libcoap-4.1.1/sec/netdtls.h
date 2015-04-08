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

#ifndef _NET_DTLS_H
#define _NET_DTLS_H

#include "net.h"
#include "address.h"
#include "pdu.h"
#include "dtls.h"

#define COAP_DTLS_DEFAULT_PORT  5684
#define COAP_DTLS_RANDOM_PORT   0

/**
 * Data structure for holding the tinyDTLS interface
 * related info.
 */
typedef struct coap_dtls_context_t {
    coap_queue_t *cachedqueue;          /**< pdu's are cached until DTLS session is formed */
    struct dtls_context_t *dtls_ctx;    /**< pointer to tinyDTLS context */
    struct pt_info_t *pt_info;          /**< used by callback during
                                             decryption to hold address/length */
    dtls_handler_t callbacks;           /**< pointer to callbacks needed by tinyDTLS */
}coap_dtls_context_t;

/**
 * Data structure for holding the decrypted data address
 * and length provided by tinyDTLS callback interface.
 */
typedef struct pt_info_t {
    uint8_t *pt;
    uint16_t ptlen;
}pt_info_t;

/**
 * Declares DTLS errors and return values. Currently used internally only.
 */
typedef enum
{
    DTLS_OK = 0,
    DTLS_FAIL,
    DTLS_SESSION_INITIATED,
    DTLS_HS_MSG
} dtls_ret;

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
int coap_dtls_init(coap_context_t *ctx, uint8_t ipAddr[]);

/**
 * Closes secure port and de-inits tinyDTLS library.
 *
 * @param ctx - handle to global coap_context_t.
 *
 */
void coap_dtls_deinit(coap_context_t *ctx);

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
 * @param cache_flag - output variable to indicate if pdu
 *                  is cached and inform the caller to
 *                  NOT free the memory holding pdu.
 *
 * @return A value less than zero on error, greater or
 *           equal otherwise.
 */
int coap_dtls_encrypt(coap_context_t *ctx,
            OCDevAddr* dst,
            coap_pdu_t *pdu,
            coap_queue_t **node,
            coap_tid_t tid,
            uint8_t *cache_flag);

/**
 * Performs DTLS decryption of the CoAP PDU received on
 * secure port. This method performs in-place decryption
 * of the cipher-text buffer. If a DTLS handshake message
 * is received or decryption failure happens, this method
 * returns -1. If a valid application PDU is decrypted, it
 * returns the length of the decrypted pdu.
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
            int* ptlen);

#endif //_NET_DTLS_H

