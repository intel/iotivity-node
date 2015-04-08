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

#ifndef OCRESOURCE_H_
#define OCRESOURCE_H_

#define OC_OBSERVER_NOT_INTERESTED       (0)
#define OC_OBSERVER_STILL_INTERESTED     (1)
#define OC_OBSERVER_FAILED_COMM          (2)

//-----------------------------------------------------------------------------
// Virtual Resource Presence Attributes
//-----------------------------------------------------------------------------
#ifdef WITH_PRESENCE
typedef struct PRESENCERESOURCE{
    OCResourceHandle handle;
    uint32_t presenceTTL;
} PresenceResource;
#endif

//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------
struct rsrc_t;

//-----------------------------------------------------------------------------
// Typedefs
//-----------------------------------------------------------------------------

// IF here stands for Interface
typedef enum {
    STACK_IF_DEFAULT = 0,
    STACK_IF_LL,
    STACK_IF_BATCH,
    STACK_IF_GROUP,
    STACK_IF_INVALID
} OCStackIfTypes;

// following structure will be created in occollection.
typedef struct occapability {
    struct occapability* next;

    char *capability;           // It is a name about resource capability.
    char *status;               // It is mean status of capability.
} OCCapability;


// following structure will be created in occollection.
typedef struct ocaction {
    struct ocaction *next;

    // Target Uri.
    // It will be used to execute the action.
    char *resourceUri;

    OCCapability* head;
} OCAction;

// following structure will be created in occollection.
typedef struct ocactionset {

    struct ocactionset *next;

    char *actionsetName;

    OCAction* head;
} OCActionSet;



typedef struct resourcetype_t {
    struct resourcetype_t *next; // linked list; for multiple types on resource

    // Name of the type; this string is ‘.’ (dot) separate list of segments where each segment is a
    // namespace and the final segment is the type; type and sub-types can be separate with
    // ‘-‘ (dash) usually only two segments would be defined. Either way this string is meant to be
    // human friendly and is used opaquely and not parsed by code. This name is used in the “rt=”
    // parameter of a resource description when resources are introspected and is also use in the
    // <base URI>/types list of available types.
    char *resourcetypename;
} OCResourceType;

typedef struct attr_t {
    struct attr_t *next; // Points to next resource in list

    // The name of the attribute; used to look up the attribute in list;
    // for a given attribute SHOULD not be changed once assigned
    const char *attrName;
    char *attrValue; // value of the attribute as string
} OCAttribute;

typedef struct resourceinterface_t {
    struct resourceinterface_t *next; // linked list; for multiple interfaces on resource

    // Name of the interface; this is ‘.’ (dot) separate list of segments where each segment is a
    // namespace and the final segment is the interface; usually only two segments would be
    // defined. Either way this string is opaque and not parsed by segment
    char *name ;

    // Supported content types to serialize request and response on this interface
    // (REMOVE for V1 – only jSON for all but core.ll that uses Link Format)
#if 0
    char *inputContentType ;
    char *outputContentType ;
#endif
    /*** Future placeholder for access control and policy ***/
} OCResourceInterface;

typedef struct rsrc_t {
    struct rsrc_t *next; // Points to next resource in list
    // Relative path on the device; will be combined with base url to create fully qualified path
    char *host;
    char *uri;
    OCResourceType *rsrcType; // Resource type(s); linked list
    OCResourceInterface *rsrcInterface; // Resource interface(s); linked list
    OCAttribute *rsrcAttributes; // Resource interface(s); linked list
    // Array of pointers to resources; can be used to represent a container of resources
    // (i.e. hierarchies of resources) or for reference resources (i.e. for a resource collection)
    struct rsrc_t *rsrcResources[MAX_CONTAINED_RESOURCES];
    //struct rsrc_t *rsrcResources;
    // Pointer to function that handles the entity bound to the resource.
    // This handler has to be explicitly defined by the programmer
    OCEntityHandler entityHandler;
    // Properties on the resource – defines meta information on the resource
    OCResourceProperty resourceProperties ; /* ACTIVE, DISCOVERABLE etc */
    // Pointer to an opaque object where app/user specific data can be placed with the resource;
    // this could be information for the entity handler between invocations
    void *context;
    // NOTE: Methods supported by this resource should be based on the interface targeted
    // i.e. look into the interface structure based on the query request Can be removed here;
    // place holder for the note above
    /* method_t methods; */
    // Sequence number for observable resources. Per the CoAP standard it is a 24 bit value.
    uint32_t sequenceNum;
    // Pointer of ActionSet which to support group action.
    OCActionSet *actionsetHead;
} OCResource;



#endif /* OCRESOURCE_H_ */
