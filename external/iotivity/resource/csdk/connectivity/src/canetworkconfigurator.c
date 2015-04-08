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

#include "canetworkconfigurator.h"
#include "uarraylist.h"
#include "logger.h"
#include "cainterfacecontroller.h"

#define TAG PCF("CA")

static u_arraylist_t *gSelectedNetworkList = NULL;

CAResult_t CAAddNetworkType(uint32_t ConnectivityType)
{
    if (gSelectedNetworkList == NULL)
    {
        OIC_LOG_V(DEBUG, TAG, "Create network list");

        gSelectedNetworkList = u_arraylist_create();
    }

    switch (ConnectivityType)
    {
        case CA_ETHERNET:

#ifndef ETHERNET_ADAPTER
            OIC_LOG_V(DEBUG, TAG, "Add network type(ETHERNET) - Not Supported");
            return CA_NOT_SUPPORTED;
#endif /* ETHERNET_ADAPTER */

            OIC_LOG_V(DEBUG, TAG, "Add network type(ETHERNET)");
            u_arraylist_add(gSelectedNetworkList, &NETWORK_ETHERNET);

            break;

        case CA_WIFI:

#ifndef WIFI_ADAPTER
            OIC_LOG_V(DEBUG, TAG, "Add network type(WIFI) - Not Supported");
            return CA_NOT_SUPPORTED;
#endif /* WIFI_ADAPTER */

            OIC_LOG_V(DEBUG, TAG, "Add network type(WIFI)");
            u_arraylist_add(gSelectedNetworkList, &NETWORK_WIFI);

            break;

        case CA_EDR:

#ifndef EDR_ADAPTER
            OIC_LOG_V(DEBUG, TAG, "Add network type(EDR) - Not Supported");
            return CA_NOT_SUPPORTED;
#endif /* EDR_ADAPTER */

            OIC_LOG_V(DEBUG, TAG, "Add network type(EDR)");
            u_arraylist_add(gSelectedNetworkList, &NETWORK_EDR);
            break;

        case CA_LE:

#ifdef LE_ADAPTER
            OIC_LOG_V(DEBUG, TAG, "Add network type(LE) - Not Supported");
            return CA_NOT_SUPPORTED;
#endif /* LE_ADAPTER */

            OIC_LOG_V(DEBUG, TAG, "Add network type(LE)");
            u_arraylist_add(gSelectedNetworkList, &NETWORK_LE);
            break;

    }

    // start selected interface adapter
    CAStartAdapter(ConnectivityType);

    return CA_STATUS_OK;
}

CAResult_t CARemoveNetworkType(uint32_t ConnectivityType)
{
    if (gSelectedNetworkList == NULL)
    {
        OIC_LOG_V(DEBUG, TAG, "Selected network not found");

        return CA_STATUS_FAILED;
    }

    switch (ConnectivityType)
    {
        case CA_ETHERNET:

#ifndef ETHERNET_ADAPTER
            OIC_LOG_V(DEBUG, TAG, "Remove network type(ETHERNET) - Not Supported");
            return CA_NOT_SUPPORTED;
#else

            OIC_LOG_V(DEBUG, TAG, "Remove network type(ETHERNET)");
            u_arraylist_remove(gSelectedNetworkList, &NETWORK_ETHERNET);
#endif /* ETHERNET_ADAPTER */
            break;

        case CA_WIFI:

#ifndef WIFI_ADAPTER
            OIC_LOG_V(DEBUG, TAG, "Remove network type(WIFI) - Not Supported");
            return CA_NOT_SUPPORTED;
#else
            OIC_LOG_V(DEBUG, TAG, "Remove network type(WIFI)");
            u_arraylist_remove(gSelectedNetworkList, &NETWORK_WIFI);
#endif /* WIFI_ADAPTER */

            break;

        case CA_EDR:

#ifndef EDR_ADAPTER
            OIC_LOG_V(DEBUG, TAG, "Remove network type(EDR) - Not Supported");
            return CA_NOT_SUPPORTED;
#else
            OIC_LOG_V(DEBUG, TAG, "Remove network type(EDR)");
            u_arraylist_remove(gSelectedNetworkList, &NETWORK_EDR);
#endif /* EDR_ADAPTER */

            break;

        case CA_LE:

#ifdef LE_ADAPTER
            OIC_LOG_V(DEBUG, TAG, "Remove network type(LE) - Not Supported");
            return CA_NOT_SUPPORTED;
#else
            OIC_LOG_V(DEBUG, TAG, "Remove network type(LE)");
            u_arraylist_remove(gSelectedNetworkList, &NETWORK_LE);
#endif /* LE_ADAPTER */

            break;
    }

    // stop selected interface adapter
    CAStopAdapter(ConnectivityType);

    return CA_STATUS_OK;
}

u_arraylist_t* CAGetSelectedNetworkList()
{
    if (gSelectedNetworkList == NULL)
    {
        OIC_LOG_V(DEBUG, TAG, "Selected network not found");

        return NULL;
    }

    return gSelectedNetworkList;
}
