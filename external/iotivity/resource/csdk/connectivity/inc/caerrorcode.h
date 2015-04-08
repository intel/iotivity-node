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

#ifndef __CA_ERROR_CODE_H_
#define __CA_ERROR_CODE_H_

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Declares Stack Results & Errors
 */
typedef enum
{
    OC_STACK_OK = 0,
    OC_STACK_INVALID_URI,
    OC_STACK_INVALID_QUERY,
    OC_STACK_INVALID_IP,
    OC_STACK_INVALID_PORT,
    OC_STACK_INVALID_CALLBACK,
    OC_STACK_INVALID_METHOD,
    OC_STACK_INVALID_PARAM,
    OC_STACK_INVALID_OBSERVE_PARAM,
    OC_STACK_NO_MEMORY,
    OC_STACK_COMM_ERROR,
    OC_STACK_NOTIMPL,
    OC_STACK_NO_RESOURCE, /* resource not found*/
    OC_STACK_RESOURCE_ERROR, /*ex: not supported method or interface*/
    OC_STACK_SLOW_RESOURCE,
    OC_STACK_NO_OBSERVERS, /* resource has no registered observers */
    OC_STACK_OBSERVER_NOT_FOUND,
    OC_STACK_OBSERVER_NOT_ADDED,
    OC_STACK_OBSERVER_NOT_REMOVED,
#ifdef WITH_PRESENCE
    OC_STACK_PRESENCE_NO_UPDATE,
    OC_STACK_PRESENCE_STOPPED,
    OC_STACK_PRESENCE_DO_NOT_HANDLE,
#endif
    OC_STACK_ERROR
} OCStackResult;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif //#ifndef __CA_ERROR_CODE_H_
