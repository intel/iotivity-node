//******************************************************************
//
// Copyright 2014 Intel Corporation All Rights Reserved.
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

#ifndef OC_SECURITY_CONFIG_H
#define OC_SECURITY_CONFIG_H

#include <stdint.h>

#define DTLS_PSK_ID_LEN 16
#define DTLS_PSK_PSK_LEN 16

#define DtlsPskCredsBlobVer_1 1 /**< Credentials stored in plaintext */
#define DtlsPskCredsBlobVer_CurrentVersion DtlsPskCredsBlobVer_1

/**
 * Credentials for a device. Includes identity and the associated PSK.
 */
typedef struct
{
   unsigned char id[DTLS_PSK_ID_LEN];
   unsigned char psk[DTLS_PSK_PSK_LEN];
} OCDtlsPskCreds;


/**
 * Binary blob containing device identity and the credentials for all devices
 * trusted by this device.
 */
typedef struct
{
   uint16_t blobVer;                        /**< version of the blob */
   uint16_t reserved;                       /**< reserved for future use */
   unsigned char identity[DTLS_PSK_ID_LEN]; /**< identity of self */
   uint32_t num;                            /**< number of credentials in this blob */
   OCDtlsPskCreds creds[1];                 /**< list of credentials. Size of this
                                                 array is determined by 'num' variable. */
} OCDtlsPskCredsBlob;


#endif //OC_SECURITY_CONFIG_H


