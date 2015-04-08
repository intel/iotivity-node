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

#include <ocstack.h>
#include <ocsecurity.h>
#include <logger.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

#define TAG "sample-common"

OCStackResult SetCredentials(const char* filename) {

    FILE *fp = NULL;
    uint8_t *data = NULL;
    struct stat st;
    OCStackResult ret = OC_STACK_ERROR;

    fp = fopen(filename, "rb");
    if (fp)
    {
        if (stat(filename, &st) == 0)
        {
            data = (uint8_t*)malloc(st.st_size);
            if (data)
            {
                if (fread(data, 1, st.st_size, fp) == (size_t)st.st_size)
                {
                    // Provide credentials to OC Stack
                    ret = OCSetDtlsPskCredentials((OCDtlsPskCredsBlob *)data,
                            st.st_size);
                }
                else
                {
                    OC_LOG_V(FATAL, TAG, "Error in reading file %s", filename);
                }
            }
        }
        fclose(fp);
    }
    else
    {
        OC_LOG_V(FATAL, TAG, "Unable to open %s file", filename);
    }

    free(data);

    return ret;
}

const char *getResult(OCStackResult result) {
    switch (result) {
    case OC_STACK_OK:
        return "OC_STACK_OK";
    case OC_STACK_RESOURCE_CREATED:
        return "OC_STACK_RESOURCE_CREATED";
    case OC_STACK_RESOURCE_DELETED:
        return "OC_STACK_RESOURCE_DELETED";
    case OC_STACK_INVALID_URI:
        return "OC_STACK_INVALID_URI";
    case OC_STACK_INVALID_QUERY:
        return "OC_STACK_INVALID_QUERY";
    case OC_STACK_INVALID_IP:
        return "OC_STACK_INVALID_IP";
    case OC_STACK_INVALID_PORT:
        return "OC_STACK_INVALID_PORT";
    case OC_STACK_INVALID_CALLBACK:
        return "OC_STACK_INVALID_CALLBACK";
    case OC_STACK_INVALID_METHOD:
        return "OC_STACK_INVALID_METHOD";
    case OC_STACK_NO_MEMORY:
        return "OC_STACK_NO_MEMORY";
    case OC_STACK_COMM_ERROR:
        return "OC_STACK_COMM_ERROR";
    case OC_STACK_INVALID_PARAM:
        return "OC_STACK_INVALID_PARAM";
    case OC_STACK_NOTIMPL:
        return "OC_STACK_NOTIMPL";
    case OC_STACK_NO_RESOURCE:
        return "OC_STACK_NO_RESOURCE";
    case OC_STACK_RESOURCE_ERROR:
        return "OC_STACK_RESOURCE_ERROR";
    case OC_STACK_SLOW_RESOURCE:
        return "OC_STACK_SLOW_RESOURCE";
    case OC_STACK_NO_OBSERVERS:
        return "OC_STACK_NO_OBSERVERS";
    #ifdef WITH_PRESENCE
    case OC_STACK_VIRTUAL_DO_NOT_HANDLE:
        return "OC_STACK_VIRTUAL_DO_NOT_HANDLE";
    case OC_STACK_PRESENCE_STOPPED:
        return "OC_STACK_PRESENCE_STOPPED";
    #endif
    case OC_STACK_ERROR:
        return "OC_STACK_ERROR";
    default:
        return "UNKNOWN";
    }
}
