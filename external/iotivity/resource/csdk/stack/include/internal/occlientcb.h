//******************************************************************
//
// Copyright 2014 Intel Mobile Communications GmbH All Rights Reserved.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//

#ifndef OC_CLIENT_CB
#define OC_CLIENT_CB

#include <ocstack.h>
#include <occoaptoken.h>
#include <ocresource.h>

typedef struct OCPresence {
    // This is the TTL associated with presence
    uint32_t TTL;
    uint32_t * timeOut;
    uint32_t TTLlevel;
}OCPresence;

typedef struct OCMulticastNode {
    unsigned char * uri;
    uint32_t nonce;
    struct OCMulticastNode * next;
} OCMulticastNode;

extern OCMulticastNode * mcPresenceNodes;

typedef struct ClientCB {
    // callback method defined in application address space
    OCClientResponseHandler callBack;
    // callback context data
    void * context;
    // callback method to delete context data
    OCClientContextDeleter deleteCallback;
    //  when a response is recvd with this token, above callback will be invoked
    OCCoAPToken token;
    // Invocation handle tied to original call to OCDoResource()
    OCDoHandle handle;
    // This is used to determine if all responses should be consumed or not.
    // (For now, only pertains to OC_REST_OBSERVE_ALL Vs. OC_REST_OBSERVE functionality)
    OCMethod method;
    // This is the sequence identifier the server applies to the invocation tied to 'handle'.
    uint32_t sequenceNumber;
    // This is the request uri associated with the call back
    unsigned char * requestUri;
    // Struct to hold TTL info for presence
    #ifdef WITH_PRESENCE
    OCPresence * presence;
    OCResourceType * filterResourceType;
    #endif
    // next node in this list
    struct ClientCB    *next;
} ClientCB;

extern struct ClientCB *cbList;

//-- AddClientCB -----------------------------------------------------------
/** @ingroup ocstack
 *
 * This method is used to add a client callback method in cbList.
 *
 * @param[out] clientCB
 *              The resulting node from making this call. Null if out of memory.
 * @param[in] cb
 *              address to client callback function.
 * @param[in] token
 *              identifier for OTA CoAP comms.
 * @param[in] handle
 *              Masked in the public API as an 'invocation handle' - Used for callback management.
 * @param[in] requestUri
 *              the resource uri of the request.
 * @param[in] resourceType
 *              the resourceType associated with a presence request.
 *
 * @brief If the handle you're looking for does not exist, the stack will reply with a RST message.
 *
 * @retval OC_STACK_OK for Success, otherwise some error value
 */
//------------------------------------------------------------------------
OCStackResult AddClientCB(ClientCB** clientCB, OCCallbackData* cbData,
        OCCoAPToken * token, OCDoHandle *handle, OCMethod method,
        unsigned char * requestUri, unsigned char * resourceType);

//-- DeleteClientCB -----------------------------------------------------------
/** @ingroup ocstack
 *
 * This method is used to remove a callback node from cbList.
 *
 * @param[in] cbNode
 *              address to client callback node.
 */
//------------------------------------------------------------------------
void DeleteClientCB(ClientCB *cbNode);


//-- GetClientCB ---------------------------------------------------------
/** @ingroup ocstack
 *
 * This method is used to search a cb node in cbList.
 *
 * @param[in] token
 *              token to search for.
 * @param[in] handle
 *              handle to search for.
 * @param[in] requestUri
 *              Uri to search for.
 *
 * @brief You can search by token OR by handle. Not both.
 *
 * @retval address of the node if found, otherwise NULL
 */
//------------------------------------------------------------------------
ClientCB* GetClientCB(OCCoAPToken * token, OCDoHandle handle, unsigned char * requestUri);


/**
 * Inserts a new resource type filter into this clientCB node.
 *
 * @param cbNode - the node to add the new resourceType filter to
 * @param resourceTypeName - the value to create the new resourceType filter from
 *
 * @return
 *      OC_STACK_OK on success
 *      OC_STACK_ERROR with invalid parameters
 *      OC_STACK_NO_MEMORY when out of memory
 */
OCStackResult InsertResourceTypeFilter(ClientCB * cbNode, const char * resourceTypeName);

//-- DeleteClientCBList --------------------------------------------------
/** @ingroup ocstack
 *
 * This method is used to clear the cbList.
 *
 */
//------------------------------------------------------------------------
void DeleteClientCBList();

//-- FindAndDeleteClientCB -----------------------------------------------
/** @ingroup ocstack
 *
 * This method is used to verify the presence of a cb node in cbList
 * and then delete it.
 *
 * @param[in] cbNode
 *              address to client callback node.
 */
//------------------------------------------------------------------------
void FindAndDeleteClientCB(ClientCB * cbNode);

/** @ingroup ocstack
 *
 * This method is used to search a multicast presence node from list.
 *
 * @param[in]  uri
 *              the uri of the request.
 *
 * @return OCMulticastNode
 *              The resulting node from making this call. Null if doesn't exist.
 */
//------------------------------------------------------------------------
OCMulticastNode* GetMCPresenceNode(unsigned char * uri);

/** @ingroup ocstack
 *
 * This method is used to add a multicast presence node to the list.
 *
 * @param[out] outnode
 *              The resulting node from making this call. Null if out of memory.
 * @param[in] uri
 *              the uri of the server.
 * @param[in] nonce
 *              current nonce for the server
 *
 * @return OC_STACK_OK for Success, otherwise some error value
 */
//------------------------------------------------------------------------
OCStackResult AddMCPresenceNode(OCMulticastNode** outnode, unsigned char* uri, uint32_t nonce);

#endif //OC_CLIENT_CB
