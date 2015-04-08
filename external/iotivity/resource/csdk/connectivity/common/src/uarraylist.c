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

#include <stdlib.h>
#include <string.h>
#include "uarraylist.h"
#include "logger.h"
#include "oic_malloc.h"

#define TAG PCF("UARRAYLIST")

u_arraylist_t *u_arraylist_create()
{
    u_arraylist_t *list = NULL;

    if (!(list = (u_arraylist_t*) OICMalloc(sizeof(u_arraylist_t))))
    {
        return NULL;
    }

    list->size = U_ARRAYLIST_DEFAULT_SIZE;
    list->length = 0;

    if (!(list->data = (void*) OICMalloc(list->size * sizeof(void*))))
    {
        OICFree(list);
        return NULL;
    }

    return list;
}

CAResult_t u_arraylist_free(u_arraylist_t *list)
{
    OICFree(list->data);
    OICFree(list);

    list = NULL;

    return CA_STATUS_OK;
}

void *u_arraylist_get(const u_arraylist_t *list, uint32_t index)
{
    if (index >= list->length)
    {
        return NULL;
    }

    if (list->data)
    {
        return list->data[index];
    }

    return NULL;
}

CAResult_t u_arraylist_add(u_arraylist_t *list, void *data)
{
    uint32_t new_size = 0;

    if (list->size <= list->length)
    {

        new_size = list->size + 1;
        if (!(list->data = (void **) realloc(list->data, new_size * sizeof(void *))))
        {
            return -1;
        }

        (void) memset(list->data + list->size, 0, (new_size - list->size) * sizeof(void *));
        list->size = new_size;
    }

    list->data[list->length] = data;
    list->length++;

    return 0;
}

void *u_arraylist_remove(u_arraylist_t *list, uint32_t index)
{
    void *removed = NULL;

    if (index >= list->length)
    {
        return NULL;
    }

    removed = list->data[index];

    if (index < list->length - 1)
    {
        memmove(&list->data[index], &list->data[index + 1],
                (list->length - index - 1) * sizeof(void *));
    }

    list->size--;
    list->length--;

    if (!(list->data = (void **) realloc(list->data, list->size * sizeof(void *))))
    {
        return NULL;
    }

    return removed;
}

uint32_t u_arraylist_length(const u_arraylist_t *list)
{
    return list->length;
}

uint8_t u_arraylist_contains(const u_arraylist_t *list, void *data)
{
    uint32_t i = 0;

    for (i = 0; i < u_arraylist_length(list); i++)
    {
        if (data == u_arraylist_get(list, i))
        {
            return 1;
        }
        else
        {
            continue;
        }
    }

    return 0;
}

