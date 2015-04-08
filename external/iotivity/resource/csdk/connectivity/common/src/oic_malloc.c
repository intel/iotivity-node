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
#include <stdlib.h>
#include "oic_malloc.h"

// Enable extra debug logging for malloc.  Comment out to disable
//#define ENABLE_MALLOC_DEBUG  (1)

#ifdef ENABLE_MALLOC_DEBUG
#include "logger.h"
#define TAG PCF("OICMalloc")
#endif

//-----------------------------------------------------------------------------
// Typedefs
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Private variables
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Internal API function
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Private internal function prototypes
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Public APIs
//-----------------------------------------------------------------------------

void *OICMalloc(size_t size)
{
#ifdef ENABLE_MALLOC_DEBUG
    void *ptr = 0;

    if (0 == size)
    {
        return NULL;
    }

    ptr = malloc(size);
    OIC_LOG_V(INFO, TAG, "malloc: ptr=%p, size=%u", ptr, size);
    return ptr;
#else
    if (0 == size)
    {
        return NULL;
    }
    return malloc(size);
#endif
}

void OICFree(void *ptr)
{
#ifdef ENABLE_MALLOC_DEBUG
    OIC_LOG_V(INFO, TAG, "free: ptr=%p", ptr);
#endif

    free(ptr);
}

