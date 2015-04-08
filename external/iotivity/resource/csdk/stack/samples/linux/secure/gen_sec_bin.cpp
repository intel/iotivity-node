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

#include "ocsecurityconfig.h"
#include "logger.h"
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

#define TAG "gen_sec_bin"

void printStruct(const char * device, OCDtlsPskCredsBlob* s)
{
    OC_LOG(INFO, TAG, device);
    OC_LOG_V(INFO, TAG, "Version - %d", s->blobVer);
    OC_LOG(INFO, TAG, "My Identity :");
    OC_LOG_BUFFER(INFO, TAG, s->identity, DTLS_PSK_ID_LEN);

    OC_LOG_V(INFO, TAG, "Number of trusted Peers  - %d", s->num);
    OC_LOG(INFO, TAG, "Peer Identity :");
    OC_LOG_BUFFER(INFO, TAG, s->creds[0].id, DTLS_PSK_ID_LEN);
    OC_LOG(INFO, TAG, "Peer Psk :");
    OC_LOG_BUFFER(INFO, TAG, s->creds[0].psk, DTLS_PSK_PSK_LEN);
}


int main()
{
    OCDtlsPskCredsBlob * s = NULL;
    OCDtlsPskCredsBlob * c = NULL;
    FILE* fps, *fpc;

    int i;

    srand(time(NULL));

    s = (OCDtlsPskCredsBlob*) malloc(sizeof(OCDtlsPskCredsBlob));
    c = (OCDtlsPskCredsBlob*) malloc(sizeof(OCDtlsPskCredsBlob));

    memset(s, 0, sizeof(OCDtlsPskCredsBlob));
    memset(c, 0, sizeof(OCDtlsPskCredsBlob));

    s->blobVer = DtlsPskCredsBlobVer_CurrentVersion;
    c->blobVer = DtlsPskCredsBlobVer_CurrentVersion;

    s->num = c->num = 1;

    for(i = 0; i < DTLS_PSK_ID_LEN; i++)
    {
        c->creds[0].id[i] = s->identity[i] = rand() % (2^8);

        s->creds[0].id[i] = c->identity[i] = rand() % (2^8);

        c->creds[0].psk[i] = s->creds[0].psk[i] = rand() % (2^8);
    }

    // Print Credentials
    printStruct("Server", s);
    printStruct("Client", c);

    // Write to files
    if ((fps = (FILE*) fopen("server_cred.bin", "wb")) != NULL)
    {
        fwrite(s, sizeof(OCDtlsPskCredsBlob), 1, fps);
        fclose(fps);
    }


    if ((fpc = (FILE*) fopen("client_cred.bin", "wb")) != NULL)
    {
        fwrite(c, sizeof(OCDtlsPskCredsBlob), 1, fpc);
        fclose(fpc);
    }

    memset(s, 0, sizeof(OCDtlsPskCredsBlob));
    memset(c, 0, sizeof(OCDtlsPskCredsBlob));
    // Read from files; print and verify manually
    if ((fps = (FILE*) fopen("server_cred.bin", "rb")) != NULL)
    {
        if (sizeof(OCDtlsPskCredsBlob) != fread(s, 1, sizeof(OCDtlsPskCredsBlob), fps))
        {
            OC_LOG(INFO, TAG, PCF("Reading from the file failed."));
        }
        fclose(fps);
    }


    if ((fpc = (FILE*) fopen("client_cred.bin", "rb")) != NULL)
    {
        if (sizeof(OCDtlsPskCredsBlob) != fread(c, 1, sizeof(OCDtlsPskCredsBlob), fpc))
        {
            OC_LOG(INFO, TAG, PCF("Reading from the file failed."));
        }
        fclose(fpc);
    }

    printf("\n\n");
    OC_LOG(INFO, TAG, PCF("Reading from file and printing again to verify manually"));
    printStruct("Server", s);
    printStruct("Client", c);

    free(s);
    free(c);
}

