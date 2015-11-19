IoT Web API
===========

Abstract
--------
This document presents a JavaScript API based on the [OIC](http://www.openinterconnect.org/)
[Core Specification](https://workspace.openinterconnect.org/apps/org/workgroup/cftg/download.php/1782/20150410v0.90%20OIC%20Core%20Specification.docx), and the C and [C++](https://api-docs.iotivity.org/0.9.0/index.html) API of the [IoTivity](https://www.iotivity.org/) project.

Introduction
------------
**IoT** (Internet of Things) is the name given for the complex environment in which diverse *resources*, implemented in *devices*, can be accessed remotely, and can notify subscribers with data and state changes. The resources act as servers, and communication with them may involve different protocols. Therefore, resources may also be represented by devices which translate between resource-specific and standard protocols. These devices are called *gateways*, or OIC intermediary devices.


Standardization is done by the [OIC](http://www.openinterconnect.org/) (Open Interconnect Consortium), which currently specifies:

  - the Core Framework for OIC core architecture, interfaces, protocols and services to enable OIC profiles implementation for IoT usages

  - Application Profiles Specification documents specify the OIC profiles to enable IoT usages for different market segments such as home, industrial, healthcare, and automotive.


The stakeholders of this environment include:

  - the IoT *platform* provides mechanisms for developing solutions, exposed through the platform (e.g. IoTivity) API's and protocols.

  - vertical *solutions* using sensors, actuators, and contextual data, with solution *providers* managing one or more vertical solutions e.g. with local or cloud based services

  - users with personal context.


The basic expected business patterns are the following:

  - a provider develops a solution, based on sensors and actuators,

  - an installable software module (called *application*) is deployed in a gateway or dedicated solution box on the field, and

  - a cloud service with user accounts and profiles, remote management etc.

Multiple providers/solutions can share a gateway (a.k.a. OIC intermediary device);
gateways' support for sensors can be extended via a software module as part of the solution.

A given solution owns the full vertical, including sensor handling, flow control, sampling, decisions, security and privacy aspects, remote configuration and fault management, accounting and billing etc. (the full FCAPS model).

For developers, this is a very complex environment to deal with, e.g. there may be platform-specific, provider-specific and solution- or user-specific applications.

In order to deal with this complexity, it is expected that a considerable number of providers will choose web-based development for at least the cloud part of their solutions, and once having web-related in-house competence they might like to leverage that in the full solution verticals, including the installable service modules deployed to gateways, which handle the sensors and actuators. As such, there is a possible need to also develop the installable service modules using web technologies, the JavaScript APIs are needed.

Another benefit for JS APIs is to convey the web development philosophy throughout the solution design, making it simpler and more efficient by adhering to contemporary development patterns: the World Wide Web proved that web technologies can possibly manage complexity even as high as in IoT.

Placeholder for OIC concepts and architecture
---------------------------------------------
Later, a digest of the OIC Core Specification may be presented here for a more extended context.


OIC high level Web API design notes
-----------------------------------
The API design starts minimal, solves the use cases relevant to JS developers, and gets expanded on demand.

This API is run on a device, either in client, server or intermediary role, which has one or more resources.

Device identification is UUID. Each device has an associated address + port.

Resource identification is URL, in full form or short form (containing only the <path> part, and the rest is fetched from the device owning the resource).

Platform needs to be discoverable on /oic/p, devices on /oic/d and resources on /oic/res but the API hides the hardcoded paths behind explicit function names/parameters.

The Core spec specifies discovery with resource type, whereas IoTivity API uses “uri”.
This API uses both a resource type and  URL (id) in DiscoveryOptions. Example:
     GET /oic/res?rt=oic.r.light

In this version of the API request (query) options (such as above) SHOULD be translated to explicit properties. Header options (from IoTivity) are omitted. Query options are omitted as well. They may be introduced in later versions, if needed, but explicit properties are better.

Presence handling is encapsulated and exposed as ‘observe’ methods and events.

Subscribing to event handlers means observe/notifications/presence are turned on; notifications are automatic for JS API, so notifyAll() and notify() are not exposed, but the user agent should encapsulate the observe/notify mechanism and expose only events.

In general, Promises may fail with Error, including: "NotSupportedError", "SecurityError", "TimeoutError", "NetworkError", "NotFoundError", "DataError".

Web IDL of the JavaScript API
-----------------------------
```javascript

// The API entry point is constructed. There could be more devices sharing a hardware.
[Constructor(optional OicDeviceSettings settings)]  // if not given, defaults are used
interface OicDevice : EventTarget {
  readonly attribute OicDeviceSettings settings;
  readonly attribute OicDeviceInfo info;
  Promise<void> configure(OicDeviceSettings settings); // partial dictionary is ok
      // equivalent to ‘onboard+configure’ in Core spec
      // maps to IoTivity Configure (C++ API), configure (Java API), OCInit (C API)
};
OicDevice implements OicServer;
OicDevice implements OicClient;

enum OicDeviceRole { "client", "server", "intermediary" };
enum OicConnectionMode { "confirmed", "best-effort", “default” };

dictionary OicDeviceSettings {
  USVString url;  // host:port
  OicDeviceRole role = "intermediary";  // both client and server
  OicConnectionMode connectionMode = "default";  // mapping to QoS in IoTivity
};

// the following info is exposed on /oic/p (platform) and /oic/d (device)
dictionary OicDeviceInfo {
  USVString uuid;
  DOMString name;
  DOMString[] dataModels = "";
    // list of <vertical>.major.minor, e.g. vertical = “Smart Home”
  DOMString coreSpecVersion = "";   // core.<major>.<minor>
  DOMString osVersion = "";         // from /oic/p
  DOMString model = "";             // from /oic/p
  DOMString manufacturerName = "";  // from /oic/p
  USVString manufacturerUrl = "";   // from /oic/p
  Date manufactureDate = "";        // from /oic/p
  DOMString platformVersion = "";   // from /oic/p
  DOMString firmwareVersion = "";   // from /oic/p
  USVString supportUrl = "";        // from /oic/p
};

// OIC URI addressing: scheme://authority/path?query
typedef dictionary {
  // scheme is generated by the implementation ('coap://', 'http://', ...)
  USVString deviceId = "";  // UUID of device, or host:port
  USVString path = "";  // resource path (short form)
  // query is omitted until proven needed
} OicResourceId;

// a snapshot of the custom properties of a Resource, together with the linked resources
dictionary OicResourceRepresentation {
  // any non-function properties that are JSON-serializable, e.g. string, number, boolean, URI
};

dictionary OicResource {
  OicResourceId id;
  DOMString[] resourceTypes;
  DOMString[] interfaces;
  boolean discoverable;
  boolean observable;
  // resource hierarchies, perhaps not needed in the first version
  readonly attribute OicResourceId? parent;  // id of parent
  readonly attribute sequence<OicResourceId> children;
  // additional, resource type specific properties are allowed as “mixin”
  OicResourceRepresentation properties;
};

[NoInterfaceObject]
interface OicDiscovery : EventTarget {
  // client API: discovery; Promise succeeds when request is successfully sent
  Promise<void> findResources(optional OicDiscoveryOptions options);  // GET oic/res
  Promise<void> findDevices();  // GET /oic/d

  attribute EventHandler<OicResourceFoundEvent> onresourcefound;  // discovery
  attribute EventHandler<OicDeviceFoundEvent> ondevicefound;  // discovery
  attribute EventHandler<OicDiscoveryErrorEvent> ondiscoveryerror;  // discovery
};

dictionary OicDiscoveryOptions {  // by default, find all
  USVString deviceId = "";
  USVString resourceURL = "";
  DOMString resourceType = "";
  // timeout could be included later, if needed
  // unsigned long long timeout = Infinity;  // in ms
};

// Errors reported from OIC stack are mapped to DOMError and exposed as Error:
// https://heycam.github.io/webidl/#idl-DOMException-error-names
// The following are mapped: "NetworkError", "TimeoutError", "DataError"
// OIC protocol errors may be included in the error message as text.
interface OicDiscoveryErrorEvent: Event {
  Error error;
  USVString? deviceId;
}

[NoInterfaceObject]
interface OicClient: EventTarget {
  // client API: CRUDN
  Promise<OicResource> createResource(OicResource resource);  // create remote res
  Promise<OicResource> retrieveResource(OicResourceId id);  // get remote res
  Promise<void> updateResource(OicResource resource);  // post or put remote res
  Promise<void> deleteResource(OicResourceId id);  // delete remote res
  Promise<void> observeResource(OicResourceId id);  // retrieve + observe flag
  Promise<void> unobserveResource(OicResourceId id);

  attribute EventHandler<OicResourceUpdateEvent> onresourceupdate;  // observe + notify
    // adding the first event listener to resourceupdate will send an observe request
    // removing the last event listener will cancel observing
};
OicClient implements OicDiscovery;

[NoInterfaceObject]
interface OicServer: EventTarget {
  // register/unregister locally constructed resource objects with OIC
  Promise<OicResource> registerResource(OicResource init);  // gets an id
  Promise<void> unregisterResource(OicResource resource);  // purge resource, then notify all

  // enable/disable presence (discovery, state changes) for this device and its resources
  Promise<void> enablePresence(optional unsigned long ttl);  // in seconds
  Promise<void> disablePresence();
  Promise<void> notify(OicResource resource, OicMethod method); // mostly “update”
  // handle CRUDN requests from clients
  attribute EventHandler<OicRequestEvent> onrequest;
};

typedef   enum {
  “init”, “observe”, “unobserve”, “create”, “retrieve”, “update”, “delete”
} OicMethod;

interface OicRequestEvent : Event {
  readonly attribute OicMethod type;
  readonly attribute unsigned long requestId;
  readonly attribute OicResourceId source;  // device uuid of the OIC client making the request
  readonly attribute OicResourceId target;  // id of the target resource
  readonly attribute OicResource? res;  // for “create” and “update” only
  Promise<void> sendResponse(optional OicResource? resource);  // for create, observe, retrieve
      // reuses request info (type, requestId, source, target) to construct response,
      // sends back “ok”, plus the resource object if applicable
  Promise<void> sendError(Error error);
      // reuses request info (type, requestId, source, target) to construct response,
      // sends an Error, where error.message maps to OIC errors, see later
};
// Note that the 'observe' and 'unobserve' events need to be generated from
// retrieve + observe flag requests in OIC.

interface OicResourceUpdateEvent : Event {
  OicResource resource;
};

interface OicResourceFoundEvent : Event {
  OicResource resource;
};

interface OicDeviceFoundEvent : Event {
  OicDeviceInfo device;
  sequence<OicResource> resources;  // path, resourceType, interfaces
};

```

Code Examples
-------------

### Getting and setting device configuration

```javascript
var device = new OicDevice();
// implementation will try to get default configuration from the HW/platform
// but if we cannot get it, could do a hard-coded configuration (not recommended)
if (device.settings.info.uuid) {  // configuration is valid
  startServer();
  startClient();
}
```

### OIC Server functionality

```javascript
var lightResource = null;
function startServer() {
  var deviceId = device.settings.info.uuid;
  // register all resources handled by this device
  device.registerResource({
    url: "/light/ambience/blue",
    deviceId: deviceId,
    resourceTypes: [ "Light" ],
    interfaces: [ "/oic/if/rw" ],
    discoverable: true,
    observable: true,
    properties: { color: "light-blue", dimmer: 0.2 }
  }).then((res) => {
    console.log("Resource " + res.id + " has been created.");
    lightResource = res;
    device.onrequest = requestHandler;
    device.enablePresence();
  }).catch((error) => {
    console.log("Error creating resource " + res.url + " : " + error.message);
  });
};

function requestHandler(request) {
  if (request.type == "update" && lightResource && request.target == lightResource.id) {
    var updates = [];
    foreach (name in request.properties) {  // e.g. "color"
      if (request.properties[name] != lightResource.properties[name]) {
        lightResource.properties[name] = request.properties[name];  // save the change
        request.sendResponse();  // sends back OK
        updates.push(name);  // collect all the updates
      }
    }
    if (updates.length) {
      device.notify(lightResource, "update");  // tell observers
    }
    request.sendError(new Error(“Failed to handle update request”));
  }
  // handle the other types of requests as well...
};
```

### OIC client functionality
```javascript
// e.g. to handle the case when remote resources can influence the states of  local resources
// for instance look for the state changes on dimming of the remote red light,
// update dimming the local blue light, then request dimming the remote light
function startClient() {
  // discover resources
  device.onresourcefound = function(event) {
    if(event.resource && event.resource.url == "/light/ambience/red")
      observe(device, res);
  }
  device.findResources({ resourceType: “Light” })
    .then( () => {
      console.log("Resource discovery started.");
    }).catch((e) => {
      console.log("Error finding resources: " + e.message);
    });
};

function observe(device, res) {
    device.observeResource(res)
        .then((red) => {
            console.log("Observing " + res.url);
            red.properties.dimmer = 0.5;
            device.updateResource(red.id, red)
              .then(() => { console.log("Changed red light dimmer"); })
              .catch(() => { console.log("Error changing red light"); });
         }).catch((error) => { console.log("Cannot observe " + res.url); };
};
```

Descriptions, algorithms
------------------------
To be updated.
TODO list:

  - work out in more detail how id mapping works both for devices and resources

  - document each function: JS API, mapping OIC flows, mapping native IoTivity calls

  - map error/result codes, add textual descriptions

  - include resource representations from other working groups/specifications (e.g. smart home).
