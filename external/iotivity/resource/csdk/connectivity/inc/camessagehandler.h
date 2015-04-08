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
#ifndef __CA_MESSAGE_HANDLER_H_
#define __CA_MESSAGE_HANDLER_H_

#include <stdint.h>
#include "cacommon.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum
{
    SUCCESS = 0, NETWORK_ERROR, FAIL
} CADetachErrorCode;

typedef void (*CAMessageHandlerCallback)(int32_t id, CADetachErrorCode code);

int32_t CADetachRequestMessage(const CARemoteEndpoint_t* object, const CARequestInfo_t* request);

int32_t CADetachResponseMessage(const CARemoteEndpoint_t* object, const CAResponseInfo_t* response);

int32_t CADetachMessageResourceUri(const CAURI_t resourceUri);

void CASetMessageHandlerCallback(CAMessageHandlerCallback callback);

void CASetRequestResponseCallbacks(CARequestCallback ReqHandler, CAResponseCallback RespHandler);

CAResult_t CAInitializeMessageHandler();

void CATerminateMessageHandler();

void CAHandleRequestResponseCallbacks();

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif //#ifndef __CA_MESSAGE_HANDLER_H_
