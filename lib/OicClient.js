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

function doDiscovery( client, prefix, fulfill, reject, query, destination, responseHandler ) {
	var handleReceptacle = {},

		// Since we have no way of knowing when the discovery process ends, and since OCCancel()
		// does not handle the conclusion of discovery, we mark this handler as stale, and if it
		// ever gets called again, we'll tell the stack to delete this particular transaction. We
		// mark this handler as stale by setting the variable below to true.
		stale = false,
		timeoutId = null,

		// If @errorMessage is undefined then the discovery has timed out and thus needs to be
		// concluded. Otherwise, we need to clear the timeout and throw the error.
		discoveryFinished = function( errorMessage, result ) {
			stale = true;
			if ( errorMessage ) {
				if ( timeoutId ) {
					clearTimeout( timeoutId );
				}
				reject( _.extend( new Error( prefix + ": " + errorMessage + " failed" ), {
					result: result
				} ) );
			} else {
				fulfill();
			}
		},
		ret = iotivity.OCDoResource( handleReceptacle, iotivity.OCMethod.OC_REST_DISCOVER,
			query, destination, null,
			iotivity.OCConnectivityType.CT_DEFAULT,
			qosMap.known[ client._device._settings.connectionMode ] || qosMap.unknown,
			function( handle, response ) {
				if ( stale ) {
					return deleteTransaction;
				}

				if ( !response || response.result !== ocStackOk ) {
					discoveryFinished( "OCDoResource", response.result );
					return deleteTransaction;
				}

				responseHandler( response );

				// Push the timeout forward by 5 seconds
				if ( timeoutId ) {
					clearTimeout( timeoutId );
				}
				timeoutId = setTimeout( discoveryFinished, 5000 );

				return keepTransaction;
			},
			null,
			0 );

	if ( ret === ocStackOk ) {
		timeoutId = setTimeout( discoveryFinished, 5000 );
	} else {
		discoveryFinished( "OCDoResource", ret );
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

	removeEventListener: OicClient.prototype.removeEventListener,

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

	findResources: function( options ) {
		options = options || {};

		return new Promise( _.bind( function( fulfill, reject ) {

			// If a resource ID is given we do a retrieve and return
			if ( options.resourceId ) {
				this.retrieveResource( options.resourceId ).then(
					function( resource ) {
						this.dispatchEvent( "resourcefound",
							_.extend( new OicResourceFoundEvent(), { resource: resource } ) );
						fulfill();
					},
					function( error ) {
						reject( error );
					} );
				return;
			}

			doDiscovery( this, "findResources", fulfill, reject,

				// We request resources optionally filtered by type
				iotivity.OC_RSRVD_WELL_KNOWN_URI +
				( options.resourceType ? ( "?rt=" + options.resourceType ) : "" ),

				// If a destination is given use the corresponding address
				options.deviceId ? ( this._devices[ options.deviceId ].address || null ) : null,

				_.bind( function findResourcesHandler( response ) {
					var resource, i;
					var resources = response.payload.resources;
					var devAddr = response.devAddr;
					var resourceCount = resources.length;

					if ( resourceCount > 0 ) {
						var deviceId = sidToString( resources[ 0 ].sid );

						if ( !this._devices[ deviceId ] ) {
							this._devices[ deviceId ] = { address: devAddr };
						}

						for ( i = 0; i < resourceCount; i++ ) {
							resource = new OicResource( resources[ i ] );

							this._resources[ resource.id ] = resource;
							this.dispatchEvent( "resourcefound",
								_.extend( new OicResourceFoundEvent(), {
									resource: resource
								} ) );
						}
					}
				}, this ) );
		}, this ) );
	},

	createResource: function( init ) {
		return new Promise( _.bind( function( fulfill, reject ) {
			var devId = init.deviceId;

			//FIXME: Do we ahve to do this? What does init has?
			//var resource = this._resources[id];

			devId = null;

			//How to get devAddr from deviceId ?
			//devId = init.devAddr;

			var uri = init.url;
			var handle = {};
			init.type = iotivity.OCPayloadType.PAYLOAD_TYPE_REPRESENTATION;

			var ret = iotivity.OCDoResource( handle, iotivity.OCMethod.OC_REST_POST,
				uri, devId, init,
				iotivity.OCConnectivityType.CT_DEFAULT,
				qosMap.known[ this._device._settings.connectionMode ] || qosMap.unknown,
				function() {
					fulfill();
				},
				null, //FIXME: HeaderOptions?
				0 );

			if ( ret !== ocStackOk ) {
				reject( _.extend( new Error( "createResource: OCDoResource failed" ), {
					result: ret
				} ) );
				return;
			}

		}, this ) );
	},

	retrieveResource: function( id ) {
		return new Promise( _.bind( function( fulfill, reject ) {
			var resource = this._resources[ id ];
			var devAddr = this._devices[ resource.deviceId ].address;
			var handle = {};
			var result = iotivity.OCDoResource( handle, iotivity.OCMethod.OC_REST_GET,
				resource.uri, devAddr, null,
				iotivity.OCConnectivityType.CT_DEFAULT,
				qosMap.known[ this._device._settings.connectionMode ] || qosMap.unknown,
				function( handle, response ) {
					if ( response.payload ) {
						_.extend( resource.properties, response.payload.values );
						fulfill( resource );
					} else {
						reject( _.extend(
							new Error( "retrieveResource: response has no payload" ), {
								result: response.result
							} ) );
					}
					return deleteTransaction;
				},
				null, //FIXME: HeaderOptions?
				0 );

			if ( result !== ocStackOk ) {
				reject( _.extend( new Error( "retrieveResource: OCDoResource failed" ), {
					result: result
				} ) );
				return;
			}
		}, this ) );
	},

	updateResource: function( resource ) {
		return new Promise( _.bind( function( fulfill, reject ) {
			var devAddr = this._devices[ resource.deviceId ].address;

			var handle = {};

			var ret = iotivity.OCDoResource( handle, iotivity.OCMethod.OC_REST_PUT,
				resource.uri, devAddr, resource,
				iotivity.OCConnectivityType.CT_DEFAULT,
				qosMap.known[ this._device._settings.connectionMode ] || qosMap.unknown,
				function() {
					fulfill();
				},
				null, //FIXME: HeaderOptions?
				0 );

			if ( ret !== ocStackOk ) {
				reject( _.extend( new Error( "updateResource: OCDoResource failed" ), {
					result: ret
				} ) );
				return;
			}

		}, this ) );
	},

	deleteResource: function( id ) {
		return new Promise( _.bind( function( fulfill, reject ) {
			var resource = this._resources[ id ];

			var result = iotivity.OCDoResource( handle,
				iotivity.OCMethod.OC_REST_DELETE,
				resource.uri,
				this._devices[ resource.deviceId ].address, null,
				iotivity.OCConnectivityType.CT_DEFAULT,
				qosMap.known[ this._device._settings.connectionMode ] || qosMap.unknown,
				function( handle, response ) {
					fulfill();
					return deleteTransaction;
				},
				null,
				0 );

			if ( result !== ocStackOk ) {
				reject( _.extend( new Error( "retrieveResource: OCDoResource failed" ), {
					result: result
				} ) );
				return;
			}
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
					resource.uri,
					this._devices[ resource.deviceId ].address, null,
					iotivity.OCConnectivityType.CT_DEFAULT,
					qosMap.known[ this._device._settings.connectionMode ] || qosMap.unknown,
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
					qosMap.known[ this._device._settings.connectionMode ] || qosMap.unknown, null,
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
