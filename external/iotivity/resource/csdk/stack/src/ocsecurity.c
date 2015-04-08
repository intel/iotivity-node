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

#include "ocstack.h"
#include "ocmalloc.h"
#include "ocsecurity.h"
#include "ocsecurityconfig.h"
#include <string.h>

static OCDtlsPskCredsBlob* pskCredsBlob;
static int pskCredsBlobLen;

// Internal API. Invoked by OC stack to cleanup memory
void DeinitOCSecurityInfo()
{
    if (pskCredsBlob)
    {
        // Initialize sensitive data to zeroes before freeing.
        memset(pskCredsBlob, 0, pskCredsBlobLen);

        OCFree(pskCredsBlob);
        pskCredsBlob = NULL;
    }
}

// Internal API. Invoked by OC stack to retrieve credentials from this module
void OCGetDtlsPskCredentials(OCDtlsPskCredsBlob **credInfo)
{
    *credInfo = pskCredsBlob;
}

/**
 * Provides the DTLS PSK credetials blob to OC stack.
 *
 * @param credInfo
 *     binary blob containing credentials
 * @param len
 *     length of binary blob
 *
 * @retval OC_STACK_OK for Success, otherwise some error value
 */
OCStackResult OCSetDtlsPskCredentials(const OCDtlsPskCredsBlob *credInfo,
                size_t len)
{
    if(credInfo && len > 0)
    {
        if (credInfo->blobVer != DtlsPskCredsBlobVer_CurrentVersion)
        {
            return OC_STACK_INVALID_PARAM;
        }

        // Remove existing blob
        DeinitOCSecurityInfo();
        // Allocate storage for new blob
        pskCredsBlob = (OCDtlsPskCredsBlob*)OCMalloc(len);
        if (pskCredsBlob)
        {
            memcpy(pskCredsBlob, credInfo, len);
            pskCredsBlobLen = len;
            return OC_STACK_OK;
        }

        return OC_STACK_NO_MEMORY;
    }

    return OC_STACK_INVALID_PARAM;
}


