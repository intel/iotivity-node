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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "cainterface.h"
#include "camessagehandler.h"
#include "caremotehandler.h"
#include "cainterfacecontroller.h"
#include "uqueue.h"
#include "logger.h"
#include "config.h" /* for coap protocol */
#include "coap.h"
#include "uthreadpool.h" /* for thread pool */
#include "umutex.h"
#include "oic_malloc.h"

#define TAG PCF("CA")

#define MEMORY_ALLOCK_CHECK(arg) { if (arg == NULL) {OIC_LOG_V(DEBUG, TAG, "memory error"); goto memory_error_exit;} }
#define MAX_ACTION_NUM   300

#define MAX_THREAD_POOL_SIZE    10

#ifndef TRUE
#define TRUE    1
#endif

#ifndef FALSE
#define FALSE   0
#endif

typedef struct
{
    int32_t actionId;
    CARemoteEndpoint_t* remoteEndpoint;
    CARequestInfo_t* requestInfo;
    CAResponseInfo_t* responseInfo;
} CAData_t;

typedef void (*CAThreadTask)(CAData_t* data);

typedef struct
{
    u_mutex threadMutex;
    u_cond threadCond;
    CAThreadTask threadTask;
    int32_t isStop;
    u_queue_t* dataQueue;
} CAThread_t;

// thread pool handle
static u_thread_pool_t gThreadPoolHandle = NULL;

// message handler main thread
static CAThread_t gSendThread;

// message handler callback
static int32_t gCurrentActionId = 0;
static CAMessageHandlerCallback gHandlerCallback = NULL;

// handler field
static CARequestCallback gRequestHandler = NULL;
static CAResponseCallback gResponseHandler = NULL;

static u_queue_t* gMessageQueue = NULL;
static u_mutex gMessageQueueMutex = NULL;

static void CAAddReceiveData(CAData_t* data)
{
    OIC_LOG_V(DEBUG, TAG, "CAAddReceiveData");

    // create thread data
    u_queue_message_t* message = (u_queue_message_t*) OICMalloc(sizeof(u_queue_message_t));

    if (message == NULL)
    {
        OIC_LOG_V(DEBUG, TAG, "memory error!!");
        return;
    }
    memset(message, 0, sizeof(u_queue_message_t));

    message->msg = data;
    message->size = sizeof(CAData_t);

    // mutex lock
    u_mutex_lock(gMessageQueueMutex);

    // add thread data into list
    u_queue_add_element(gMessageQueue, message);

    // mutex unlock
    u_mutex_unlock(gMessageQueueMutex);
}

static void CAAddSendData(CAData_t* data)
{
    OIC_LOG_V(DEBUG, TAG, "CAAddSendData!!");

    // create thread data
    u_queue_message_t* message = (u_queue_message_t*) OICMalloc(sizeof(u_queue_message_t));

    if (message == NULL)
    {
        OIC_LOG_V(DEBUG, TAG, "memory error!!");
        return;
    }
    memset(message, 0, sizeof(u_queue_message_t));

    message->msg = data;
    message->size = sizeof(CAData_t);

    // mutex lock
    u_mutex_lock(gSendThread.threadMutex);

    // add thread data into list
    u_queue_add_element(gSendThread.dataQueue, message);

    // notity the thread
    u_cond_signal(gSendThread.threadCond);

    // mutex unlock
    u_mutex_unlock(gSendThread.threadMutex);
}

static void CAStopSendThread()
{
    OIC_LOG_V(DEBUG, TAG, "CAStopSendThread request!!");

    // mutex lock
    u_mutex_lock(gSendThread.threadMutex);

    // set stop flag
    gSendThread.isStop = TRUE;

    // notity the thread
    u_cond_signal(gSendThread.threadCond);

    // mutex unlock
    u_mutex_unlock(gSendThread.threadMutex);
}

static void CASendThreadProcess(CAData_t* data)
{
    if (data == NULL)
    {
        OIC_LOG(DEBUG, TAG, "thread data error!!");
        return;
    }

    if (NULL == data->remoteEndpoint)
    {
        OIC_LOG(DEBUG, TAG, "remoteEndpoint is null");
        return;
    }

    OIC_LOG_V(DEBUG, TAG, "thread action id : %d", data->actionId);

    CADetachErrorCode code = FAIL;
    int32_t res = 0;

    if (data->requestInfo != NULL)
    {
        OIC_LOG(DEBUG, TAG, "requestInfo is available");

        coap_pdu_t* pdu = NULL;
        pdu = CAGeneratePdu(data->remoteEndpoint->resourceUri, data->requestInfo->method,
                data->requestInfo->info);

        // interface controller function call.
        if (NULL != pdu)
        {
            OIC_LOG_V(DEBUG, TAG, "PDU Maker - payload : %s", pdu->data);

            OIC_LOG_V(DEBUG, TAG, "PDU Maker - code : %d", pdu->hdr->code);

            OIC_LOG_V(DEBUG, TAG, "PDU Maker - buffer data : %s", pdu->hdr);

            res = CASendUnicastData(data->remoteEndpoint, pdu->hdr, pdu->length);
        }
    }
    else if (data->responseInfo != NULL)
    {
        OIC_LOG_V(DEBUG, TAG, "responseInfo is available..");

        coap_pdu_t* pdu = NULL;

        pdu = CAGeneratePdu(data->remoteEndpoint->resourceUri, data->responseInfo->result,
                data->responseInfo->info);

        // interface controller function call.
        if (NULL != pdu)
        {
            OIC_LOG_V(DEBUG, TAG, "PDU Maker - payload : %s", pdu->data);

            OIC_LOG_V(DEBUG, TAG, "PDU Maker - code : %d", pdu->hdr->code);

            OIC_LOG_V(DEBUG, TAG, "PDU Maker - buffer data : %x", pdu->hdr);

            res = CASendUnicastData(data->remoteEndpoint, pdu->hdr, pdu->length);
        }
    }
    else
    {
        OIC_LOG(DEBUG, TAG, "both requestInfo & responseInfo is not available");

        coap_pdu_t* pdu = NULL;
        CAInfo_t info;
        memset(&info, 0, sizeof(CAInfo_t));
        pdu = CAGeneratePdu(data->remoteEndpoint->resourceUri, CA_GET, info);

        if (NULL != pdu)
        {
            OIC_LOG_V(DEBUG, TAG, "PDU Maker - payload : %s", pdu->data);

            OIC_LOG_V(DEBUG, TAG, "PDU Maker - type : %d", pdu->hdr->type);

            OIC_LOG_V(DEBUG, TAG, "PDU Maker - code : %d", pdu->hdr->code);

            OIC_LOG_V(DEBUG, TAG, "PDU Maker - id : %d", pdu->hdr->id);

            OIC_LOG_V(DEBUG, TAG, "PDU Maker - buffer data : %x", pdu->hdr);

            res = CASendMulticastData(pdu->hdr, pdu->length);
        }

    }

    if (res)
    {
        code = SUCCESS;
    }

    if (gHandlerCallback != NULL)
    {
        gHandlerCallback(data->actionId, code);
    }
}

static void* CAThreadBaseRoutine(void* treadData)
{
    OIC_LOG_V(DEBUG, TAG, "message handler main thread start..");

    CAThread_t* thread = (CAThread_t*) treadData;

    if (thread == NULL)
    {
        OIC_LOG_V(DEBUG, TAG, "thread data passing error!!");

        return NULL;
    }

    while (!thread->isStop)
    {
        // mutex lock
        u_mutex_lock(thread->threadMutex);

        // if queue is empty, thread will wait
        if (u_queue_get_size(thread->dataQueue) <= 0)
        {
            OIC_LOG_V(DEBUG, TAG, "wait..");
            // wait
            u_cond_wait(thread->threadCond, thread->threadMutex);

            OIC_LOG_V(DEBUG, TAG, "wake up..");
        }

        // mutex unlock
        u_mutex_unlock(thread->threadMutex);

        // check stop flag
        if (thread->isStop)
            continue;

        // get data
        u_queue_message_t* message = u_queue_get_element(thread->dataQueue);

        CAData_t* data = (CAData_t*) message->msg;

        // process data
        thread->threadTask(data);
    }

    OIC_LOG_V(DEBUG, TAG, "message handler main thread end..");

    return NULL;
}

static int32_t CAIncreaseActionId()
{
    ++gCurrentActionId;

    gCurrentActionId = (gCurrentActionId > MAX_ACTION_NUM) ? 0 : gCurrentActionId;

    return gCurrentActionId;
}

static void CAReceivedPacketCallback(CARemoteEndpoint_t* endpoint, void* data, uint32_t dataLen)
{
    OIC_LOG(DEBUG, TAG, "receivedPacketCallback in message handler!!");

    if (NULL == data)
    {
        OIC_LOG(DEBUG, TAG, "received data is null");
        return;
    }

    coap_pdu_t* pdu;
    uint32_t code = CA_NOT_FOUND;
    pdu = CAParsePDU(data, &code);

    if (code == CA_GET || code == CA_POST || code == CA_PUT || code == CA_DELETE)
    {
        CARequestInfo_t ReqInfo;
        memset(&ReqInfo, 0, sizeof(CARequestInfo_t));
        CAGetRequestInfoFromPdu(pdu, &ReqInfo);

        if (NULL != ReqInfo.info.options && NULL != endpoint)
        {
            OIC_LOG_V(DEBUG, TAG, "Request PDU - optionID: %d", ReqInfo.info.options->optionID);

            OIC_LOG_V(DEBUG, TAG, "Request PDU - optionlist: %s", ReqInfo.info.options->optionData);

            OIC_LOG_V(DEBUG, TAG, "Request PDU  - payload: %s", ReqInfo.info.payload);

            OIC_LOG_V(DEBUG, TAG, "Request PDU  - code: %d", ReqInfo.method);

            endpoint->resourceUri = (char*) OICMalloc(strlen(ReqInfo.info.options->optionData) + 1);
            memcpy(endpoint->resourceUri, ReqInfo.info.options->optionData,
                    strlen(ReqInfo.info.options->optionData));
            OIC_LOG_V(DEBUG, TAG, "added resource URI : %s", endpoint->resourceUri);
        }

        // store the data at queue.
        CAData_t* cadata = NULL;
        cadata = (CAData_t*) OICMalloc(sizeof(CAData_t));
        memset(cadata, 0, sizeof(CAData_t));
        cadata->actionId = 1;
        cadata->remoteEndpoint = endpoint;
        cadata->requestInfo = &ReqInfo;
        cadata->responseInfo = NULL;
        CAAddReceiveData(cadata);

    }
    else
    {
        CAResponseInfo_t ResInfo;
        memset(&ResInfo, 0, sizeof(CARequestInfo_t));
        CAGetResponseInfoFromPdu(pdu, &ResInfo);

        if (NULL != ResInfo.info.options && NULL != endpoint)
        {
            OIC_LOG_V(DEBUG, TAG, "Response PDU - optionID: %d", ResInfo.info.options->optionID);

            OIC_LOG_V(DEBUG, TAG, "Response PDU - optionlist: %s", ResInfo.info.options->optionData);

            OIC_LOG_V(DEBUG, TAG, "Response PDU - payload: %s", ResInfo.info.payload);

            OIC_LOG_V(DEBUG, TAG, "Response PDU - code: %d", ResInfo.result);

            endpoint->resourceUri = (char*) OICMalloc(strlen(ResInfo.info.options->optionData) + 1);
            memcpy(endpoint->resourceUri, ResInfo.info.options->optionData,
                    strlen(ResInfo.info.options->optionData));
            OIC_LOG_V(DEBUG, TAG, "added resource URI : %s", endpoint->resourceUri);
        }

        // store the data at queue.
        CAData_t* cadata = NULL;
        cadata = (CAData_t*) OICMalloc(sizeof(CAData_t));
        memset(cadata, 0, sizeof(CAData_t));
        cadata->actionId = 1;
        cadata->remoteEndpoint = endpoint;
        cadata->requestInfo = NULL;
        cadata->responseInfo = &ResInfo;
        CAAddReceiveData(cadata);
    }
}

void CAHandleRequestResponseCallbacks()
{
    OIC_LOG_V(DEBUG, TAG, "CAHandleRequestResponseCallbacks");

    // parse the data and call the callbacks.
    // #1 parse the data
    // #2 get endpoint

    u_mutex_lock(gMessageQueueMutex);

    u_queue_message_t* item = u_queue_get_element(gMessageQueue);

    u_mutex_unlock(gMessageQueueMutex);

    if (item == NULL)
        return;

    // get values
    void* msg = item->msg;

    if (msg == NULL)
        return;

    // get endpoint
    CAData_t* td = (CAData_t*) msg;

    CARemoteEndpoint_t* rep = td->remoteEndpoint;

    if (rep == NULL)
        return;

    if (td->requestInfo != NULL)
    {
        if (gRequestHandler)
        {
            gRequestHandler(rep, NULL);
        }
    }

    if (td->responseInfo != NULL)
    {
        if (gResponseHandler)
        {
            gResponseHandler(rep, NULL);
        }
    }

    u_queue_remove_element(gMessageQueue);
}

int32_t CADetachRequestMessage(const CARemoteEndpoint_t* object, const CARequestInfo_t* request)
{
    OIC_LOG_V(DEBUG, TAG, "CADetachRequestMessage");

    if (object == NULL || request == NULL)
    {
        return -1;
    }

    int32_t id = 0;

    // create action id
    id = CAIncreaseActionId();

    CAData_t* data = (CAData_t*) OICMalloc(sizeof(CAData_t));
    MEMORY_ALLOCK_CHECK(data);

    // initialize
    memset(data, 0, sizeof(CAData_t));

    // clone remote endpoint
    CARemoteEndpoint_t* remoteEndpoint = CACloneRemoteEndpoint(object);
    MEMORY_ALLOCK_CHECK(remoteEndpoint);

    // clone request info
    CARequestInfo_t* requestInfo = CACloneRequestInfo(request);
    MEMORY_ALLOCK_CHECK(requestInfo);

    // save data
    data->actionId = id;
    data->remoteEndpoint = remoteEndpoint;
    data->requestInfo = requestInfo;
    data->responseInfo = NULL;

    // add thread
    CAAddSendData(data);

    return id;

    // memory error label.
    memory_error_exit:

    CADestroyRemoteEndpointInternal(remoteEndpoint);

    CADestroyRequestInfoInternal(requestInfo);

    if (data != NULL)
    {
        OICFree(data);
    }

    return -1;
}

int32_t CADetachResponseMessage(const CARemoteEndpoint_t* object, const CAResponseInfo_t* response)
{
    OIC_LOG_V(DEBUG, TAG, "CADetachResponseMessage");

    if (object == NULL || response == NULL)
    {
        return -1;
    }

    int32_t id = 0;

    // create action id
    id = CAIncreaseActionId();

    CAData_t* data = (CAData_t*) OICMalloc(sizeof(CAData_t));
    MEMORY_ALLOCK_CHECK(data);

    // initialize
    memset(data, 0, sizeof(CAData_t));

    // clone remote endpoint
    CARemoteEndpoint_t* remoteEndpoint = CACloneRemoteEndpoint(object);
    MEMORY_ALLOCK_CHECK(remoteEndpoint);

    // clone response info
    CAResponseInfo_t* responseInfo = CACloneResponseInfo(response);
    MEMORY_ALLOCK_CHECK(responseInfo);

    // save data
    data->actionId = id;
    data->remoteEndpoint = remoteEndpoint;
    data->requestInfo = NULL;
    data->responseInfo = responseInfo;

    // add thread
    CAAddSendData(data);

    return id;

    // memory error label.
    memory_error_exit:

    CADestroyRemoteEndpointInternal(remoteEndpoint);

    CADestroyResponseInfoInternal(responseInfo);

    if (data != NULL)
    {
        OICFree(data);
    }

    return -1;
}

int32_t CADetachMessageResourceUri(const CAURI_t resourceUri)
{
    if (resourceUri == NULL)
    {
        return -1;
    }

    int32_t id = 0;

    // create action id
    id = CAIncreaseActionId();

    CAData_t* data = (CAData_t*) OICMalloc(sizeof(CAData_t));
    MEMORY_ALLOCK_CHECK(data);

    // initialize
    memset(data, 0, sizeof(CAData_t));

    CAAddress_t addr;
    memset(&addr, 0, sizeof(CAAddress_t));
    CARemoteEndpoint_t* remoteEndpoint = CACreateRemoteEndpointInternal(resourceUri, addr,
            CA_ETHERNET | CA_WIFI | CA_EDR | CA_LE);

    // save data
    data->actionId = id;
    data->remoteEndpoint = remoteEndpoint;
    data->requestInfo = NULL;
    data->responseInfo = NULL;

    // add thread
    CAAddSendData(data);

    return id;

    // memory error label.
    memory_error_exit:

    CADestroyRemoteEndpointInternal(remoteEndpoint);

    if (data != NULL)
    {
        OICFree(data);
    }

    return -1;
}

void CASetMessageHandlerCallback(CAMessageHandlerCallback callback)
{
    OIC_LOG_V(DEBUG, TAG, "set message handler callback.");

    gHandlerCallback = callback;
}

void CASetRequestResponseCallbacks(CARequestCallback ReqHandler, CAResponseCallback RespHandler)
{
    OIC_LOG_V(DEBUG, TAG, "set request, response handler callback.");

    gRequestHandler = ReqHandler;
    gResponseHandler = RespHandler;
}

CAResult_t CAInitializeMessageHandler()
{
    CASetPacketReceivedCallback(CAReceivedPacketCallback);

    // create thread pool
    CAResult_t res;
    res = u_thread_pool_init(MAX_THREAD_POOL_SIZE, &gThreadPoolHandle);

    if (res != CA_STATUS_OK)
    {
        OIC_LOG_V(DEBUG, TAG, "thread pool initialize error.");
        return res;
    }

    // send thread initialize
    memset(&gSendThread, 0, sizeof(CAThread_t));

    // mutex init
    u_mutex_init();

    // set send thread data
    gSendThread.dataQueue = u_queue_create();
    gSendThread.threadMutex = u_mutex_new();
    gSendThread.threadCond = u_cond_new();
    gSendThread.isStop = FALSE;
    gSendThread.threadTask = CASendThreadProcess;

    // start send thread
    res = u_thread_pool_add_task(gThreadPoolHandle, CAThreadBaseRoutine, &gSendThread);

    if (res != CA_STATUS_OK)
    {
        OIC_LOG_V(DEBUG, TAG, "thread pool add task error.");
        return res;
    }

    // set receive queue
    gMessageQueue = u_queue_create();
    gMessageQueueMutex = u_mutex_new();

    // initialize interface adapters by controller
    CAInitializeAdapters();

    return CA_STATUS_OK;
}

void CATerminateMessageHandler()
{
    // terminate interface adapters by controller
    CATerminateAdapters();

    // stop thread
    CAStopSendThread();

    // delete thread data
    u_mutex_free(gSendThread.threadMutex);
    u_cond_free(gSendThread.threadCond);
    u_queue_delete(gSendThread.dataQueue);

    // destroy thread pool
    u_thread_pool_free(gThreadPoolHandle);

    OIC_LOG_V(DEBUG, TAG, "message handler terminate completed!");

    u_queue_delete(gMessageQueue);
    u_mutex_free(gMessageQueueMutex);
}

