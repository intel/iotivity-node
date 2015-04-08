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
 * @file    umutex.c
 * @brief   This file provides APIs related to mutex and semaphores
 */

#include "umutex.h"
#include <glib.h>

#include "logger.h"
#define TAG PCF("UMUTEX")

void u_mutex_init(void)
{
    /*Initialize the glib thread system if it is not. GMutex works only if the threadsystem is initialized*/
    if (!g_thread_supported())
    {
        g_thread_init(NULL);
    }
}

u_mutex u_mutex_new(void)
{
    if (!g_thread_supported())
    {
        return NULL;
    }

    GMutex *mutexLock = g_mutex_new();
    return (u_mutex) mutexLock;
}

void u_mutex_lock(u_mutex mutex)
{
    if (NULL == mutex)
    {
        OIC_LOG_V(ERROR, TAG ,"Invalid mutex !");
        return;
    }

    GMutex *mutexLock = (GMutex*) mutex;
    g_mutex_lock(mutexLock);
}

CABool_t u_mutex_trylock(u_mutex mutex)
{
    if (NULL == mutex)
    {
        OIC_LOG_V(ERROR, TAG,"Invalid mutex !");
        return CA_FALSE;
    }

    GMutex *mutexLock = (GMutex*) mutex;
    gboolean ret = g_mutex_trylock(mutexLock);
    if (TRUE == ret)
    {
        return CA_TRUE;
    }

    return CA_FALSE;
}

void u_mutex_unlock(u_mutex mutex)
{
    if (NULL == mutex)
    {
        OIC_LOG_V(ERROR, TAG,"Invalid mutex !");
        return;
    }

    GMutex *mutexLock = (GMutex*) mutex;
    g_mutex_unlock(mutexLock);
}

void u_mutex_free(u_mutex mutex)
{
    if (NULL == mutex)
    {
        OIC_LOG_V(ERROR, TAG,"Invalid mutex !");
        return;
    }

    GMutex *mutexLock = (GMutex*) mutex;
    g_mutex_free(mutexLock);
}

u_cond u_cond_new(void)
{
    if (!g_thread_supported())
    {
        return NULL;
    }

    GCond *condition = g_cond_new();
    return (u_cond) condition;
}

void u_cond_signal(u_cond cond)
{
    if (NULL == cond)
    {
        OIC_LOG_V(ERROR, TAG,"Invalid condition !");
        return;
    }

    GCond *condition = (GCond*) cond;
    g_cond_signal(condition);
}

void u_cond_broadcast(u_cond cond)
{
    if (NULL == cond)
    {
        OIC_LOG_V(ERROR, TAG,"Invalid condition !");
        return;
    }

    GCond *condition = (GCond*) cond;
    g_cond_broadcast(condition);
}

void u_cond_wait(u_cond cond, u_mutex mutex)
{
    if (NULL == mutex)
    {
        OIC_LOG_V(ERROR, TAG,"Invalid mutex !");
        return;
    }

    if (NULL == cond)
    {
        OIC_LOG_V(ERROR, TAG,"Invalid condition !");
        return;
    }

    GMutex *mutexLock = (GMutex*) mutex;
    GCond *condition = (GCond*) cond;
    g_cond_wait(condition, mutexLock);
}

void u_cond_free(u_cond cond)
{
    if (NULL == cond)
    {
        OIC_LOG_V(ERROR, TAG,"Invalid condition !");
        return;
    }

    GCond *condition = (GCond*) cond;
    g_cond_free(condition);
}

