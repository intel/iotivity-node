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

#ifndef OCMALLOC_H_
#define OCMALLOC_H_

// The purpose of this module is to allow custom dynamic memory allocation
// code to easily be added to the TB Stack by redefining the OCMalloc and
// OCFree functions.  Examples of when this might be needed are on TB
// platforms that do not support dynamic allocation or if a memory pool
// needs to be added.
//
// Note that these functions are intended to be used ONLY within the TB
// stack and NOT by the application code.  The application should be
// responsible for its own dynamic allocation.

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

//-----------------------------------------------------------------------------
// Defines
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Typedefs
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function prototypes
//-----------------------------------------------------------------------------

/**
 * Allocates a block of size bytes, returning a pointer to the beginning of
 * the allocated block.
 *
 * @param size - Size of the memory block in bytes, where size > 0
 *
 * @return
 *     on success, a pointer to the allocated memory block
 *     on failure, a null pointer is returned
 */
void *OCMalloc(size_t size);

/**
 * Allocates a block of memory for an array of num elements, each of them
 * size bytes long and initializes all its bits to zero.
 *
 * @param num - The number of elements
 * @param size - Size of the element type in bytes, where size > 0
 *
 * @return
 *     on success, a pointer to the allocated memory block
 *     on failure, a null pointer is returned
 */
void *OCCalloc(size_t num, size_t size);

/**
 * Deallocate a block of memory previously allocated by a call to OCMalloc
 *
 * @param ptr - Pointer to block of memory previously allocated by OCMalloc.
 *              If ptr is a null pointer, the function does nothing.
 */
void OCFree(void *ptr);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif /* OCMALLOC_H_ */
