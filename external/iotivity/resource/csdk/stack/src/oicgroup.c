//******************************************************************
//
// Copyright 2014 Samsung Electronics All Rights Reserved.
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

#define _POSIX_C_SOURCE 200112L
#include <string.h>

#include "cJSON.h"
#include "ocmalloc.h"
#include "oicgroup.h"
#include "ocresource.h"
#include "occollection.h"

#define TAG PCF("OICGROUP")

#define DESC_DELIMITER "\""
#define ACTION_DELIMITER "*"
#define ATTR_DELIMITER "|"

typedef struct aggregatehandleinfo
{
    OCServerRequest *ehRequest;
    OCDoHandle required;
    OCResource *collResource;

    struct aggregatehandleinfo *next;
} ClientRequstInfo;

// unsigned int nHandleIdx = 0;
// ClientRequstInfo g_AggregateResponseHandle[10];

ClientRequstInfo *clientRequstList = NULL;

void AddClientRequestInfo(ClientRequstInfo **head, ClientRequstInfo* add)
{
    ClientRequstInfo *tmp = NULL;

    if (*head != NULL)
    {
        tmp = *head;

        while (tmp->next)
        {
            tmp = tmp->next;
        }
        tmp->next = add;
    }
    else
    {
        *head = add;
    }
}

ClientRequstInfo* GetClientRequestInfo(ClientRequstInfo *head, OCDoHandle handle)
{
    ClientRequstInfo *tmp = NULL;

    tmp = head;

    if (tmp)
    {
        while (tmp)
        {
//            printf("%p :: %p\n", tmp->required, handle);
            if (tmp->required == handle)
            {
                break;
            }

            tmp = tmp->next;
        }

        return tmp;
    }
    return NULL;
}

void RemoveClientRequestInfo(ClientRequstInfo **head, ClientRequstInfo* del)
{
    ClientRequstInfo *tmp = NULL;

    if (*head == del)
    {
        *head = (*head)->next;
    }
    else
    {
        tmp = *head;
        while (tmp->next && (tmp->next != del))
        {
            tmp = tmp->next;
        }
        if (tmp->next)
        {
            tmp->next = del->next;
        }
    }
}



void AddCapability(OCCapability** head, OCCapability* node)
{
    OCCapability *pointer = *head;
    if (NULL == pointer)
    {
        *head = node;
    }
    else
    {
        while (pointer->next != NULL)
        {
            pointer = pointer->next;
        }

        pointer->next = node;
    }
}

void AddAction(OCAction** head, OCAction* node)
{
    OCAction *pointer = *head;
    if (NULL == pointer)
    {
        *head = node;
    }
    else
    {

        while (pointer->next != NULL)
        {
            pointer = pointer->next;
        }

        pointer->next = node;
    }
}

void AddActionSet(OCActionSet **head, OCActionSet* node)
{
    OCActionSet *pointer = *head;
    if (NULL == pointer)
    {
        *head = node;
    }
    else
    {

        while (pointer->next != NULL)
        {
            pointer = pointer->next;
        }

        pointer->next = node;
    }
}

void DeleteCapability(OCCapability *del)
{
    free(del->capability);
    del->capability = NULL;
    free(del->status);
    del->status = NULL;
}

void DeleteAction(OCAction** action)
{
    OCCapability* pointer = (*action)->head;
    OCCapability* pDel = NULL;

    while (pointer)
    {
        pDel = pointer;
        pointer = pointer->next;

        DeleteCapability(pDel);
        pDel->next = NULL;
    }
    OCFree((*action)->resourceUri);
    (*action)->resourceUri = NULL;
    (*action)->next = NULL;
}

void DeleteActionSet(OCActionSet** actionset)
{
    OCAction* pointer = (*actionset)->head;
    OCAction* pDel = NULL;

    while (pointer)
    {
        pDel = pointer;
        pointer = pointer->next;

        DeleteAction(&pDel);
        pDel->next = NULL;
    }

    OCFree((*actionset)->actionsetName);
    (*actionset)->head = NULL;
}

OCStackResult FindAndDeleteActionSet(OCResource **resource, const char * actionsetName)
{

    if (*resource != NULL)
    {

        OCActionSet *pointer = NULL;
        OCActionSet *pDel = NULL;

        pointer = (*resource)->actionsetHead;

        if (pointer == NULL)
        {
            return OC_STACK_ERROR;
        }
        else
        {
            if (strcmp(pointer->actionsetName, actionsetName) == 0)
            {
                if (pointer->next != NULL)
                    (*resource)->actionsetHead = pointer->next;
                else
                    (*resource)->actionsetHead = NULL;

                DeleteActionSet(&pointer);

            }
            else if (pointer->next != NULL)
            {
                while (pointer)
                {
                    if (pointer->next != NULL)
                    {
                        if (strcmp(pointer->next->actionsetName, actionsetName) == 0)
                        {
                            pDel = pointer->next;
                            pointer->next = pointer->next->next;

                            DeleteActionSet(&pDel);
                        }
                    }
                }
            }

            return OC_STACK_OK;
        }

    }

    return OC_STACK_ERROR;
}

OCStackResult DeleteActionSets(OCResource** resource)
{
    OCActionSet *pointer = (*resource)->actionsetHead;
    OCActionSet *pDel = pointer;

    while (pointer)
    {
        pDel = pointer;
        pointer = pointer->next;

        DeleteActionSet(&pDel);
        pDel->next = NULL;
    }

    (*resource)->actionsetHead = NULL;
    return OC_STACK_OK;
}

OCStackResult GetActionSet(const char *actionName, OCActionSet *head, OCActionSet** actionset)
{
    OCActionSet *pointer = head;

    while (pointer)
    {
        if (strcmp(pointer->actionsetName, actionName) == 0)
        {
            *actionset = pointer;
            return OC_STACK_OK;
        }

        pointer = pointer->next;
    }

    return OC_STACK_ERROR;

}

OCStackResult GetActionSetFromString(OCResource **resource, unsigned char *request, char** method,
        char **actionsetName)
{
    char *acitonRequest;
    char *iterTokenPtr = NULL;
    char *iterToken = NULL;
    char *description = NULL;
    char *iterDescPtr = NULL;

    char *attributes = NULL;
    char *iterAttrbutesPtr = NULL;

    char *attr = NULL;
    char *iterAttrPtr = NULL;

    OCActionSet* actionset = NULL;
    OCAction* action = NULL;

    acitonRequest = (char *) OCMalloc(strlen((char *) request) + 1);
    strncpy(acitonRequest, (char *) request, strlen((char *) request) + 1);

    //printf("\t%s\n", acitonRequest);
    if (acitonRequest != NULL)
    {
        iterToken = (char *) strtok_r(acitonRequest, DESC_DELIMITER, &iterTokenPtr);

        while (iterToken != NULL)
        {
            if (strcmp(iterToken, "ActionSet") == 0)
            { // if iterToken is ActionSet, will be created and added a new action set.

                *method = (char *) OCMalloc(strlen(iterToken) + 1);
                strncpy(*method, iterToken, strlen(iterToken) + 1);

                //GetActionName(iterToken, &actionsetName);
                // printf("%s\n", iterToken, &iterTokenPtr);
                iterToken = (char *) strtok_r(NULL, DESC_DELIMITER, &iterTokenPtr); // it is mean ':'.
                // printf("%s\n", iterToken);
                iterToken = (char *) strtok_r(NULL, DESC_DELIMITER, &iterTokenPtr); // it is body of action description.
                // printf("%s\n", iterToken);

                // printf("DESC :: %s\n", iterToken);
                description = (char *) OCMalloc(strlen(iterToken) + 1);
                strncpy(description, iterToken, strlen(iterToken) + 1);
                // printf("DESC Copied :: %s\n", description);

                // Find the action name from description.
                iterDescPtr = NULL;
                iterToken = (char *) strtok_r(description, ACTION_DELIMITER, &iterDescPtr);
                //while(iterToken != NULL)
                if (iterToken != NULL)
                {
                    if (*actionsetName != NULL)
                    {
                        // printf("ERROR :: ACTIONSET NAME as ActionSet(%s)\n", iterToken);
                        return OC_STACK_ERROR; // ERROR OCCURED.
                    }
                    else
                    {
                        //  Actionset name.
                        // printf("ACTION SET NAME :: %s\n", iterToken);
                        *actionsetName = (char *) OCMalloc(strlen(iterToken) + 1);

                        strncpy(*actionsetName, iterToken, strlen(iterToken) + 1);
                        // printf("ACTION SET NAME :: %s\n", *actionsetName);
                        // break;
                    }

                    iterToken = (char *) strtok_r(NULL, ACTION_DELIMITER, &iterDescPtr);
                }
                else
                {
                    return OC_STACK_ERROR;

                } // end Action Set Name.

                // New ActionSet Add to OCResource's ActionSet list.
                // 1. Allocate a new pointer for actionset.
                actionset = (OCActionSet*) OCMalloc(sizeof(OCActionSet));
                // 2. Initiate actionset.
                memset(actionset, 0, sizeof(OCActionSet));
                actionset->actionsetName = (char *) OCMalloc(strlen(*actionsetName) + 1);
                strncpy(actionset->actionsetName, *actionsetName, strlen(*actionsetName) + 1);
                // printf("ACTION SET NAME :: %s\n", actionset->actionsetName);

                while (iterToken != NULL)
                {
                    action = (OCAction *) OCMalloc(sizeof(OCAction));
                    memset(action, 0, sizeof(OCAction));

                    // printf("ATTR Copied :: %s\n", iterToken);
                    attributes = (char *) OCMalloc(strlen(iterToken) + 1);
                    strncpy(attributes, iterToken, strlen(iterToken) + 1);
                    // printf("ATTR Copied :: %s\n", attributes);

                    iterToken = (char *) strtok_r(attributes, ATTR_DELIMITER, &iterAttrbutesPtr);
                    while (iterToken != NULL)
                    {
                        attr = (char *) OCMalloc(strlen(iterToken) + 1);
                        strncpy(attr, iterToken, strlen(iterToken) + 1);

                        iterToken = (char *) strtok_r(attr, "=", &iterAttrPtr);
                        while (iterToken != NULL)
                        {
                            // Find the URI from description.
                            if (strcmp(iterToken, "uri") == 0)
                            {
                                iterToken = (char *) strtok_r(NULL, "=", &iterAttrPtr);
                                //printf("uri :: %s\n", iterToken);
                                action->resourceUri = (char *) OCMalloc(strlen(iterToken) + 1);
                                strncpy(action->resourceUri, iterToken, strlen(iterToken) + 1);
                            }
                            else
                            {
                                OCCapability* capa = (OCCapability*) OCMalloc(sizeof(OCCapability));
                                memset(capa, 0, sizeof(OCCapability));
                                //printf("%s :: ", iterToken);
                                capa->capability = (char *) OCMalloc(strlen(iterToken) + 1);
                                strncpy(capa->capability, iterToken, strlen(iterToken) + 1);
                                iterToken = (char *) strtok_r(NULL, "=", &iterAttrPtr);
                                //printf("%s\n", iterToken);
                                capa->status = (char *) OCMalloc(strlen(iterToken) + 1);
                                strncpy(capa->status, iterToken, strlen(iterToken) + 1);

                                AddCapability(&action->head, capa);
                            }

                            iterToken = (char *) strtok_r(NULL, "=", &iterAttrPtr);
                        }

                        iterToken = (char *) strtok_r(NULL, ATTR_DELIMITER, &iterAttrbutesPtr);
                    } // End of Action

                    AddAction(&actionset->head, action);

                    iterToken = (char *) strtok_r(NULL, ACTION_DELIMITER, &iterDescPtr);
                }

                // 3. Add the pointer OCResource's ActionSet list.
                AddActionSet(&(*resource)->actionsetHead, actionset);
                return OC_STACK_OK;
            }
            else if (strcmp(iterToken, "DoAction") == 0 || strcmp(iterToken, "DelActionSet") == 0
                    || strcmp(iterToken, "GetActionSet") == 0)
            {
                *method = (char *) OCMalloc(strlen(iterToken) + 1);
                strncpy(*method, iterToken, strlen(iterToken) + 1);

                iterToken = (char *) strtok_r(NULL, DESC_DELIMITER, &iterTokenPtr); // it is mean ':'.
                // printf("%s\n", iterToken);
                iterToken = (char *) strtok_r(NULL, DESC_DELIMITER, &iterTokenPtr); // it is body of action description.
                // printf("%s\n", iterToken);

                description = (char *) OCMalloc(strlen(iterToken) + 1);
                strncpy(description, iterToken, strlen(iterToken) + 1);

                // Find the action name from description.
                iterDescPtr = NULL;
                iterToken = (char *) strtok_r(description, ACTION_DELIMITER, &iterDescPtr);
                if (iterToken != NULL)
                {
                    if (*actionsetName != NULL)
                    {
                        // printf("ERROR :: ACTIONSET NAME as ActionSet(%s)\n", iterToken);
                        return OC_STACK_ERROR; // ERROR OCCURED.
                    }
                    else
                    {
                        //  Actionset name.
                        // printf("ACTION SET NAME :: %s\n", iterToken);
                        *actionsetName = (char *) OCMalloc(strlen(iterToken) + 1);

                        strncpy(*actionsetName, iterToken, strlen(iterToken) + 1);
                        // printf("ACTION SET NAME :: %s\n", *actionsetName);
                    }

                    iterToken = (char *) strtok_r(NULL, ACTION_DELIMITER, &iterDescPtr);
                    return OC_STACK_OK;
                }
                else
                {
                    return OC_STACK_ERROR;

                } // end Action Set Name.
                break;
            }

            iterToken = (char *) strtok_r(NULL, DESC_DELIMITER, &iterTokenPtr);
        }
    }

    return OC_STACK_ERROR;
}

OCStackResult GetStringFromActionSet(OCActionSet* actionset, char** desc)
{
    char temp[1024] =
    { 0 };
    int remaining = 1023;

    // OCActionSet *as = resource->actionsetHead;
    // while(as != NULL)
    // {
    printf("\n\n\nAction Set Name :: %s\n", actionset->actionsetName);
    OCAction *action = actionset->head;

    if (remaining >= strlen(actionset->actionsetName) + 1)
    {
        strcat(temp, actionset->actionsetName);
        remaining -= strlen(actionset->actionsetName);
        strcat(temp, "*");
        remaining--;
    }
    else
    {
        return OC_STACK_ERROR;
    }

    while (action != NULL)
    {
        printf("\tURI :: %s\n", action->resourceUri);
        strcat(temp, "uri=");
        remaining -= strlen("uri=");
        strcat(temp, action->resourceUri);
        remaining -= strlen(action->resourceUri);
        strcat(temp, "|");
        remaining--;

        OCCapability *capas = action->head;
        while (capas != NULL)
        {
            printf("\t\t%s = %s\n", capas->capability, capas->status);
            strcat(temp, capas->capability);
            remaining -= strlen(capas->capability);
            strcat(temp, "=");
            remaining--;
            strcat(temp, capas->status);
            remaining -= strlen(capas->capability);

            capas = capas->next;
            if (capas != NULL)
            {
                strcat(temp, "|");
            }
        }

        action = action->next;
        if (action != NULL)
        {
            strcat(temp, "*");
            remaining--;
        }
    }
    //     as = as->next;
    // }

    *desc = (char *) OCMalloc(1024 - remaining);
    strcpy(*desc, temp);
    // printf("\t\tPlain Text = %s(%i)\n", *desc, 1024 - remaining);

    return OC_STACK_OK;
}

OCStackApplicationResult ActionSetCB(void* context, OCDoHandle handle,
        OCClientResponse* clientResponse)
{
    printf("\n\n\tcallback is called\n\n");

    ClientRequstInfo *info = GetClientRequestInfo(clientRequstList, handle);

    if (info)
    {
        int idx;

        unsigned char *responseJson;
        responseJson = (unsigned char *) OCMalloc(
                (unsigned int) (strlen((char *) clientResponse->resJSONPayload) + 1));

        // We need the body of response.
        // Copy the body from the response
        strcpy((char *) responseJson, ((char *) clientResponse->resJSONPayload
                + OC_JSON_PREFIX_LEN));
        idx = strlen((char *) responseJson) - OC_JSON_SUFFIX_LEN;
        // And insert NULL at the end of body.
        (responseJson[idx]) = 0;

        OCEntityHandlerResponse response = { 0 };
        response.ehResult = OC_EH_OK;
        response.payload = responseJson;
        response.payloadSize = (unsigned int) strlen((char *) responseJson) + 1;
        response.persistentBufferFlag = 0;
        response.requestHandle = (OCRequestHandle) info->ehRequest;
        response.resourceHandle = (OCResourceHandle) info->collResource;

        OCDoResponse(&response);

        RemoveClientRequestInfo(&clientRequstList, info);
        OCFree(responseJson);
    }

    // g_AggregateResponseHandle

    return OC_STACK_KEEP_TRANSACTION;
}

void ActionSetCD(void *context)
{
    // printf("\n\t\tCD is called\n");

    // free( context );
}

OCStackResult BuildActionJSON(OCAction* action, unsigned char* bufferPtr, uint16_t *remaining)
{
    OCStackResult ret = OC_STACK_ERROR;
    cJSON *json;
    cJSON *body;

    char *jsonStr;
    uint16_t jsonLen;

    OC_LOG(INFO, TAG, PCF("Entering BuildActionJSON"));
    json = cJSON_CreateObject();

    cJSON_AddItemToObject(json, "rep", body = cJSON_CreateObject());

    OCCapability* pointerCapa = action->head;
    while (pointerCapa)
    {
        cJSON_AddStringToObject(body, pointerCapa->capability, pointerCapa->status);
        pointerCapa = pointerCapa->next;
    }

    jsonStr = cJSON_PrintUnformatted(json);

    jsonLen = strlen(jsonStr);
    if (jsonLen < *remaining)
    {
        strcat((char*) bufferPtr, jsonStr);
        *remaining -= jsonLen;
        bufferPtr += jsonLen;
        ret = OC_STACK_OK;
    }

    cJSON_Delete(json);
    free(jsonStr);

    return ret;
}

unsigned int GetNumOfTargetResource(OCAction *actionset)
{
    int numOfREsource = 0;

    OCAction *pointerAction = actionset;

    while (pointerAction != NULL)
    {
        numOfREsource++;
        pointerAction = pointerAction->next;
    }

    return numOfREsource;
}

OCStackResult SendAction(OCDoHandle *handle, const char *targetUri, const unsigned char *action)
{
    OCCallbackData cbdata =
    { 0 };
    cbdata.cb = &ActionSetCB;
    cbdata.cd = &ActionSetCD;
    cbdata.context = (void *) 0x99;

    return OCDoResource(handle, OC_REST_PUT, targetUri,
    //temp->rsrcType->resourcetypename,
            NULL, (char *) action, OC_NA_QOS, &cbdata, NULL, 0);
}

OCStackResult BuildCollectionGroupActionJSONResponse(OCMethod method/*OCEntityHandlerFlag flag*/,
        OCResource *resource, OCEntityHandlerRequest *ehRequest)
{
    OCStackResult stackRet = OC_STACK_ERROR;

    OC_LOG(INFO, TAG, PCF("Group Action is requested."));
    // if (stackRet == OC_STACK_OK)
    {

        char *doWhat = NULL;
        char *actionName = NULL;

        size_t bufferLength = 0;
        unsigned char buffer[MAX_RESPONSE_LENGTH] =
        { 0 };
        unsigned char *bufferPtr = NULL;

        bufferPtr = buffer;

        OCResource * collResource = (OCResource *) ehRequest->resource;

        char *jsonResponse;

        cJSON *json;
        cJSON *format;

        if (method == OC_REST_PUT)
        {
            json = cJSON_CreateObject();
            cJSON_AddStringToObject(json, "href", resource->uri);
            cJSON_AddItemToObject(json, "rep", format = cJSON_CreateObject());

            OC_LOG(INFO, TAG, PCF("Group Action[PUT]."));

            unsigned char *actionPtr = (unsigned char *) ehRequest->reqJSONPayload;
            GetActionSetFromString(&resource, actionPtr, &doWhat, &actionName);

            if (strcmp(doWhat, "DelActionSet") == 0)
            {
                if (FindAndDeleteActionSet(&resource, actionName) == OC_STACK_OK)
                {
                    stackRet = OC_STACK_OK;
                }
                else
                {
                    stackRet = OC_STACK_ERROR;
                }
            }

            jsonResponse = cJSON_Print(json);
            cJSON_Delete(json);

            strcat((char *) bufferPtr, jsonResponse);

            bufferLength = strlen((const char *) buffer);
            if (bufferLength > 0)
            {
                OCEntityHandlerResponse response =
                { 0 };
                response.ehResult = OC_EH_OK;
                response.payload = buffer;
                response.payloadSize = bufferLength + 1;
                response.persistentBufferFlag = 0;
                response.requestHandle = (OCRequestHandle) ehRequest->requestHandle;
                response.resourceHandle = (OCResourceHandle) collResource;
                stackRet = OCDoResponse(&response);
            }

            stackRet = OC_STACK_OK;
        }

        if (method == OC_REST_POST)
        {
            OC_LOG(INFO, TAG, PCF("Group Action[POST]."));

            OCActionSet *actionset = NULL;
            unsigned char *actionPtr = (unsigned char *) ehRequest->reqJSONPayload;

            GetActionSetFromString(&resource, actionPtr, &doWhat, &actionName);

            json = cJSON_CreateObject();
            cJSON_AddStringToObject(json, "href", resource->uri);

            if (strcmp(doWhat, "DoAction") == 0)
            {
                if (GetActionSet(actionName, resource->actionsetHead, &actionset) != OC_STACK_OK)
                {
                    OC_LOG(INFO, TAG, PCF("ERROR"));
                    stackRet = OC_STACK_ERROR;
                }

                if (actionset == NULL)
                {
                    OC_LOG(INFO, TAG, PCF("ERROR"));
                    stackRet = OC_STACK_ERROR;
                }
                else
                {

                    OCAction *pointerAction = actionset->head;

                    unsigned int num = GetNumOfTargetResource(pointerAction);

                    ((OCServerRequest *) ehRequest->requestHandle)->ehResponseHandler =
                            HandleAggregateResponse;
                    ((OCServerRequest *) ehRequest->requestHandle)->numResponses = num + 1;

//                    printf("ActionSet Name :: %s\n", actionset->actionsetName);
                    while (pointerAction != NULL)
                    {
                        unsigned char actionDesc[MAX_RESPONSE_LENGTH] = { 0 };
                        unsigned char* actionDescPtr = actionDesc;
                        uint16_t remaining = MAX_RESPONSE_LENGTH;

                        strcpy((char *) actionDescPtr, (const char *) OC_JSON_PREFIX);
                        BuildActionJSON(pointerAction, actionDescPtr, &remaining);
                        strcat((char *) actionDescPtr, (const char *) OC_JSON_SUFFIX);

                        ClientRequstInfo *info = (ClientRequstInfo *) OCMalloc(
                                sizeof(ClientRequstInfo));
                        memset(info, 0, sizeof(ClientRequstInfo));

                        info->collResource = resource;
                        info->ehRequest = (OCServerRequest *) ehRequest->requestHandle;

                        SendAction(&info->required, pointerAction->resourceUri, actionDescPtr);

                        AddClientRequestInfo(&clientRequstList, info);


                        pointerAction = pointerAction->next;
                    }


                    stackRet = OC_STACK_OK;
                }
            }
            else if (strcmp(doWhat, "GetActionSet") == 0)
            {
                char *plainText = NULL;
                OCActionSet *actionset = NULL;

                cJSON_AddItemToObject(json, "rep", format = cJSON_CreateObject());
                GetActionSet(actionName, resource->actionsetHead, &actionset);
                if (actionset != NULL)
                {
                    GetStringFromActionSet(actionset, &plainText);

                    if (plainText != NULL)
                    {
                        cJSON_AddStringToObject(format, "ActionSet", plainText);
                    }

                    stackRet = OC_STACK_OK;
                }
            }

            jsonResponse = cJSON_Print(json);
            cJSON_Delete(json);

            strcat((char *) bufferPtr, jsonResponse);

            bufferLength = strlen((const char *) buffer);
            if (bufferLength > 0)
            {
                OCEntityHandlerResponse response =
                { 0 };
                response.ehResult = OC_EH_OK;
                response.payload = buffer;
                response.payloadSize = bufferLength + 1;
                response.persistentBufferFlag = 0;
                response.requestHandle = (OCRequestHandle) ehRequest->requestHandle;
                response.resourceHandle = (OCResourceHandle) collResource;
                stackRet = OCDoResponse(&response);
            }
        }
    }

    return stackRet;
}
