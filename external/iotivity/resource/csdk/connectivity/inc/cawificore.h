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

/**
 * @file cawificore.h
 * @brief This file contains the APIs for Wi-Fi communications.
 */
#ifndef __CA_WIFICORE_H_
#define __CA_WIFICORE_H_

#include "cacommon.h"
#include "config.h"
#include "coap.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef void (*CAPacketReceiveCallback)(const char* address, const char* data);

void CAWiFiInitialize();

void CAWiFiTerminate();

int32_t CAWiFiSendUnicastMessage(const char* address, const char* data, int32_t length);

int32_t CAWiFiSendMulticastMessage(const char* m_address, const char* data);

int32_t CAWiFiStartUnicastServer(const char* address, int32_t port);

int32_t CAWiFiStartMulticastServer(const char* m_address, int32_t port);

int32_t CAWiFiStopUnicastServer(int32_t server_id);

int32_t CAWiFiStopMulticastServer(int32_t server_id);

void CAWiFiSetCallback(CAPacketReceiveCallback callback);

void* CAUnicastReceiveThread(void* data);

void* CAUnicastSendThread(void* data);

void CASendUnicastMessage(char* address, void* data);

int32_t CASendUnicastMessageImpl(const char* address, const char* data, int32_t lengh);

void CAStopUnicastSendThread();

void* CAMulticastReceiveThread(void* data);

void* CAMulticastSendThread(void* data);

void CASendMulticastMessage(void* data);

int32_t CASendMulticastMessageImpl(const char* msg);

void CAStopMulticastSendThread();

void CAGetLocalAddress(char* addressBuffer);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
