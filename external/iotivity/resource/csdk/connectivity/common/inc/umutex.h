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
 * @file    umutex.h
 * @brief   This file provides APIs related to mutex and semaphores
 */

#ifndef __UMUTEX_H_
#define __UMUTEX_H_

#include "cacommon.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef void *u_mutex;
typedef void *u_cond;

/**
 * @fn  u_mutex_init
 * @brief  Initializes the thread system for using other mutex related APIs
 *
 */
void u_mutex_init(void);

/**
 * @fn  u_mutex_new
 * @brief  Creates new mutex
 *
 * @return  Reference to newly created mutex, otherwise NULL.
 *
 * @see u_mutex_Init
 */
u_mutex u_mutex_new(void);

/**
 * @fn  u_mutex_lock
 * @brief  Lock the mutex
 *
 * @param  mutex  The mutex to be locked
 *
 */
void u_mutex_lock(u_mutex mutex);

/**
 * @fn  u_mutex_trylock
 * @brief  Checks if the mutex can be locked
 *
 * @param  mutex  The mutex to be locked
 *
 * @return  CA_TRUE if the mutex is not locked currently, otherwise CA_FALSE.
 *
 */
CABool_t u_mutex_trylock(u_mutex mutex);

/**
 * @fn  u_mutex_unlock
 * @brief  Unlock the mutex
 *
 * @param  mutex  The mutex to be unlocked
 *
 */
void u_mutex_unlock(u_mutex mutex);

/**
 * @fn  u_mutex_free
 * @brief  Free the mutex
 *
 * @param  mutex  The mutex to be freed
 *
 */
void u_mutex_free(u_mutex mutex);

/**
 * @fn  u_cond_new
 * @brief  Creates new condition
 *
 * @return  Reference to newly created @u_cond, otherwise NULL.
 *
 * @see u_mutex_Init
 */
u_cond u_cond_new(void);

/**
 * @fn  u_cond_signal
 * @brief  One of threads is woken up if multiple threads are waiting for @cond
 *
 * @param  cond  The condtion to be signaled
 *
 */
void u_cond_signal(u_cond cond);

/**
 * @fn  u_cond_broadcast
 * @brief  All of threads are woken up if multiple threads are waiting for @cond
 *
 * @param  cond  The condtion to be signaled
 *
 */
void u_cond_broadcast(u_cond cond);

/**
 * @fn  u_cond_wait
 * @brief  Waits untill this thread woken up on @cond
 *
 * @param  cond  The condtion to be wait for to signal
 * @param  mutex  The mutex which is currently locked from calling thread
 *
 */
void u_cond_wait(u_cond cond, u_mutex mutex);

/**
 * @fn  u_cond_free
 * @brief  Free the condition
 *
 * @param  mutex  The condition to be freed
 *
 */
void u_cond_free(u_cond cond);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif //__UMUTEX_H_
