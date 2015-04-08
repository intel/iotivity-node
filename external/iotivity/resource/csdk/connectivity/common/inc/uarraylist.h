/******************************************************************
 *
 * Copyright 2014 Samsung Electronics All Rights Reserved.
 *
 *
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ******************************************************************/

#ifndef __U_ARRAYLIST_H_
#define __U_ARRAYLIST_H_

#include <stdint.h>
#include "cacommon.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Use this default size when initialized
 */
#define U_ARRAYLIST_DEFAULT_SIZE 1

/**
 * @struct u_arraylist_t
 * @brief array list structure
 */
typedef struct u_arraylist_t
{
    void **data;
    uint32_t length;
    uint32_t size;
} u_arraylist_t;

/**
 * @brief API to creates array list and initializes the elements.
 * @return  u_arraylist_t if Success, NULL otherwise
 */
u_arraylist_t* u_arraylist_create();

/**
 * @brief Resets and deletes the array list
 * application should free the memory of data in array list
 * @param u_arraylist_t- u_arraylist pointer
 * @return CAResult_t
 * CA_STATUS_OK if Success, CA_STATUS_FAILED otherwise
 */
CAResult_t u_arraylist_free(u_arraylist_t *list);

/**
 * @brief Returns the data of the index from the array list
 * @param u_arraylist
 *     [IN] pointer of array list
 * @param index
 *     [IN] index of array list
 * @return void pointer of the data
 */
void* u_arraylist_get(const u_arraylist_t *list, uint32_t index);

/**
 * @brief Add data in the array list
 * @param u_arraylist
 *     [IN] pointer of array list
 * @param data
 *     [IN] pointer of data
 * @return CAResult_t
 * CA_STATUS_OK if Success, CA_STATUS_FAILED otherwise
 */
CAResult_t u_arraylist_add(u_arraylist_t *list, void *data);

/**
 * @brief Remove the data of the index from the array list
 * @param u_arraylist
 *     [IN] pointer of array list
 * @param index
 *     [IN] index of array list
 * @return void pointer of the data
 */
void* u_arraylist_remove(u_arraylist_t *list, uint32_t index);

/**
 * @brief Returns the length of the array list
 * @param u_arraylist
 *     [IN] pointer of array list
 * @return length of the array list
 */
uint32_t u_arraylist_length(const u_arraylist_t *list);

/**
 * @brief Returns whether the data exists or not
 * @param u_arraylist
 *     [IN] pointer of array list
 * @param data
 *     [IN] pointer of data
 * @return 1 if exists, 0 otherwise
 */
uint8_t u_arraylist_contains(const u_arraylist_t *list, void *data);

#ifdef __cplusplus
}
#endif

#endif /* _U_ARRAYLIST_H_ */
