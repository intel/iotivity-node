var iotivity = require( "bindings" )( "iotivity" ),
	_ = require( "lodash" ),
	sidToString = require( "./utils" ).sidToString,

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
	_isOicResourceChangedEvent: true,
	updatedPropertyNames: []
} );
_.extend( OicDeviceFoundEvent.prototype, {
	_isOicDeviceFoundEvent: true
} );

require( "util" ).inherits( OicClient, require( "events" ).EventEmitter );

// Used for create/retrieve/update/delete
function oneShotRequest( client, prefix, resource, restMethod, payload, createAnswer ) {

	// Return a promise
	return new Promise( function( fulfill, reject ) {
		var handleReceptacle = {},

			// Call OCDoResource() with the appropriate method
			result = iotivity.OCDoResource( handleReceptacle, iotivity.OCMethod[ restMethod ],
				resource.url, client._devices[ resource.deviceId ].address, payload,
				iotivity.OCConnectivityType.CT_DEFAULT,
				qosMap.known[ client._device.settings.connectionMode ] || qosMap.unknown,
				function oneShotRequestHandler( handle, response ) {
					var answer;

					if ( response.result === ocStackOk ) {

						// In case of success, @createAnswer() knows what to give fulfill()
						answer = createAnswer ? createAnswer( response ) : undefined;
						fulfill.apply( this, answer !== undefined ? [ answer ] : [] );
					} else {
						reject( _.extend( new Error( prefix + ": OCDoResource failed" ), {
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
	} );
}

function doDiscovery( client, prefix, fulfill, reject, query, destination, responseHandler ) {
	var result,
		handleReceptacle = {};

	// If we have a previous handle for a discovery of type @prefix, cancel and discard it
	if ( client[ prefix ]._handle ) {
		result = iotivity.OCCancel( client[ prefix ]._handle,
			qosMap.known[ client._device.settings.connectionMode ] || qosMap.unknown,
			null, 0 );
		if ( result !== iotivity.OCStackResult.OC_STACK_OK ) {
			reject( _.extend( new Error( prefix + ": " + "failed to cancel previous handle" ), {
				result: result
			} ) );
			return;
		}
	}
	delete client[ prefix ]._handle;

	// Issue a new discovery request of type @prefix and store its handle
	result = iotivity.OCDoResource( handleReceptacle, iotivity.OCMethod.OC_REST_DISCOVER,
		query, destination, null,
		iotivity.OCConnectivityType.CT_DEFAULT,
		qosMap.known[ client._device.settings.connectionMode ] || qosMap.unknown,
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
		client[ prefix ]._handle = handleReceptacle.handle;
		fulfill();
	} else {
		reject( _.extend( new Error( prefix + ": " + "OCDoResource failed" ), {
			result: result
		} ) );
	}
}

_.extend( OicClient.prototype, {
	_isOicClient: true,
	onresourcechange: null,
	onresourcefound: null,
	ondevicefound: null,
	_resources: {},

	// Map of deviceId: { info: OicDeviceInfo, address: OCDevAddr }
	_devices: {},

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
			doDiscovery( this, "findDevices", fulfill, reject,
				iotivity.OC_RSRVD_DEVICE_URI, null,
				_.bind( function findDevicesHandler( response ) {
					var deviceId = sidToString( response.payload.sid );

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

	_processFindResourceResponse: function( response ) {
		var i;
		var resources = response.payload.resources;
		var devAddr = response.devAddr;
		var resourceCount = resources.length;

		if ( resourceCount > 0 ) {
			var deviceId = sidToString( resources[ 0 ].sid );

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

		// If no resource was found above, create a new one, add it to the list, and return it.
		returnValue = new OicResource( resource );
		this._resources[ returnValue.id ] = returnValue;

		return returnValue;
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
				oneShotRequest( this, "findResources", {
						url: requestUrl,
						deviceId: options.deviceId
					}, "OC_REST_DISCOVER", null,
					_.bind( this._processFindResourceResponse, this ) ) :
			new Promise( _.bind( function( fulfill, reject ) {

				doDiscovery( this, "findResources", fulfill, reject,

					// We request resources optionally filtered by type
					requestUrl,

					null,

					_.bind( this._processFindResourceResponse, this ) );
			}, this ) );
	},

	createResource: function( resource ) {
		return oneShotRequest( this, "createResource", resource, "OC_REST_POST", {
			type: iotivity.OCPayloadType.PAYLOAD_TYPE_REPRESENTATION,
			values: resource.properties
		}, _.bind( function createResourceFromResponse() {
			return this._maybeAddResource( resource );
		}, this ) );
	},

	retrieveResource: function( id ) {
		var resource = this._resources[ id ];

		return oneShotRequest( this, "retrieveResource", resource, "OC_REST_GET", null,
			function retrieveResourceFromResponse( response ) {
				_.extend( resource.properties, response.payload.values );
				return resource;
			} );
	},

	updateResource: function( id, resource ) {
		return oneShotRequest( this, "updateResource", this._resources[ id ], "OC_REST_PUT", {
			type: iotivity.OCPayloadType.PAYLOAD_TYPE_REPRESENTATION,
			values: resource.properties
		}, function updateResourceFromResponse() {

			// We know that when the update proves successful the remote resource will have the
			// properties we sent to it
			_.extend( this._resources[ id ].properties, resource.properties );
		} );
	},

	deleteResource: function( id ) {
		return oneShotRequest( this, "deleteResource", this._resources[ id ], "OC_REST_DELETE",
			null,
			function deleteResourceFromResponse() {
				delete this._resources[ id ];
			} );
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
							var oneProperty,
								event = new OicResourceChangedEvent();

							event.resource = resource;
							if ( response.payload.values ) {
								for ( oneProperty in response.payload.values ) {
									if ( response.payload.values[ oneProperty ] !==
											event.resource.properties[ oneProperty ] ) {
										event.resource.properties[ oneProperty ] =
											response.payload.values[ oneProperty ];
										event.updatedPropertyNames.push( oneProperty );
									}
								}
							}

							this.dispatchEvent( "resourcechange", event );
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
