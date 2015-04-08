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
#ifndef __CA_REMOTE_HANDLER_H_
#define __CA_REMOTE_HANDLER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cacommon.h"

#ifdef __cplusplus
extern "C"
{
#endif

CARemoteEndpoint_t* CACloneRemoteEndpoint(const CARemoteEndpoint_t* rep);

CARemoteEndpoint_t* CACreateRemoteEndpointUriInternal(const CAURI_t uri);

CARemoteEndpoint_t* CACreateRemoteEndpointInternal(const CAURI_t resourceUri,
        const CAAddress_t addr, const CAConnectivityType_t type);

void CADestroyRemoteEndpointInternal(CARemoteEndpoint_t* rep);

CARequestInfo_t* CACloneRequestInfo(const CARequestInfo_t* rep);

void CADestroyRequestInfoInternal(CARequestInfo_t* rep);

CAResponseInfo_t* CACloneResponseInfo(const CAResponseInfo_t* rep);

void CADestroyResponseInfoInternal(CAResponseInfo_t* rep);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif //#ifndef __CA_REMOTE_HANDLER_H_
