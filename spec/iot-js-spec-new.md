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
A given sensor or actuator may be represented by multiple properties. A read-only property in a resource belongs to a sensor input, whereas a read-write property belongs to the state of an actuator.

The devices support for resources can be extended via installable software modules called *applications*.

This API enables writing the applications that implement resources and business logic.

OIC high level Web API design notes
-----------------------------------
The API design starts minimal, solves the use cases relevant to JS developers, and gets expanded on demand.

Code using this API is deployed to a device, which has one or more resources. In this version of the API it is assumed that the execution context of the code is separated for each device.

Therefore the **API entry point** is an object that exposes the local device functionality, and can be requested with a ```require``` statement which invokes the constructor. When the object is garbage collected, the implementations should clean up native state.

**Device identification** is UUID. Each device has an associated address + port.

When a device is constructed, the implementation should announce its presence.
Adding an event listener to the 'devicefound' event should turn on presence observing, i.e. the API implementation should make a request to watch presence notifications, and fire a 'devicefound' event when a presence notification is received.

**Resource identification** is URL path, relative to a given device.

On each device there are special resources, implementing device discovery, resource discovery, platform discovery, etc. Platform needs to be discoverable on a resource with a fixed URI ```/oic/p```, device on ```/oic/d``` and resources on ```/oic/res```. This API encapsulates these special resources and the hardcoded/fixed URIs by explicit function names and parameters.

**Device discovery** uses endpoint discovery: multicast request "GET /oic/res" to "All CoAP nodes" (224.0.1.187 for IPv4 and FF0X::FD for IPv6, port 5683). The response lists devices and their resources (at least URI, resource type, interfaces, and media types). Since this is basically a resource discovery, it is merged with resource discovery.

**Resource discovery** is based on the existence of resources (directories) set up for discovery. It can be achieved in 3 ways:
- direct discovery through peer inquiry (unicast or multicast)
- indirect discovery based on a 3rd party directory (a server for resource discovery)
- presence advertisment: the resource enabling discovery is local to the initiator, and it is maintained by presence notifications.

Implementations should encapsulate the resource discovery type, and should map the DiscoveryOptions values to the best suited protocol request(s).

In this API the OIC request (query) options and header options are also encapsulated. When they convey functionality, they are exposed through explicit properties and parameters.

Implementations should support automatic notifications for changed resources that have observers. However, a "manual" notify API is also exposed on the OicServer interface, for experimental purpose.

Web IDL of the JavaScript API
-----------------------------

This API uses [Promises](http://www.ecma-international.org/ecma-262/6.0/#sec-promise-objects).

### Error handling
Errors reported from OIC stack are mapped to [DOMError](https://heycam.github.io/webidl/#idl-DOMException-error-names) and exposed as [Error](http://www.ecma-international.org/ecma-262/6.0/#sec-error-objects). The following names may be used when rejecting Promises: ```NotSupportedError```, ```SecurityError```, ```TimeoutError```, ```NotFoundError```, ```NoModificationAllowedError```, ```InvalidModificationError```, ```TypeMismatchError```, ```InvalidStateError```, ```InvalidAccessError```, ```InvalidNodeTypeError```, ```NotReadableError```, ```IndexSizeError```, ```DataCloneError```.

The following error event is used for protocol errors:
```javascript
interface OicDeviceErrorEvent: Event {
  Error error;
  USVString url = "";  // URL of the device which failed (if available)
}
```
The valid error names are: ```NetworkError```, ```TimeoutError```, ```DataError```. OIC protocol errors may be mapped and included in the error message as text.

### OIC Device
The API entry point is the local device where the code is run (because the assumption that the execution context is not shared among devices).

When the constructor is invoked without parameters, the device is started in the default (server) role.
```javascript
var device = require('oic')();
```
If the device is forced in client-only mode,  the ```OicServer``` API is not available on it (all methods fail with ```NotSupportedError```).
```javascript
var device = require('oic')('client');
```

The ```require()``` call returns an initialized ```OicDevice``` object.

The OIC device contains functionality for reading settings, discovering remote devices and resources, adding and removing local resources, and creating remote resources.
```javascript
[Constructor(optional OicDeviceRole role)]
interface OicDevice: EventTarget {
  readonly attribute OicDeviceInfo info;

  // client API exposed on device
  Promise<OicResource> register(OicResourceInit resource);
  Promise<void> unregister(OicResourceId id);
  Promise<OicResource> create(OicResourceInit resource);
};

OicDevice implements OicDiscovery;
OicDevice implements OicServer;
OicDevice implements OicPresence;

enum OicDeviceRole { "client", "server" };  // client does not have OicServer

// the following info is exposed on /oic/p (platform) and /oic/d (device)
interface OicDeviceInfo {
  readonly attribute USVString uuid;
  readonly attribute USVString url;  // host:port
  readonly attribute OicDeviceRole role = "server";  // both client and server
  readonly attribute DOMString name = "";
  readonly attribute sequence<DOMString> dataModels = "";
    // list of <vertical>.major.minor, e.g. vertical = “Smart Home”
  readonly attribute DOMString coreSpecVersion = "";   // core.<major>.<minor>
  readonly attribute DOMString osVersion = "";         // from /oic/p
  readonly attribute DOMString model = "";             // from /oic/p
  readonly attribute DOMString manufacturerName = "";  // from /oic/p
  readonly attribute USVString manufacturerUrl = "";   // from /oic/p
  readonly attribute Date manufactureDate = "";        // from /oic/p
  readonly attribute DOMString platformVersion = "";   // from /oic/p
  readonly attribute DOMString firmwareVersion = "";   // from /oic/p
  readonly attribute USVString supportUrl = "";        // from /oic/p

  // setters may be supported later
};
```

### OIC Resource
The resources are represented as follows:
```javascript
typedef dictionary {
  USVString deviceId = "";  // UUID or URL (host:port)
  USVString path = "";  // resource path (short form)
  // query is omitted until proven needed
} OicResourceId;

dictionary OicResourceInit {
  OicResourceId id;
  sequence<DOMString> resourceTypes;
  sequence<DOMString> interfaces;
  sequence<DOMString> mediaTypes;
  boolean discoverable;
  boolean observable;
  boolean secure;
  boolean slow;
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

[NoInterfaceObject]
interface OicResource: EventTarget {
  // gets all properties of OicResourceInit, all read-only

  readonly attribute OicResourceRepresentation properties;

  // supporting resource hierarchies/collections (e.g. for ACL split)
  attribute sequence<OicResourceId> children;

  // resource client API
  Promise<void> retrieve();
  Promise<void> delete();
  Promise<void> update(optional OicResourceInit resource);  // partial dictionary

  attribute EventHandler<OicResourceUpdateEvent> onupdate;
  attribute EventHandler ondelete;  // simple event
};
```
Adding the first event listener to ```onupdate``` should send an observation request.

Removing the last event listener from ```onupdate``` should send an observation cancellation request.


### Discovery
Discovery is exposed by the following interface:
```javascript
[NoInterfaceObject]
interface OicDiscovery: EventTarget {
  // resource discovery: fire 'resourcefound' events on results
  Promise<void> findResources(optional OicDiscoveryOptions options);

  // get info on given remote device (no multicast supported by this call)
  Promise<OicDeviceInfo> getDeviceInfo(USVString url);

  // multicast device info request
  Promise<void> findDevices();  // fire a 'deviceinfo' event for each found

  attribute EventHandler<OicResourceFoundEvent> onresourcefound;
  attribute EventHandler<OicDeviceInfoEvent> ondeviceinfo;
  attribute EventHandler<OicDeviceErrorEvent> ondiscoveryerror;
}

dictionary OicDiscoveryOptions {
  USVString deviceId;      // if provided, make direct discovery
  DOMString resourceType;  // if provided, include this in the discovery request
  USVString resourcePath;  // if provided, filter the results locally
  // timeout could be included later, if needed
  // unsigned long long timeout = Infinity;  // in ms
};

interface OicResourceFoundEvent : Event {
  readonly attribute OicResource resource;
};

interface OicDeviceInfoEvent : Event {
  readonly attribute OicDeviceInfo deviceInfo;
};
```
When the ```findResources()``` method is invoked,
- Return a Promise object ```promise``` and continue [in parallel](https://html.spec.whatwg.org/#in-parallel).
- If the functionality is not supported, reject ```promise``` with ```NotSupportedError```.
- If there is no permission to use the method, reject ```promise``` with ```SecurityError```.
- Configure a discovery request on resources as follows:
  - If ```options.resourcePath``` is specified, filter results locally.
  - If ```options.deviceId``` is specified, make a direct discovery request to that device
  - If ```options.resourceType``` is specified, include it as the ```rt``` parameter in a new endpoint multicast discovery request ```GET /oic/res``` to "All CoAP nodes" (```224.0.1.187``` for IPv4 and ```FF0X::FD``` for IPv6, port ```5683```).
- If sending the request fails, reject the Promise with ```"NetworkError"```, otherwise resolve the Promise.
- When a resource is discovered, fire an ```onresourcefound``` event with an ```OicResourceFoundEvent``` structure, containing an ```OicResource``` object created from the received information.
- If there is an error during the discovery protocol, fire a ```discoveryerror``` event on the device with that error.

When the ```getDeviceInfo(url)``` method is invoked,
- Return a Promise object ```promise``` and continue [in parallel](https://html.spec.whatwg.org/#in-parallel).
- If the functionality is not supported, reject ```promise``` with ```NotSupportedError```.
- If there is no permission to use the method, reject ```promise``` with ```SecurityError```.
- Send a direct discovery request ```GET /oic/d``` to the given URL, and wait for the answer.
- If there is an error during the request, reject ```promise``` with that error.
- When the answer is received, resolve ```promise``` with an ```OicDeviceInfo``` object created from the response.


When the ```findDevices()``` method is invoked,
- Return a Promise object ```promise``` and continue [in parallel](https://html.spec.whatwg.org/#in-parallel).
- If the functionality is not supported, reject ```promise``` with ```NotSupportedError```.
- If there is no permission to use the method, reject ```promise``` with ```SecurityError```.
- Send a multicast request for retrieving ```/oic/d``` and wait for the answer.
- If the sending the request fails, reject the Promise with ```"NetworkError"```, otherwise resolve the Promise.
- If there is an error during the discovery protocol, fire a ```discoveryerror``` event on the device with that error.
- When a device information is discovered, fire a ```deviceinfo``` event with an ```OicDeviceInfoEvent``` structure, containing an ```OicDeviceInfo``` object.

### OIC Presence
The client API for accessing OIC Presence functionality.
```javascript
[NoInterfaceObject]
interface OicPresence: EventTarget {
  Promise<void> subscribe(optional USVString url);
  Promise<void> unsubscribe(optional USVString url);
  attribute EventHandler<OicDeviceEvent> ondevicechange;
};

// event received for device presence notifications
interface OicDeviceEvent : Event {
  readonly attribute enum { "added", "deleted", "changed" } type;
  readonly attribute USVString uuid;
  readonly attribute USVString url;  // host:port
  readonly attribute DOMString name;
};
```
When the ```subscribe()``` method is invoked, turn on presence listening for the specified ```deviceId```, or if it is not specified, for any device. The Promise may be rejected with ```NotSupportedError```.

### OIC Server
The server API provides functionality for handling requests.
```javascript
[NoInterfaceObject]
interface OicServer: EventTarget {
  // handle CRUDN requests from clients
  attribute EventHandler<OicObserveEvent> onobserverequest;
  attribute EventHandler<OicObserveEvent> onunobserverequest;
  attribute EventHandler<OicRequestEvent> onretrieverequest;
  attribute EventHandler<OicRequestEvent> ondeleterequest;
  attribute EventHandler<OicUpdateEvent>  onupdaterequest;
  attribute EventHandler<OicCreateEvent>  oncreaterequest;

  // update notification could be done automatically in most cases,
  // but in a few cases manual notification is needed
  Promise<void> notify(OicResourceInit resource);
  // delete notifications should be made automatic by implementations

  // enable/disable presence for this device
  Promise<void> enablePresence(optional unsigned long long ttl);  // in ms
  Promise<void> disablePresence();
};

interface OicRequestEvent : Event {
  readonly attribute OicResourceId source;
  readonly attribute OicResourceId target;

  Promise<void> sendResponse(optional OicResource? resource);
      // reuses request info (type, requestId, source, target) to construct response,
      // sends back “ok”, plus the resource object if applicable

  Promise<void> sendError(Error error);
      // reuses request info (type, requestId, source, target) to construct response,
      // sends an Error, where error.message maps to OIC errors, see later
};

interface OicObserveEvent : OicRequestEvent {
  OicResourceId resourceId;
};

interface OicCreateEvent : OicRequestEvent {
  readonly attribute OicResourceInit resource;
};

interface OicUpdateEvent : OicRequestEvent {
  readonly attribute OicResourceInit resource;
};
```

Code Examples
-------------
### Getting and setting device configuration

```javascript
var device = new OicDevice();
if (device.info.uuid) {  // configuration is valid
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
    id: { deviceId: device.info.uuid; path: "/light/ambience/blue" },
    resourceTypes: [ "Light" ],
    interfaces: [ "/oic/if/rw" ],
    discoverable: true,
    observable: true,
    properties: { color: "light-blue", dimmer: 0.2 }
  }).then((res) => {
    console.log("Local resource " + res.id.path + " has been registered.");
    lightResource = res;
    lightResource.addEventListener("update", onLightUpdate);
    lightResource.addEventListener("observe", onLightObserve);
    lightResource.addEventListener("delete", onLightDelete);
    }
  }).catch((error) => {
    console.log("Error creating resource " + res.id.path + " : " + error.message);
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

### OIC client functionality
```javascript
// e.g. to handle the case when remote resources can influence the states of local resources
// for instance look for the state changes on dimming of the remote red light,
// update dimming the local blue light, then request dimming the remote light
function startClient() {
  // discover resources
  var red = null;
  device.onresourcefound = function(event) {
    if(event.resource && event.resource.id.path == "/light/ambience/red") {
      red = event.resource;
      red.addEventListener('update', redHandler);
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
    red.update({ properties: { dimmer: 0.5 }})
      .then(() => { console.log("Changed red light dimmer"); })
      .catch(() => { console.log("Error changing red light"); });
  }
};
```
