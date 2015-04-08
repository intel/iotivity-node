/******************************************************************
 *
 * Copyright 2014 Samsung Electronics All Rights Reserved.
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
/**
 * @file    uthreadpool.h
 * @brief   This file provides APIs related to thread pool
 */

#ifndef __UTHREAD_POOL_H_
#define __UTHREAD_POOL_H_

#include <stdio.h>
#include <malloc.h>
#include <glib.h>
#include <glib/gthreadpool.h>

#include "cacommon.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/**
 * @var u_thread_func
 * @brief Callback type can be registered to thread pool.
 */
typedef void (*u_thread_func)(void *);

/**
 * @struct u_thread_msg_t
 * @brief Structure to maintain the data which needs to send to task function.
 */
typedef struct
{
    void *data;
    u_thread_func func;
} u_thread_msg_t;

/**
 * @var u_thread_pool_t
 * @brief Thread pool type.
 */
typedef void *u_thread_pool_t;

/**
 * This function creates a newly allocated thread pool.
 *
 * @param num_of_threads The number of worker thread used in this pool.
 * @param thread_pool_handle Handle to newly create thread pool.
 * @return Error code, CA_STATUS_OK if success, else error number.
 */
CAResult_t u_thread_pool_init(uint32_t num_of_threads, u_thread_pool_t *thread_pool_handle);

/**
 * This function adds a routine to be executed by the thread pool at some future time.
 *
 * @param pool The thread pool structure.
 * @param routine The routine to be executed.
 * @param data The data to be passed to the routine.
 *
 * @return CA_STATUS_OK on success.
 * @return Error on failure.
 */
CAResult_t u_thread_pool_add_task(u_thread_pool_t thread_pool, void (*routine)(void *), void *data);

/**
 * This function stops all the worker threads (stop & exit). And frees all the allocated memory.
 * Function will return only after joining all threads executing the currently scheduled tasks.
 *
 * @param pool The thread pool structure.
 */
void u_thread_pool_free(u_thread_pool_t thread_pool);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __UTHREAD_POOL_H_ */
