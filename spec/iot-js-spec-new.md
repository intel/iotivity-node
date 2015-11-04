IoT Web API
===========

Abstract
--------
This document presents a JavaScript API based on the [OIC](http://www.openinterconnect.org/)
[Core Specification](http://openinterconnect.org/developer-resources/specs/), and the C and [C++](https://api-docs.iotivity.org/latest/index.html) API of the [IoTivity](https://www.iotivity.org/) project.

Introduction
------------
**IoT** (Internet of Things) is the name given for the complex environment in which diverse *resources*, implemented in *devices*, can be accessed remotely, and can notify subscribers with data and state changes. The resources act as servers, and communication with them may involve different protocols. Therefore, resources may also be represented by devices which translate between resource-specific and standard protocols. These devices are called *gateways*, or OIC intermediary devices.


Standardization is done by the [OIC](http://www.openinterconnect.org/) (Open Interconnect Consortium), which currently specifies:

  - the Core Framework for OIC core architecture, interfaces, protocols and services to enable OIC profiles implementation for IoT usages

  - Application Profiles Specification documents specify the OIC profiles to enable IoT usages for different market segments such as home, industrial, healthcare, and automotive.

Multiple providers/solutions can share a physical hardware *platform*.
A platform may host multiple physical or virtual *devices*.
Devices run the OIC software stack, and are addressable endpoints of communication. A device hosts multiple physical or virtual *resources*.
A resource represents *sensors* and *actuators*.
A given sensor or actuator may be represented by multiple properties, but usually one. A read-only property in a resource belongs to a sensor input, whereas a read-write property belongs to the state of an actuator.

The devices support for resources can be extended via installable software modules called *applications*, as part of a *solution*.

The basic expected business patterns are the following:

  - a provider develops a solution, based on sensors/actuators represented as resources

  - the provider deploys an application to a device that implements the solution specific resources

  - the provider optionally maintains a cloud service with user accounts, settings, remote management etc.

  - the installators/users provide personalized settings.

This API enables writing the applications that implement resources.

A given solution owns the full vertical, including resource handling, data flow control and sampling, local and cloud based decisions, security and privacy aspects, remote configuration and fault management, accounting and billing etc. (the full FCAPS model).

For developers, this is a very complex environment to deal with, e.g. there may be platform-specific, provider-specific and solution- or user-specific applications. Developers need to know the IoT architecture, endpoints and communication patterns. As these are designed using Web paradigms, the easiest way to develop solutions would be using web technologies.

Also, it is expected that a considerable number of providers will choose web-based development for at least the cloud part of their solutions, and once having web-related in-house competence they might want to leverage that in the full solution verticals, including the applications deployed to devices.

OIC high level Web API design notes
-----------------------------------
The API design starts minimal, solves the use cases relevant to JS developers, and gets expanded on demand.

Code using this API is deployed to a device, which has one or more resources. In this version of the API it is assumed that the execution context of the code is separated for each device.
Therefore the API entry point is an object that exposes the local device functionality, and can be requested with a 'require' statement which invokes the constructor. When the object is garbage collected, the implementations should clean up native state.

Device identification is UUID. Each device has an associated address + port.
When a device is constructed, the implementation should announce its presence.
Adding an event listener to the 'devicefound' event should turn on presence observing, i.e. the API implementation should make a request to watch presence notifications, and fire a 'devicefound' event when a presence notification is received.

Resource identification is URL, given in full form or relative form (which only the <path> part, and the rest is provided by the device owning the resource).

On each device there are special resources, implementing device discovery, resource discovery, platform discovery, etc. Platform needs to be discoverable on a resource with a fixed URI '/oic/p', device on '/oic/d' and resources on '/oic/res'. The API hides these special resources, and the hardcoded/fixed URIs behind explicit function names and parameters.

Device discovery uses endpoint discovery: multicast request "GET /oic/res" to "All CoAP nodes" (224.0.1.187 for IPv4 and FF0X::FD for IPv6, port 5683). The response lists devices and their resources (at least URI, resource type, interfaces, and media types).

Resource discovery is based on the existence of resources (directories) set up for discovery. It can be achieved in 3 ways:
- direct discovery through peer inquiry
- indirect discovery based on a 3rd party directory (a server for resource discovery)
- presence advertisment: the resource enabling discovery is local to the initiator, and it is maintained by presence notifications.
Implementations should encapsulate the resource discovery type, and should map the DiscoveryOptions values to the best suited protocol request(s).

In this API the OIC request (query) options and header options are also encapsulated. When they convey functionality, they are exposed through explicit properties and parameters.

Implementations should support automatic notifications for changed resources that have observers. However, a "manual" notify API is also exposed on the OicServer interface, for experimental purpose.

Web IDL of the JavaScript API
-----------------------------
The API entry point is the local device where the code is run.
When the constructor is invoked without parameters, the device gets the default configuration.
```javascript
var device = require('oic')();
```
It is possible to create the device and also provide partial or full configuration.
In the next example the role is provided by the custom configuration, and the device address and device information is fetched from the defaults.
```javascript
var device = require('oic')({ role: "server" });
```
When a device is started in strict client mode, then the OicServer API is not available on it (all methods fail with NotSupportedError), but OicDiscovery is available. When a device is started in 'server' or 'intermediary' role, the full functionality is available. The default role is 'server'.

The OIC device contains functionality for reading settings, configuring the device, discovering remote devices and resources, is described by the following interface:
```javascript
[Constructor(optional OicDeviceSettings settings)]
interface OicDevice: EventTarget {
  readonly attribute OicDeviceSettings settings;
  readonly attribute OicDeviceInfo info;

  Promise<void> configure(OicDeviceSettings settings); // partial dictionary is ok
      // equivalent to ‘onboard+configure’ in Core spec
      // maps to IoTivity Configure (C++ API), configure (Java API), OCInit (C API)

  // enable/disable presence for this device
  Promise<void> enablePresence(optional unsigned long long ttl);  // in ms
  Promise<void> disablePresence();

  // client API specific to devices
  Promise<OicResource> register(OicResourceInit resource);  // update discovery info
  Promise<OicResource> create(OicResourceInit resource);  // remote
};

OicDevice implements OicDiscovery;
OicDevice implements OicServer;

dictionary OicDeviceSettings {
  USVString url;  // host:port
  OicDeviceRole role;
};

enum OicDeviceRole { "client", "server", "intermediary" };

// the following info is exposed on /oic/p (platform) and /oic/d (device)
dictionary OicDeviceInfo {
  USVString uuid;
  DOMString name;
  DOMString[] dataModels;  // list of <vertical>.major.minor, e.g. vertical = “Smart Home”
  DOMString coreSpecVersion;   // core.<major>.<minor>
  DOMString osVersion;         // from /oic/p
  DOMString model;             // from /oic/p
  DOMString manufacturerName;  // from /oic/p
  USVString manufacturerUrl;   // from /oic/p
  Date manufactureDate;        // from /oic/p
  DOMString platformVersion;   // from /oic/p
  DOMString firmwareVersion;   // from /oic/p
  USVString supportUrl;        // from /oic/p
};
```

The resources are represented as follows:
```javascript
interface OicResource: EventTarget {
  // also gets all properties of OicResourceInit, all read-only
  readonly attribute OicResourceRepresentation properties;

  // resource client API
  Promise<void> retrieve();
  Promise<void> delete();
  Promise<void> update(optional OicResourceInit resource);  // partial dictionary

  attribute EventHandler<OicResourceUpdateEvent> onupdate;  // observe turned on if used
  attribute EventHandler ondelete;  // simple event
  attribute EventHandler<OicObserveEvent> onobserve;
};

dictionary OicResourceInit {
  USVString url;  // short form (path)
  USVString deviceId;  // opaque, usually UUID, mapped to address:port
  DOMString[] resourceTypes;
  DOMString[] interfaces;
  DOMString[] mediaTypes;
  boolean discoverable;
  boolean observable;
  OicResourceRepresentation properties;
};

// a snapshot of the custom properties of a Resource, together with the linked resources
dictionary OicResourceRepresentation {
  // DateTime timestamp;
  // other properties
};  // JSON serializable

interface OicResourceUpdateEvent: Event {
  readonly attribute OicResourceInit updates;
};
```
Discovery is exposed by the following interface:
```javascript
interface OicDiscovery: EventTarget {
  // client API: discovery; Promise resolves when request is successfully sent
  Promise<void> findDevices();  // GET /oic/d
  Promise<void> findResources(optional OicDiscoveryOptions options);

  attribute EventHandler<OicResourceFoundEvent> onresourcefound;  // discovery
  attribute EventHandler<OicDeviceFoundEvent> ondevicefound;  // discovery+presence
}

dictionary OicDiscoveryOptions {
  USVString deviceId;
  USVString resourceURL;
  DOMString resourceType;
};

interface OicResourceFoundEvent : Event {
  OicResource resource;
};

interface OicDeviceFoundEvent : Event {
  OicDeviceInfo device;
  sequence<OicResource> resources;  // of device, with limited number of properties
  // retrieveResource needs to be called for getting full resource (given deviceId and url).
};
```

The server API provides functionality for handling requests (and eventually notifications).
```javascript
interface OicServer: EventTarget {
  // handle CRUDN requests from clients
  attribute EventHandler<OicObserveEvent> onobserve;  // request with observe method
  attribute EventHandler<OicRequestEvent> onretrieve; // request with retrieve method
  attribute EventHandler<OicUpdateEvent>  onupdate;   // request with update method
  attribute EventHandler<OicRequestEvent> ondelete;   // request with delete method
  attribute EventHandler<OicCreateEvent>  oncreate;   // request with create method

  // update notification could be done automatically in most cases,
  // but in a few cases manual notification is needed
  Promise<void> notify(OicResourceInit resource);
  // delete notifications should be made automatic by implementations
};

interface OicRequestEvent : Event {
  readonly attribute USVString source;  // device uuid of the OIC client making the request
  readonly attribute USVString target;  // id of the target resource

  Promise<void> sendResponse(optional OicResource? resource);
      // reuses request info (type, requestId, source, target) to construct response,
      // sends back “ok”, plus the resource object if applicable

  Promise<void> sendError(Error error);
      // reuses request info (type, requestId, source, target) to construct response,
      // sends an Error, where error.message maps to OIC errors, see later
};

interface OicObserveEvent : OicRequestEvent {
  sequence<USVString> observers;  // updated list of observer URIs
};

interface OicCreateEvent : OicRequestEvent {
  readonly attribute OicResourceInit? res;
};

interface OicUpdateEvent : OicRequestEvent {
  readonly attribute OicResourceInit res;
};
```

Code Examples
-------------
### Getting and setting device configuration

```javascript
var device = new OicDevice();
if (device.settings.info.uuid) {  // configuration is valid
  startServer();
  startClient();
}
```

### OIC Server functionality

```javascript
var lightResource = null;
function startServer() {
  // register the specific resources handled by this solution
  // which are not exposed by the device firmware
  device.registerResource({
    url: "/light/ambience/blue",
    deviceId: device.settings.info.uuid,
    resourceTypes: [ "Light" ],
    interfaces: [ "/oic/if/rw" ],
    discoverable: true,
    observable: true,
    properties: { color: "light-blue", dimmer: 0.2 }
  }).then((res) => {
    console.log("Local resource " + res.id + " has been registered.");
    lightResource = res;
    lightResource.addEventListener("update", onLightUpdate);
    lightResource.addEventListener("observe", onLightObserve);
    lightResource.addEventListener("delete", onLightDelete);
    }
  }).catch((error) => {
    console.log("Error creating resource " + res.url + " : " + error.message);
  });
};

function onLightUpdate(event) {
  // the device has by now processed an update request to this resource (lightResource)
  // this is a hook to update the business logic
  console.log("Resource " + event.target + " updated. Running the update hook.");
  // after local processing, do the notifications manually
  device.notify(lightResource)
    .then( () => { console.log("Update notification sent.");})
    .catch( (err) => {
        console.log("No observers or error sending: " + err.name);
    });
};

function onLightObserve(event) {
  console.log("Resource " + event.target + " observer list changed.");
  lightResource.observers = event.observers;
  console.log("Running the observer hook.");
  device.notify(lightResource)
    .catch( (err) => {
      if(err.name == 'NoModificationAllowedError') {
        console.log("No observers.");
        // switch to minimal mode
      } else if(err.name == 'NetworkError') {
        console.log("Error sending notifications.");
      }
    });
  }
};

function onLightDelete(event) {
  console.log("Resource " + event.target + " has been requested to be deleted.");
  console.log("Running the delete hook.");
  // clean up local state
  // notification about deletion is automatic
};

```

In the above example, manual invocation of OicServer.notifyUpdate() and OicServer.notifyDelete() is not strictly needed. It would be enough the expose the update and delete hook, and after its completion, the implementation could automatically notify all observers (if any). The OIC core spec anyway mandates issuing these notifications, so this would also ensure clients cannot opt out from notifying.

After the delete hook completed, the resource must be deleted by the implementation, and all its observers notified. Similarly to update, this could be done automatically.

The observe hook could be used for managing local policies (e.g. update frequency, power management settings, etc).

### OIC client functionality
```javascript
// e.g. to handle the case when remote resources can influence the states of local resources
// for instance look for the state changes on dimming of the remote red light,
// update dimming the local blue light, then request dimming the remote light
function startClient() {
  // discover resources
  var red = null;
  device.onresourcefound = function(event) {
    if(event.resource && event.resource.url == "/light/ambience/red") {
      red = event.resource;
      red.addEventListener('update', redHandler);  // will also request observing
    }
  }
  device.findResources({ resourceType: “Light” })
    .then( () => { console.log("Resource discovery started.");})
    .catch((e) => {
      console.log("Error finding resources: " + e.message);
    });
};

function redHandler(red, updates) {
  console.log("Update received on " + red.id);
  console.lof("Running local business logic to determine further actions...");
  if (red.properties.dimmer > 0.5) {
    // do something, e.g. limit output
    red.properties.dimmer = 0.5;
    red.update({ properties: { dimmer: 0.5 }})
      .then(() => { console.log("Changed red light dimmer"); })
      .catch(() => { console.log("Error changing red light"); });
  }
};
```

Descriptions, algorithms
------------------------
### OicResource.findDevices
Signature:
```Promise<sequence<OicDeviceInfo>> findDevices();```

Steps:
- Returns a Promise object and continue asynhronously.
- Issue an endpoint discovery request on devices: a multicast request "GET /oic/res" to "All CoAP nodes" (224.0.1.187 for IPv4 and FF0X::FD for IPv6, port 5683).
- As devices are discovered one by one, fire an ```ondevicefound``` event with an ```OicDeviceFoundEvent``` structure, containing an ```OicDeviceInfo``` object and a sequence of OicResource objects, which should contain the information specified by discovery (usually URI, resource type, interfaces, and media types).
- When the discovery process is finished, resolve the Promise.
- If there is an error during discovery, reject Promise with the error.
(The devices found so far have been delivered via the ```ondevicefound``` event.)

### OicResource.findResources
Signature:
```Promise<sequence<OicResourceInfo>> findResources(OicDiscoveryOptions options);```

Steps:
- Returns a Promise object and continues asynhronously.
- Issue a discovery request on resources as follows:
  - if ```options.deviceId``` is specified, make a direct discovery request to that device
  - if ```options.resourceURL``` is specified, filter results based on it
  - if ```options.resourceType``` is specified, filter results based on it.
- As resources are discovered one by one, fire an ```onresourcefound``` event with an ```OicResourceFoundEvent``` structure, containing an ```OicResource``` object.
- When the discovery process is finished, resolve the Promise with a sequence of all ```OicResourceInfo``` objects discovered.
- If there is an error during discovery, reject Promise with the error.
(The resources found so far have been delivered via the ```onresourcefound``` event.)


### TODO list:

  - document each function: JS API, mapping OIC flows, mapping native IoTivity calls
  - map error/result codes, add textual descriptions
  - include resource representations from other working groups/specifications (e.g. smart home).
