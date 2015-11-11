var iotivity = require( "bindings" )( "iotivity" ),
	_ = require( "lodash" ),
	utils = require( "./utils" ),

	// "acked" maps to OC_HIGH_QOS and anything else maps to OC_LOW_QOS
	qosMap = {
		"known": { "acked": iotivity.OCQualityOfService.OC_HIGH_QOS },
		"unknown": iotivity.OCQualityOfService.OC_LOW_QOS
	},

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
			if ( returnValue.deviceId === resource.deviceId &&
					returnValue.url === resource.url ) {
				return returnValue;
			}
		}

		if ( !resource.deviceId ) {
			throw new Error( "Cannot create resource without deviceId!" );
		}

		// If no resource was found above, create a new one, add it to the list, and return it.
		returnValue = new OicResource( resource );
		this._resources[ returnValue.id ] = returnValue;

		return returnValue;
	},

	_processFindResourceResponse: function( response ) {
		var i;
		var resources = response.payload.resources;
		var devAddr = response.devAddr;
		var resourceCount = resources.length;

		if ( resourceCount > 0 ) {
			var deviceId = utils.sidToString( resources[ 0 ].sid );

			if ( !this._devices[ deviceId ] ) {
				this._devices[ deviceId ] = { address: devAddr };
			}

			for ( i = 0; i < resourceCount; i++ ) {
				this.dispatchEvent( "resourcefound",
					_.extend( new OicResourceFoundEvent(), {
						resource: this._maybeAddResource( _.extend( resources[ i ], {

							// We use "url", not "uri"
							url: resources[ i ].uri,
							deviceId: deviceId
						} ) )
					} ) );
			}
		}
	},

	// Used for create/retrieve/update/delete
	_oneShotRequest: function( prefix, resource, restMethod, payload, createAnswer ) {

		// Return a promise
		return new Promise( _.bind( function( fulfill, reject ) {
			var handleReceptacle = {},

				// Call OCDoResource() with the appropriate method
				result = iotivity.OCDoResource( handleReceptacle, iotivity.OCMethod[ restMethod ],
					resource.url, this._devices[ resource.deviceId ].address, payload,
					iotivity.OCConnectivityType.CT_DEFAULT,
					qosMap.known[ this._device.settings.connectionMode ] || qosMap.unknown,
					function oneShotRequestHandler( handle, response ) {
						var answer;

						if ( response.result === ocStackOk ) {

							// In case of success, @createAnswer() knows what to give fulfill()
							answer = createAnswer ? createAnswer( response ) : undefined;
							fulfill.apply( this, answer !== undefined ? [ answer ] : [] );
						} else {
							reject(
								_.extend( new Error( prefix + ": OCDoResource response failed" ), {
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
				return;
			}
		}, this ) );
	},

	_doDiscovery: function( prefix, fulfill, reject, query, destination, responseHandler ) {
		var result,
			handleReceptacle = {};

		// If we have a previous handle for a discovery of type @prefix that has not yet been
		// marked as stale by the bindings then cancel and discard it
		if ( this[ prefix ]._handle && !this[ prefix ]._handle.stale ) {
			result = iotivity.OCCancel( this[ prefix ]._handle,
				qosMap.known[ this._device.settings.connectionMode ] || qosMap.unknown,
				null, 0 );
			if ( result !== iotivity.OCStackResult.OC_STACK_OK ) {
				reject(
					_.extend( new Error( prefix + ": " + "failed to cancel previous handle" ), {
						result: result
					} ) );
				return;
			}
		}
		delete this[ prefix ]._handle;

		// Issue a new discovery request of type @prefix and store its handle
		result = iotivity.OCDoResource( handleReceptacle, iotivity.OCMethod.OC_REST_DISCOVER,
			query, destination, null,
			iotivity.OCConnectivityType.CT_DEFAULT,
			qosMap.known[ this._device.settings.connectionMode ] || qosMap.unknown,
			function( handle, response ) {

				if ( !response || response.result !== ocStackOk ) {
					reject( _.extend( new Error( prefix + ": " + "OCDoResource failed" ), {
						result: response.result
					} ) );
					return deleteTransaction;
				}

				responseHandler( response );

				return keepTransaction;
			},
			null,
			0 );

		if ( result === ocStackOk ) {
			this[ prefix ]._handle = handleReceptacle.handle;
			fulfill();
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
				_.bind( function findDevicesHandler( response ) {
					var deviceId = utils.sidToString( response.payload.sid );

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
				}, "OC_REST_DISCOVER", null,
					_.bind( this._processFindResourceResponse, this ) ) :
				new Promise( _.bind( function( fulfill, reject ) {

					this._doDiscovery( "findResources", fulfill, reject,

						// We request resources optionally filtered by type
						requestUrl,

						null,

						_.bind( this._processFindResourceResponse, this ) );
				}, this ) );
	},

	createResource: function( resource ) {
		return this._oneShotRequest( "createResource", resource, "OC_REST_POST",
			_.extend( utils.objectToPayload( resource.properties ), {
				types: resource.resourceTypes || [],
				interfaces: resource.interfaces || [],
				uri: resource.url
			} ),
			_.bind( function createResourceFromResponse() {
				return this._maybeAddResource( resource );
			}, this ) );
	},

	retrieveResource: function( id ) {
		var resource = this._resources[ id ];

		return this._oneShotRequest( "retrieveResource", resource, "OC_REST_GET", null,
			function retrieveResourceFromResponse( response ) {
				_.extend( resource.properties, utils.payloadToObject( response.payload.values ) );
				return resource;
			} );
	},

	updateResource: function( id, resource ) {
		return this._oneShotRequest( "updateResource", this._resources[ id ], "OC_REST_PUT",
			utils.objectToPayload( resource.properties ),
			_.bind( function updateResourceFromResponse() {

				// We know that when the update proves successful the remote resource will have the
				// properties we sent to it
				_.extend( this._resources[ id ].properties, resource.properties );
			}, this ) );
	},

	deleteResource: function( id ) {
		return this._oneShotRequest( "deleteResource", this._resources[ id ], "OC_REST_DELETE",
			null,
			_.bind( function deleteResourceFromResponse() {
				delete this._resources[ id ]
				;
			}, this ) );
	},

	startObserving: function( id ) {
		return new Promise( _.bind( function( fulfill, reject ) {
			var ret,
				resource = this._resources[ id ];

			if ( !resource._observationHandle ) {
				var handleReceptacle = {};
				ret = iotivity.OCDoResource( handleReceptacle,
					iotivity.OCMethod.OC_REST_OBSERVE,
					resource.url,
					this._devices[ resource.deviceId ].address, null,
					iotivity.OCConnectivityType.CT_DEFAULT,
					qosMap.known[ this._device.settings.connectionMode ] || qosMap.unknown,
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
				resource = this._resources[ id ];

			if ( resource._observationHandle ) {
				ret = iotivity.OCCancel( resource._observationHandle,
					qosMap.known[ this._device.settings.connectionMode ] || qosMap.unknown, null,
					0 );
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
