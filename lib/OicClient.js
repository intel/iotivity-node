var iotivity = require( "bindings" )( "iotivity" ),
	_ = require( "lodash" ),
	utils = require( "./utils" ),

	// Shorten some references
	keepTransaction = iotivity.OCStackApplicationResult.OC_STACK_KEEP_TRANSACTION,
	deleteTransaction = iotivity.OCStackApplicationResult.OC_STACK_DELETE_TRANSACTION,
	ocStackOk = iotivity.OCStackResult.OC_STACK_OK,

	OicResource = require( "./OicResource" ),
	OicClient = function( device ) {
		if ( !this._isOicClient ) {
			return new OicClient( device );
		}
		this._device = device;
	},
	OicResourceFoundEvent = function() {
		if ( !this._isOicResourceFoundEvent ) {
			return new OicResourceFoundEvent();
		}
	},
	OicResourceChangedEvent = function() {
		if ( !this._isOicResourceChangedEvent ) {
			return new OicResourceChangedEvent();
		}
	},
	OicDeviceFoundEvent = function() {
		if ( !this._isOicDeviceFoundEvent ) {
			return new OicDeviceFoundEvent();
		}
	};

_.extend( OicResourceFoundEvent.prototype, {
	_isOicResourceFoundEvent: true
} );
_.extend( OicResourceChangedEvent.prototype, {
	_isOicResourceChangedEvent: true
} );
_.extend( OicDeviceFoundEvent.prototype, {
	_isOicDeviceFoundEvent: true
} );

require( "util" ).inherits( OicClient, require( "events" ).EventEmitter );

_.extend( OicClient.prototype, {
	_isOicClient: true,
	onresourcechange: null,
	onresourcefound: null,
	ondevicefound: null,
	_resources: {},

	// Map of deviceId: { info: OicDeviceInfo, address: OCDevAddr }
	_devices: {},

	_maybeAddResource: function( resource ) {
		var index, returnValue;

		// If we have already seen this resource, as identified by the deviceId and the url, then
		// we do not create a new resource, but return the existing resource instead.
		for ( index in this._resources ) {
			returnValue = this._resources[ index ];
			if ( returnValue.id && resource.id &&
					returnValue.id.deviceId === resource.id.deviceId &&
					returnValue.id.path === resource.id.path ) {
				return returnValue;
			}
		}

		// If no resource was found above, create a new one, add it to the list, and return it.
		returnValue = new OicResource( resource );
		this._resources[ returnValue.id.deviceId + ":" + returnValue.id.path ] = returnValue;

		return returnValue;
	},

	// Process resources from a discovery request. Either @reject() or @fulfill() is called every
	// time this function runs so the first response to a discovery request will determine if the
	// promise associated with that request is fulfilled or rejected. Examples:
	//
	// 1. OCDoResource()...
	//   ... first response: success (response.result === OC_STACK_OK) -> promise is fulfilled
	//   ... second response: failure (response.result !== OC_STACK_OK) -> reject() is called, but
	// since the the promise has already been fulfilled, it remains fulfilled, so this failure is
	// absorbed silently.
	//   ...
	//   ... subsequent responses: status continues to be absorbed silently.
	//
	// 2. OCDoResource()...
	//   ... first response: failure (response.result !== OC_STACK_OK) -> promise is rejected
	//   ... second response: success (response.result === OC_STACK_OK) -> fulfill() is called, but
	// since the the promise has already been rejected, it remains rejected, so this success is
	// absorbed silently.
	//   ...
	//   ... subsequent responses: status continues to be absorbed silently.
	//
	// The above explanation contains a small simplification: response.result !== OC_STACK_OK does
	// not cause rejection if response.result === OC_STACK_NO_RESOURCE because in the case of
	// discovery that also indicates success by way of the fact that a device on the network
	// reports that it has no resources.
	_processResources: function( handle, response, fulfill, reject ) {
		var i, resources;
		var resourceCount = 0;
		var devAddr = response.devAddr;
		var resolve = function( result, message ) {
			if ( arguments.length > 0 ) {
				reject( _.extend( new Error( "findResources: OCDoResource response failed" +
					( message ? ( ": " + message ) : "" ) ), {
					result: result
				} ) );
				return deleteTransaction;
			} else {
				fulfill();
				return keepTransaction;
			}
		};

		if ( !response ) {
			return resolve( undefined, "response missing" );
		}

		if ( response.result !== ocStackOk ) {
			return resolve.apply( this,
				( response.result === iotivity.OCStackResult.OC_STACK_NO_RESOURCE ) ?
					[] : [ response.result ] );
		}

		if ( !response.payload ) {
			resolve( response.result, "no payload" );
		}

		if ( !response.payload.resources ) {
			resolve( response.result, "unexpected absence of resources in payload" );
		}

		resources = response.payload.resources;
		resourceCount = resources.length;

		if ( resourceCount > 0 ) {
			var deviceId = utils.sidToString( resources[ 0 ].sid );

			if ( !this._devices[ deviceId ] ) {
				this._devices[ deviceId ] = { address: devAddr };
			}

			for ( i = 0; i < resourceCount; i++ ) {
				this.dispatchEvent( "resourcefound",
					_.extend( new OicResourceFoundEvent(), {
						resource: this._maybeAddResource( _.extend( resources[ i ], {
							id: { path: resources[ i ].uri, deviceId: deviceId }
						} ) )
					} ) );
			}
		}
		return resolve();
	},

	// Used for create/retrieve/update/delete
	_oneShotRequest: function( prefix, resource, restMethod, payload, createAnswer, handler ) {

		// Return a promise
		return new Promise( _.bind( function( fulfill, reject ) {
			var handleReceptacle = {},

				// Call OCDoResource() with the appropriate method
				result = iotivity.OCDoResource( handleReceptacle, iotivity.OCMethod[ restMethod ],
					resource.id.path, this._devices[ resource.id.deviceId ].address, payload,
					iotivity.OCConnectivityType.CT_DEFAULT,
					iotivity.OCQualityOfService.OC_HIGH_QOS,
					handler ?
						function( handle, response ) {
							return handler( handle, response, fulfill, reject );
						} :
						function oneShotRequestHandler( handle, response ) {
							var answer;

							if ( response.result === ocStackOk ) {

								// In case of success, @createAnswer() knows what to give fulfill()
								answer = createAnswer ? createAnswer( handle, response, reject ) :
									undefined;
								fulfill.apply( this, answer !== undefined ? [ answer ] : [] );
							} else {
								reject( _.extend(
									new Error( prefix + ": OCDoResource response failed" ), {
										result: response.result
									} ) );
							}

							return deleteTransaction;
						},
					null, 0 );

			if ( result !== ocStackOk ) {
				reject( _.extend( new Error( prefix + ": OCDoResource failed" ), {
					result: result
				} ) );
			}
		}, this ) );
	},

	_doDiscovery: function( prefix, fulfill, reject, query, destination, responseHandler ) {
		var result,
			handleReceptacle = {};

		// If we have a previous handle for a discovery of type @prefix that has not yet been
		// marked as stale by the bindings then cancel and discard it
		if ( this[ prefix ]._handle ) {
			if ( !this[ prefix ]._handle.stale ) {
				result = iotivity.OCCancel( this[ prefix ]._handle,
					iotivity.OCQualityOfService.OC_HIGH_QOS,
					null, 0 );
				if ( result !== iotivity.OCStackResult.OC_STACK_OK ) {
					reject(
						_.extend( new Error( prefix + ": failed to cancel previous handle" ), {
							result: result
						} ) );
					return;
				}
			}
			this[ prefix ]._handle.reject(
				_.extend( new Error( prefix + ": previous discovery aborted" ), {
					discoveryAborted: true
				} ) );
		}
		delete this[ prefix ]._handle;

		// Issue a new discovery request of type @prefix and store its handle
		result = iotivity.OCDoResource( handleReceptacle, iotivity.OCMethod.OC_REST_DISCOVER,
			query, destination, null,
			iotivity.OCConnectivityType.CT_DEFAULT,
			iotivity.OCQualityOfService.OC_HIGH_QOS,
			responseHandler,
			null,
			0 );

		if ( result === ocStackOk ) {
			this[ prefix ]._handle = handleReceptacle.handle;
			this[ prefix ]._handle.reject = reject;
		} else {
			reject( _.extend( new Error( prefix + ": " + "OCDoResource failed" ), {
				result: result
			} ) );
		}
	},

	addEventListener: OicClient.prototype.addListener,

	removeEventListener: OicClient.prototype.removeListener,

	dispatchEvent: function( event, request ) {
		if ( typeof this[ "on" + event ] === "function" ) {
			this[ "on" + event ]( request );
		}
		this.emit( event, request );
	},

	findDevices: function() {
		return new Promise( _.bind( function( fulfill, reject ) {
			this._doDiscovery( "findDevices", fulfill, reject,
				iotivity.OC_RSRVD_DEVICE_URI, null,
				_.bind( function findDevicesHandler( handle, response ) {
					var deviceId;

					if ( !response || response.result !== ocStackOk ) {
						reject( _.extend(
							new Error( "findDevices: OCDoResource response failed" ), {
								result: response.result
							} ) );
						return deleteTransaction;
					}

					if ( response.payload ) {
						deviceId = utils.sidToString( response.payload.sid );

						this._devices[ deviceId ] = {
							address: response.devAddr,
							info: {
								uuid: deviceId,
								name: response.payload.deviceName,
								coreSpecVersion: response.payload.specVersion
							}
						};

						this.dispatchEvent( "devicefound",
							_.extend( new OicDeviceFoundEvent(), {
								device: this._devices[ deviceId ].info
							} ) );
					}

					fulfill();
					return keepTransaction;
				}, this ) );
		}, this ) );
	},

	findResources: function( options ) {
		options = options || {};

		// We request resources optionally filtered by type
		var requestUrl = iotivity.OC_MULTICAST_DISCOVERY_URI +
			( options.resourceType ? ( "?rt=" + options.resourceType ) : "" );

		return options.resourceId ?
			this.retrieveResource( options.resourceId ).then(
				function( resource ) {
					this.dispatchEvent( "resourcefound",
						_.extend( new OicResourceFoundEvent(), { resource: resource } ) );
				} ) :
			options.deviceId ?
				this._oneShotRequest( "findResources", {
					url: requestUrl,
					deviceId: options.deviceId
				}, "OC_REST_DISCOVER", null, undefined,
					_.bind( function oneShotDiscoveryHandler( handle, response, fulfill, reject ) {
						this._processResources( handle, response, fulfill, reject );
						return deleteTransaction;
					}, this ) ) :
				new Promise( _.bind( function( fulfill, reject ) {

					this._doDiscovery( "findResources", fulfill, reject,

						// We request resources optionally filtered by type
						requestUrl,

						null,

						_.bind( function openEndedDiscoveryHandler( handle, response ) {
							return this._processResources( handle, response, fulfill, reject );
						}, this ) );
				}, this ) );
	},

	createResource: function( resource ) {
		return this._oneShotRequest( "createResource", resource, "OC_REST_POST",
			_.extend( utils.objectToPayload( resource.properties ), {
				types: resource.resourceTypes || [],
				interfaces: resource.interfaces || [],
				uri: resource.id.path
			} ),
			_.bind( function createResourceFromResponse() {
				return this._maybeAddResource( resource );
			}, this ) );
	},

	retrieveResource: function( id ) {
		var resource = this._resources[ id.deviceId + ":" + id.path ];

		return this._oneShotRequest( "retrieveResource", resource, "OC_REST_GET", null,
			function retrieveResourceFromResponse( handle, response ) {
				_.extend( resource.properties, utils.payloadToObject( response.payload.values ) );
				return resource;
			} );
	},

	updateResource: function( id, resource ) {
		id = id.deviceId + ":" + id.path;
		return this._oneShotRequest( "updateResource", this._resources[ id ], "OC_REST_PUT",
			utils.objectToPayload( resource.properties ),
			_.bind( function updateResourceFromResponse() {

				// We know that when the update proves successful the remote resource will have the
				// properties we sent to it
				_.extend( this._resources[ id ].properties, resource.properties );
			}, this ) );
	},

	deleteResource: function( id ) {
		id = id.deviceId + ":" + id.path;
		return this._oneShotRequest( "deleteResource", this._resources[ id ], "OC_REST_DELETE",
			null,
			_.bind( function deleteResourceFromResponse() {
				delete this._resources[ id ];
			}, this ) );
	},

	startObserving: function( id ) {
		return new Promise( _.bind( function( fulfill, reject ) {
			var ret,
				resource = this._resources[ id.deviceId + ":" + id.path ];

			if ( !resource._observationHandle ) {
				var handleReceptacle = {};
				ret = iotivity.OCDoResource( handleReceptacle,
					iotivity.OCMethod.OC_REST_OBSERVE,
					resource.id.path,
					this._devices[ resource.id.deviceId ].address, null,
					iotivity.OCConnectivityType.CT_DEFAULT,
					iotivity.OCQualityOfService.OC_HIGH_QOS,
					_.bind( function( handle, response ) {
						if ( response.payload ) {
							_.extend( resource.properties,
								utils.payloadToObject( response.payload.values ) );

							this.dispatchEvent( "resourcechange",
								_.extend( new OicResourceChangedEvent(), {
									resource: resource
								} ) );
						}

						return keepTransaction;
					}, this ),
					null, //FIXME: HeaderOptions?
					0 );
				if ( ret === ocStackOk ) {
					resource._observationHandle = handleReceptacle.handle;
				}
			}

			if ( resource._observationHandle ) {
				fulfill( resource );
			} else {
				reject( _.extend( new Error( "startObserving: OCDoResource failed" ), {
					result: ret
				} ) );
			}
		}, this ) );
	},

	cancelObserving: function( id ) {
		return new Promise( _.bind( function( fulfill, reject ) {
			var ret,
				resource = this._resources[ id.deviceId + ":" + id.path ];

			if ( resource._observationHandle ) {
				ret = iotivity.OCCancel( resource._observationHandle,
					iotivity.OCQualityOfService.OC_HIGH_QOS,
					null, 0 );
				if ( ret === ocStackOk ) {
					delete resource._observationHandle;
				}
			}

			if ( resource._observationHandle ) {
				reject( _.extend( new Error( "cancelObserving: OCCancel failed" ), {
					result: ret
				} ) );
			} else {
				fulfill();
			}
		}, this ) );
	}
} );

module.exports = OicClient;
