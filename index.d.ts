// Type definitions for iotivity-node 1.2.0-2
// Project: iotivity-node
// Definitions by: Hans Bakker https://www.github.com/wind-rider

/// <reference types="node" />

import { EventEmitter } from "events";

/*~ If this module has methods, declare them as functions like so.
 */
//export function myMethod(a: string): string;
//export function myOtherMethod(a: number): number;

/*~ You can declare types that are available via importing the module */
export interface IPlatformInfo {
    id: string;
    osVersion: string;
    model: string;
    manufacturerName: string;
    manufacturerURL: string;
    manufacturerDate: Date;
    platformVersion: string;
    firmwareVersion: string;
    supportURL: string;
}

export interface IDeviceInfo {
    url: string;
    name: string;
    dataModels: string[];
    coreSpecVersion: string;
    uuid: string;
}

/*~ You can declare properties of the module using const, let, or var */
//export const myField: number;

export var device: IDeviceInfo;
export var platform: IPlatformInfo;

/*~ If there are types, properties, or methods inside dotted names
 *~ of the module, declare them inside a 'namespace'.
 */
//export namespace server {
/*~ For example, given this definition, someone could write:
 *~   import { subProp } from 'yourModule';
 *~   subProp.foo();
 *~ or
 *~   import * as yourMod from 'yourModule';
 *~   yourMod.subProp.foo();
 */
//    export function foo(): void;
//}

export const server: Server;
export const client: Client;

export class Server {
    constructor();

    oncreate(handler: RequestHandler): Server;
    register(init: IResourceInfo): Promise<ServerResource> |any;//todo
}

export class Request {
    constructor(init?: Request);

    id?: any;// TODO
    observe: boolean;
    target?: ServerResource; // TODO
    type: string;
    data?: ServerResource|any; //TODO

    respond(data?: ServerResource | any): Promise<{}>; //TODO
    respondWithError(error: string): Promise<{}>;
}

export type RequestHandler = (request: Request) => void;

export class ServerResource implements IResourceInfo {
    constructor();

    deviceId: string;
    slow: boolean;
    active: boolean;
    secure: boolean;

    ontranslate(handler: RequestHandler);
    onretrieve(handler: RequestHandler);
    onupdate(handler: RequestHandler);
    ondelete(handler: RequestHandler);
    oncreate(handler: RequestHandler);

    unregister(): Promise<void>; //TODO
    notify(): Promise<void>;

    resourcePath: string;
    resourceTypes: string[];
    interfaces: string[];
    discoverable: boolean;
    observable: boolean;
    properties: any;
}

export interface IResourceInfo {
    deviceId?: string;
    resourcePath: string;
    resourceTypes: string[];
    interfaces: string[];
    discoverable?: boolean;
    observable?: boolean;
    properties: any;
    slow?: boolean;
    active?: boolean;
    secure?: boolean;
}

export class Client extends EventEmitter implements IClientEvent {
    // deviceId could also be an object but this is undocumented
    getDeviceInfo(deviceId: string): Promise<{}>;
    getPlatformInfo(deviceId: string): Promise<{}>;

    //TODO
    create(resourceInit?: IResourceInfo, target?: any): Promise<ClientResource>;
    retrieve(resourceId: IResourceInfo, listener?: ClientResourceListener): Promise<ClientResource>;
    retrieve(resourceId: IResourceInfo, query: any, listener?: ClientResourceListener): Promise<ClientResource>;
    update(resource: ClientResource): Promise<ClientResource>;
    delete(resourceId: IResourceInfo): Promise<any>;

    findDevices(listener?: DeviceHandler): Promise<void>;
    findPlatforms(listener?: PlatformHandler): Promise<void>;
    findResources(listener?: ClientResourceListener): Promise<void>;
    findResources(options? : any, listener?: ClientResourceListener): Promise<void>;

    on(event: 'devicefound', listener: DeviceHandler): this;
    on(event: 'platformfound', listener: PlatformHandler): this;
    on(event: 'resourcefound', listener: ClientResourceListener): this;
    on(event: 'error', listener: (error: Error) => void): this;
    on(event: string | symbol, listener: Function): this;
}

export type DeviceHandler = (device: IDeviceInfo) => void;
export type PlatformHandler = (platform: IPlatformInfo) => void;
export type ClientResourceListener = (resource: ClientResource) => void;

export interface IClientEvent {
    on(event: 'devicefound', listener: DeviceHandler): this;
    on(event: 'platformfound', listener: PlatformHandler): this;
    on(event: 'resourcefound', listener: ClientResourceListener): this;
    on(event: 'error', listener: (error: Error) => void): this;
    on(event: string | symbol, listener: Function): this;
}

export class ClientResource extends EventEmitter implements IClientResourceEvent, IResourceInfo {
    constructor(init, forceNew);

    resourcePath: string;
    resourceTypes: string[];
    interfaces: string[];
    discoverable: boolean;
    observable: boolean;
    properties: any;

    deviceId: string;
    slow: boolean;
    active: boolean;
    secure: boolean;

    on(event: 'delete', listener: ClientResourceListener): this;
    on(event: 'update', listener: ClientResourceListener): this;
    on(event: 'error', listener: (error: Error) => void): this;
    on(event: string, listener: Function): this;
}

export interface IClientResourceEvent {
    on(event: 'delete', listener: ClientResourceListener): this;
    on(event: 'update', listener: ClientResourceListener): this;
    on(event: 'error', listener: (error: Error) => void): this;
    on(event: string, listener: Function): this;
}