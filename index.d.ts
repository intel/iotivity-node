// Type definitions for iotivity-node 1.2.0-2
// Project: iotivity-node
// Definitions by: Hans Bakker https://www.github.com/wind-rider

/// <reference types="node" />

import { EventEmitter } from "events";

/**
 * Exposes information about the OCF device that runs the current OCF stack instance.
 * 
 * @interface ocf.OcfDevice
 */
export interface OcfDevice {

    /**
     * UUID of the device
     * 
     * @type {string}
     * @memberOf OcfDevice
     */
    uuid: string;

    /**
     * host:port
     * 
     * @type {string}
     * @memberOf OcfDevice
     */
    url?: string;

    /**
     * Name of the device
     * 
     * @type {string}
     * @memberOf OcfDevice
     */
    name?: string;

    /**
     * List of supported OCF data models.
     * The dataModels property is in the following format: vertical.major.minor where major and minor are numbers and vertical is a string such as "Smart Home".
     * 
     * @type {string[]}
     * @memberOf OcfDevice
     */
    dataModels: string[];

    /**
     * OCF Core Specification version
     * 
     * @type {string}
     * @memberOf OcfDevice
     */
    coreSpecVersion: string;
}

/**
 * Exposes information about the OCF platform that hosts the current device.
 * 
 * @export
 * @interface OcfPlatform
 */
export interface OcfPlatform {

    /**
     * Platform identifier
     * 
     * @type {string}
     * @memberOf OcfPlatform
     */
    id: string;

    /**
     * OS version
     * 
     * @type {string}
     * @memberOf OcfPlatform
     */
    osVersion?: string;

    /**
     * Model of the hardware
     * 
     * @type {string}
     * @memberOf OcfPlatform
     */
    model?: string;

    /**
     * Manufacturer name
     * 
     * @type {string}
     * @memberOf OcfPlatform
     */
    manufacturerName: string;

    /**
     * Manufacturer web page
     * 
     * @type {string}
     * @memberOf OcfPlatform
     */
    manufacturerURL: string;

    /**
     * Manufacturing date
     * 
     * @type {Date}
     * @memberOf OcfPlatform
     */
    manufacturerDate: Date;

    /**
     * Platform version
     * 
     * @type {string}
     * @memberOf OcfPlatform
     */
    platformVersion: string;

    /**
     * Firmware version
     * 
     * @type {string}
     * @memberOf OcfPlatform
     */
    firmwareVersion: string;

    /**
     * Product support web page
     * 
     * @type {string}
     * @memberOf OcfPlatform
     */
    supportURL: string;
}

/**
 * TODO follow implementation or spec?
 * 
 * @export
 * @abstract
 * @class OcfError
 * @extends {Error}
 */
export abstract class OcfError extends Error {
    /**
     * UUID of the device.
     * Represents the device UUID causing the error. The value null means the local device,
     * and the value undefined means the error source device is not available.
     * @type {string}
     * @memberOf OcfError
     */
    deviceId?: string;

    /**
     * URI path of the resource.
     * Represents the resource path of the resource causing the error.
     * If deviceId is undefined, then the value of resourcePath should also be set to undefined.
     * @type {string}
     * @memberOf OcfError
     */
    resourcePath?: string;
}

/**
 * Identifies an OCF resource by the UUID of the device that hosts the resource,
 * and the URI path of the resource that uniquely identifies the resource inside a device.
 * 
 * @export
 * @interface ResourceId
 */
export interface ResourceId {

    /**
     * UUID of the device
     * 
     * @type {string}
     * @memberOf ResourceId
     */
    deviceId: string;

    /**
     * URI path of the resource
     * 
     * @type {string}
     * @memberOf ResourceId
     */
    resourcePath: string;
}

/**
 * Represents an OCF resource Link used in Collections.
 * The {@link discoverable} and {@link observable} properties come from the {@link p} (policy) property of a Web Link.
 * @export
 * @interface ResourceLink
 * @extends {ResourceId}
 */
export interface ResourceLink extends ResourceId {
    /**
    * List of OCF resource types
    * 
    * @type {string[]}
    * @memberOf ResourceLink
    */
    resourceTypes: string[];

    /**
     * List of supported interfaces
     * 
     * @type {string[]}
     * @memberOf ResourceLink
     */
    interfaces: string[];

    /**
     * Whether the resource is discoverable
     * 
     * @type {boolean}
     * @memberOf ResourceLink
     */
    discoverable: boolean;

    /**
     * Whether the resource is observable
     * 
     * @type {boolean}
     * @memberOf ResourceLink
     */
    observable: boolean;

    /**
     * Immutable instance identifier of a link
     * 
     * @type {string}
     * @memberOf ResourceLink
     */
    ins?: string;
}

/**
 * Object to create resources via Client
 * 
 * @export
 * @interface ResoureCreateObject
 */
export interface ResoureCreateObject {

    /**
     * OCF resource URI path
     * 
     * @type {string}
     * @memberOf ResoureCreateObject
     */
    resourcePath: string;

    /**
     * OCF resource types
     * TODO the spec says resourceType_ is string_
     * @type {string[]}
     * @memberOf ResoureCreateObject
     */
    resourceTypes: string[];
}

/**
 * Used for creating and registering resources,
 * exposes the properties of an OCF resource. All properties are read-write.
 * Either the {@link properties} or the {@link links} property MUST be defined.
 * The {@link interfaces} property MUST at least contain "oic.if.baseline".
 * @export
 * @interface Resource
 * @extends {ResourceId}
 */
export interface Resource extends ResourceId, ResoureCreateObject {

    /**
     * List of OCF resource types
     * 
     * @type {string[]}
     * @memberOf Resource
     */
    resourceTypes: string[];

    /**
     * List of supported interfaces
     * 
     * @type {string[]}
     * @memberOf Resource
     */
    interfaces: string[];

    /**
     * List of supported Internet media types
     * 
     * @type {string[]}
     * @memberOf Resource
     */
    mediaTypes: string[];
    /**
     * Whether the resource is discoverable
     * 
     * @type {boolean}
     * @memberOf Resource
     */
    discoverable: boolean;

    /**
     * Whether the resource is observable
     * 
     * @type {boolean}
     * @memberOf Resource
     */
    observable: boolean;

    /**
     * Collection of links
     * When present, it means the resource is an OCF Collection resource that contains
     * zero or more [RFC5988 Web Links]{@link https://tools.ietf.org/html/rfc5988}
     * represented by the {@link ResourceLink} dictionary.
     * @type {ResourceLink[]}
     * @memberOf Resource
     */
    links: ResourceLink[]

    /**
     * Whether the resource is secure
     * 
     * @type {boolean}
     * @memberOf Resource
     */
    secure: boolean;

    /**
     * Whether the resource is constrained
     * 
     * @type {boolean}
     * @memberOf Resource
     */
    slow?: boolean;

    /**
     * Resource representation properties as described in the data model.
     * Contains resource-specific properties and values
     * usually described in the RAML data model definition of the resource.
     * @type {*}
     * @memberOf Resource
     */
    properties?: any;

    /**
     * TODO Not in the spec???
     * True means that the resource is initialized, otherwise the resource is 'inactive'.
     * 'inactive' signifies that the resource has been marked for deletion or is already deleted.
     * @type {boolean}
     * @memberOf Resource
     */
    active?: boolean;
}

export interface ResourceRepresentation {
    //TODO is timestamp always in ResourceRepresentation?
}

export interface FindResourcesOptions {
    /**
     * OCF device UUID
     * 
     * @type {string}
     * @memberOf FindResourcesOptions
     */
    deviceId?: string;

    /**
     * OCF resource type
     * 
     * @type {string}
     * @memberOf FindResourcesOptions
     */
    resourceType?: string;

    /**
     * OCF resource path
     * 
     * @type {string}
     * @memberOf FindResourcesOptions
     */
    resourcePath?: string;

    /**
     * Polling period for discovery
     * 
     * @type {number}
     * @memberOf FindResourcesOptions
     */
    polling?: number;

    /**
     * Timeout period for discovery
     * 
     * @type {number}
     * @memberOf FindResourcesOptions
     */
    timeout?: number;
}

export var device: OcfDevice;
export var platform: OcfPlatform;

export const server: Server;
export const client: Client;


export interface RegisterResourceDictionary {
    resourcePath: string;
    resourceTypes: string[];
    interfaces: string[];
    mediaTypes: string[];
    discoverable: boolean;
    observable: boolean;
    secure: boolean;
    slow: boolean;
    properties?: {};
    links?: ResourceLink[];
}

/**
 * The Server API provides the means to register and unregister resources,
 * register handlers that serve CRUDN requests on a device,
 * notify of resource changes.
 * A device that implements the Server API may provide special resources to handle CRUDN requests.
 * The Server API object does not expose its own properties, only methods for registering handlers. 
 * @export
 * @class Server
 */
export class Server {

    /**
     * Registers a function to handle OCF create resource requests
     * 
     * @param {RequestHandler} handler
     * @returns {Server} this for chaining
     * 
     * @memberOf Server
     */
    oncreate(handler: RequestHandler): Server;

    /**
     * Registers a resource in the OCF network.
     * 
     * @param {RegisterResourceDictionary} init
     * @returns {Promise<ServerResource>}
     * 
     * @memberOf Server
     */
    register(init: RegisterResourceDictionary): Promise<ServerResource>;//todo
}

/**
 * Describes an object that is passed to server event listeners.
 * 
 * @export
 * @class OcfRequest
 */
export class OcfRequest {

    /**
     * TODO not part of the spec?
     * 
     * @type {*}
     * @memberOf OcfRequest
     */
    id?: any;// TODO


    /**
     * OCF request type.
     * Can be "create", "retrieve", "update", "delete"
     * @type {string}
     * @memberOf OcfRequest
     */
    type: string;

    /**
     * Requesting resource
     * 
     * @type {ResourceId}
     * @memberOf OcfRequest
     */
    source: ResourceId;

    /**
     * Request handling resource
     * 
     * @type {ResourceId}
     * @memberOf OcfRequest
     */
    target: ResourceId; // TODO
    data: ResourceId | Resource | ResourceRepresentation; //TODO

    /**
     * Object whose properties represent the REST query parameters
     * passed along with the request as a JSON-serializable dictionary.
     * The semantics of the parameters are application-specific
     * (e.g. requesting a resource representation in metric or imperial units).
     * For instance request options may be used with the retrieve request.
     * 
     * @type {*}
     * @memberOf OcfRequest
     */
    options?: any //TODO

    /**
     * Flag for OCF retrieve requests that tells whether observation
     * for the requested resource should be on or off. For
     * requests other than "retrieve" the value SHOULD be undefined.
     * 
     * @type {boolean}
     * @memberOf OcfRequest
     */
    observe: boolean;

    /**
     * Sends a response to this request.
     * The data argument is optional and used with requests such as create and update.
     * The method is typically used from request handlers, and internally
     * reuses the request information in order to construct a response message.
     * 
     * @param {(ResourceId | Resource | ResourceRepresentation)} [data]
     * @returns {Promise<void>}
     * 
     * @memberOf OcfRequest
     */
    respond(data?: ResourceId | Resource | ResourceRepresentation): Promise<void>;

    /**
     * Sends an error response to this request.
     * The error argument is an Error object.
     * The method is typically used from request handlers, and internally
     * reuses the request information in order to construct a response message.
     * 
     * @param {string} error
     * @returns {Promise<void>}
     * 
     * @memberOf OcfRequest
     */
    respondWithError(error: string): Promise<void>;
}

export type TranslateHandler = (requestOptions: any) => ResourceRepresentation; //TODO
export type RequestHandler = (request: OcfRequest) => void;

export class ServerResource implements Resource {

    /**
     * Registers a callback function handler for translation and returns this.
     * 
     * The handler function will be invoked by the implementation when the client
     * requests a certain representation of the resource by the means of request options.
     * The handler function will receive as argument a dictionary object options that contains
     * the REST request options parsed into property-value pairs.
     * The handler can use options and this.properties in order to compute
     * the modified resource representation object.
     * 
     * @param {TranslateHandler} handler
     * @returns {ServerResource}
     * 
     * @memberOf ServerResource
     */
    ontranslate(handler: TranslateHandler): ServerResource;

    /**
     * Registers the function handler to handle OCF retrieve resource requests
     * and returns this for chaining.
     * 
     * @param {RequestHandler} handler
     * @returns {ServerResource}
     * 
     * @memberOf ServerResource
     */
    onretrieve(handler: RequestHandler): ServerResource;

    /**
     * Registers the function handler to handle OCF update resource requests
     * and returns this for chaining.
     * 
     * @param {RequestHandler} handler
     * @returns {ServerResource}
     * 
     * @memberOf ServerResource
     */
    onupdate(handler: RequestHandler): ServerResource;

    /**
     * Registers the handler function to handle OCF delete resource requests
     * and returns this for chaining. 
     * 
     * @param {RequestHandler} handler
     * @returns {ServerResource}
     * 
     * @memberOf ServerResource
     */
    ondelete(handler: RequestHandler): ServerResource;

    /**
     * TODO not part of the spec?
     * 
     * @param {RequestHandler} handler
     * @returns {ServerResource}
     * 
     * @memberOf ServerResource
     */
    oncreate(handler: RequestHandler): ServerResource;

    /**
     * Notifies subscribed clients about a resource representation change.
     * Notifications are sent with the update event
     * @returns {Promise<void>}
     * 
     * @memberOf ServerResource
     */
    notify(): Promise<void>;

    /**
     * Unregisters the given resource from the OCF network.
     * 
     * @returns {Promise<void>}
     * 
     * @memberOf ServerResource
     */
    unregister(): Promise<void>; //TODO

    /**
     * UUID of the device
     * 
     * @type {string}
     * @memberOf ResourceId
     */
    deviceId: string;

    /**
     * URI path of the resource
     * 
     * @type {string}
     * @memberOf ResourceId
     */
    resourcePath: string;

    /**
     * List of OCF resource types
     * 
     * @type {string[]}
     * @memberOf Resource
     */
    resourceTypes: string[];

    /**
     * List of supported interfaces
     * 
     * @type {string[]}
     * @memberOf Resource
     */
    interfaces: string[];

    /**
     * List of supported Internet media types
     * 
     * @type {string[]}
     * @memberOf Resource
     */
    mediaTypes: string[];
    /**
     * Whether the resource is discoverable
     * 
     * @type {boolean}
     * @memberOf Resource
     */
    discoverable: boolean;

    /**
     * Whether the resource is observable
     * 
     * @type {boolean}
     * @memberOf Resource
     */
    observable: boolean;

    /**
     * Collection of links
     * When present, it means the resource is an OCF Collection resource that contains
     * zero or more [RFC5988 Web Links]{@link https://tools.ietf.org/html/rfc5988}
     * represented by the {@link ResourceLink} dictionary.
     * @type {ResourceLink[]}
     * @memberOf Resource
     */
    links: ResourceLink[]

    /**
     * Whether the resource is secure
     * 
     * @type {boolean}
     * @memberOf Resource
     */
    secure: boolean;

    /**
     * Whether the resource is constrained
     * 
     * @type {boolean}
     * @memberOf Resource
     */
    slow?: boolean;

    /**
     * Resource representation properties as described in the data model.
     * Contains resource-specific properties and values
     * usually described in the RAML data model definition of the resource.
     * @type {*}
     * @memberOf Resource
     */
    properties?: any;

    /**
     * TODO Not in the spec???
     * 
     * @type {boolean}
     * @memberOf Resource
     */
    active?: boolean;
}

/**
 * The OCF Client API implements CRUDN (Create, Retrieve, Update, Delete, Notify) functionality
 * that enables remote access to resources on the network, as well as OCF discovery.
 * 
 * @export
 * @class Client
 * @extends {EventEmitter}
 * @implements {IClientEvent}
 */
export class Client extends EventEmitter {

    /**
     * Fetches a remote platform information object. The {@link deviceId} argument is
     * a string that contains an OCF device UUID. 
     * 
     * @param {string} deviceId
     * @returns {Promise<{}>}
     * 
     * @memberOf Client
     */
    getPlatformInfo(deviceId: string): Promise<OcfPlatform>;

    /**
     * Fetches a remote device information object. The deviceId argument is a string that contains an OCF device UUID. 
     * deviceId could also be an object but this is undocumented
     * @param {string} deviceId
     * @returns {Promise<OcfDevice>}
     * 
     * @memberOf Client
     */
    getDeviceInfo(deviceId: string): Promise<OcfDevice>;

    /**
     * Initiates a platform discovery network operation.
     * When a platform is discovered, a {@link platformfound} event is fired that contains
     * a property named {@link platform}, whose value is a {@link OcfPlatform} object.
     * @param {PlatformHandler} [listener]
     * @returns {Promise<void>}
     * 
     * @memberOf Client
     */
    findPlatforms(listener?: PlatformHandler): Promise<void>;

    /**
     * Initiates a device discovery network operation.
     * When a device is discovered, a {@link devicefound} event is fired that contains
     * a property named {@link device}, whose value is a {@link OcfDevice} object.
     * @param {DeviceHandler} [listener]
     * @returns {Promise<void>}
     * 
     * @memberOf Client
     */
    findDevices(listener?: DeviceHandler): Promise<void>;

    /**
     * Whenever a resource resource is discovered, the {@link resourcefound} event is fired
     * with {@link resource} as an argument to the listener function.
     * 
     * @param {ClientResourceListener} [listener]
     * @returns {Promise<void>}
     * 
     * @memberOf Client
     */
    findResources(listener?: ClientResourceListener): Promise<void>;

    /**
     * Whenever a resource resource is discovered, the {@link resourcefound} event is fired
     * with {@link resource} as an argument to the listener function.
     * 
     * @param {FindResourcesOptions} [options]
     * @param {ClientResourceListener} [listener]
     * @returns {Promise<void>}
     * 
     * @memberOf Client
     */
    findResources(options?: FindResourcesOptions, listener?: ClientResourceListener): Promise<void>;

    /**
     * Creates a remote resource on a given device, and optionally specifies a {@link target} resource
     * that is supposed to create the new resource. The device's {@link oncreate} event handler
     * takes care of dispatching the request to the {@link target} resource that will handle
     * creating the resource, and responds with the created resource, or with an error.
     * @param {Resource} [resource] It should contain at least the resourcePath and resourceType properties
     * @param {ResourceId} [target] The optional target argument is a ResourceId object that contains at least a device UUID and a resource path that identifies the target resource responsible for creating the requested resource.
     * @returns {Promise<ClientResource>}
     * @memberOf Client
     */
    create(resource: ResoureCreateObject, target?: ResourceId): Promise<Resource>;
    retrieve(resourceId: Resource, listener?: ClientResourceListener): Promise<Resource>;
    retrieve(resourceId: Resource, query: any, listener?: ClientResourceListener): Promise<Resource>;

    /**
     * Updates a resource on the network by sending a request to the device specified by resource.deviceId.
     * The device's update event handler takes care of updating the resource and replying
     * with the updated resource, or with an error. The resource identified by resource is updated.
     * 
     * @param {Resource} resource
     * @returns {Promise<void>}
     * 
     * @memberOf Client
     */
    update(resource: Resource): Promise<void>;

    /**
     * Deletes a resource from the network by sending a request to the device specified
     * in resourceId.deviceId. The device's delete event handler takes care of
     * deleting (unregistering) the resource and reporting success or error.
     * 
     * @param {ResourceId} resourceId
     * @returns {Promise<any>}
     * 
     * @memberOf Client
     */
    delete(resourceId: ResourceId): Promise<void>;

    on(event: 'platformfound', listener: PlatformHandler): this;
    on(event: 'devicefound', listener: DeviceHandler): this;
    on(event: 'devicelost', listener: DeviceHandler): this;
    on(event: 'resourcefound', listener: ClientResourceListener): this;
}

export type DeviceHandler = (device: OcfDevice) => void;
export type PlatformHandler = (platform: OcfPlatform) => void;
export type ClientResourceListener = (resource: ClientResource) => void;

/**
 * Note that applications should not create {@link ClientResource} objects,
 * as they are created and tracked by implementations. Applications can create and use
 * {@link ResourceId} and {@link Resource} objects as method arguments,
 * but client-created {@link ClientResource} objects are not tracked by implementations
 * and will not receive events.
 * @export
 * @class ClientResource
 * @extends {EventEmitter}
 * @implements {IClientResourceEvent}
 * @implements {Resource}
 */
export class ClientResource extends EventEmitter implements Resource {

    /**
     * UUID of the device
     * 
     * @type {string}
     * @memberOf ResourceId
     */
    deviceId: string;

    /**
     * URI path of the resource
     * 
     * @type {string}
     * @memberOf ResourceId
     */
    resourcePath: string;
    /**
    * List of OCF resource types
    * 
    * @type {string[]}
    * @memberOf Resource
    */
    resourceTypes: string[];

    /**
     * List of supported interfaces
     * 
     * @type {string[]}
     * @memberOf Resource
     */
    interfaces: string[];

    /**
     * List of supported Internet media types
     * 
     * @type {string[]}
     * @memberOf Resource
     */
    mediaTypes: string[];
    /**
     * Whether the resource is discoverable
     * 
     * @type {boolean}
     * @memberOf Resource
     */
    discoverable: boolean;

    /**
     * Whether the resource is observable
     * 
     * @type {boolean}
     * @memberOf Resource
     */
    observable: boolean;

    /**
     * Collection of links
     * When present, it means the resource is an OCF Collection resource that contains
     * zero or more [RFC5988 Web Links]{@link https://tools.ietf.org/html/rfc5988}
     * represented by the {@link ResourceLink} dictionary.
     * @type {ResourceLink[]}
     * @memberOf Resource
     */
    links: ResourceLink[]

    /**
     * Whether the resource is secure
     * 
     * @type {boolean}
     * @memberOf Resource
     */
    secure: boolean;

    /**
     * Whether the resource is constrained
     * 
     * @type {boolean}
     * @memberOf Resource
     */
    slow?: boolean;

    /**
     * Resource representation properties as described in the data model.
     * Contains resource-specific properties and values
     * usually described in the RAML data model definition of the resource.
     * @type {*}
     * @memberOf Resource
     */
    properties?: any;

    /**
     * TODO Not in the spec???
     * 
     * @type {boolean}
     * @memberOf Resource
     */
    active?: boolean;

    /**
     * The {@link delete} event is fired on a {@link ClientResource} object when the implementation gets notified
     * about the resource being deleted or unregistered from the OCF network. The event listener receives
     * a {@link ResourceId} dictionary object that contains the {@link deviceId} and {@link resourcePath}
     * of the deleted resource.
     * 
     * @param {'delete'} event
     * @param {ClientResourceListener} listener
     * @returns {this}
     * 
     * @memberOf ClientResource
     */
    on(event: 'delete', listener: ClientResourceListener): this;

    /**
     * The update event is fired on a {@link ClientResource} object when the implementation
     * receives an OCF resource update notification because the resource representation has changed.
     * The event listener receives a dictionary object that contains the resource properties
     * that have changed.
     * In addition, the resource property values are already updated to the new values
     * when the event is fired.
     * 
     * The recommended way to observe and unobserve resources from applications is by using
     * the {@link retrieve()} method, in order to be able to specify OCF retrieve options.
     * However, for convenience, when the first listener function {@link listener} is added to
     * the {@link update} event of {@link resource}, implementations SHOULD call
     * `retrieve(resource, null, listener). When the last listener is removed,
     * the implementations SHOULD call retrieve(resource), i.e. make an OCF retrieve request
     * with the {@link observe} flag off.
     * TODO not sure how to model this because it may be partial and not sure whether it is Resource or ClientResource     * 
     * @param {'update'} event
     * @param {ClientResourceListener} listener
     * @returns {this}
     * 
     * @memberOf ClientResource
     */
    on(event: 'update', listener: ClientResourceListener): this;
}