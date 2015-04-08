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
#ifndef _NETWORK_CONFIGURATOR_H_
#define _NETWORK_CONFIGURATOR_H_

#include "uarraylist.h"

#ifdef __cplusplus
extern "C"
{
#endif

static uint32_t NETWORK_ETHERNET = CA_ETHERNET;
static uint32_t NETWORK_WIFI = CA_WIFI;
static uint32_t NETWORK_EDR = CA_EDR;
static uint32_t NETWORK_LE = CA_LE;

CAResult_t CAAddNetworkType(uint32_t CAConnectivityType);

CAResult_t CARemoveNetworkType(uint32_t CAConnectivityType);

u_arraylist_t* CAGetSelectedNetworkList();

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif //#ifndef _NETWORK_CONFIGURATOR_H_
