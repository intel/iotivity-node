var iotivity = require( "bindings" )( "iotivity" ),
	_ = require( "underscore" ),
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

_.extend( OicClient.prototype, {
	_isOicClient: true,
	onresourcechange: null,
	onresourcefound: null,
	ondevicefound: null,
	_resources: {},
	_devices: {},
	_observationhandles: {},

	addEventListener: OicClient.prototype.addListener,

	removeEventListener: OicClient.prototype.removeEventListener,

	dispatchEvent: function( event, request ) {
		if ( typeof this[ "on" + event ] === "function" ) {
			this[ "on" + event ]( request );
		}
		this.emit( event, request );
	},

	findDevices: function( options ) {
		return new Promise( _.bind( function( fulfill, reject ) {
			var discoveryURI = iotivity.OC_RSRVD_DEVICE_URI;
			var waitsec = 5;
			var handle = {};
			var timeout;

			if ( options.waitsec ) {
				waitsec = options.waitsec;
			}

			var ret = iotivity.OCDoResource( handle, iotivity.OCMethod.OC_REST_DISCOVER,
				discoveryURI, null, null,
				iotivity.OCConnectivityType.CT_ADAPTER_IP,
				iotivity.OCQualityOfService[
					( this._device._settings.connectionMode === "acked" ?
						"OC_HIGH_QOS" : "OC_LOW_QOS" )
					],
					_.bind( function( handle, response ) {
						if ( !response || response === "" ||
								response.result !== iotivity.OCStackResult.OC_STACK_OK ) {
							reject( _.extend( new Error( "findDevices: OCDoResource failed" ), {
								result: response.result
							} ) );
							return iotivity.OCStackApplicationResult.OC_STACK_KEEP_TRANSACTION;
						}

						var event = new OicDeviceFoundEvent();
						event.device = response.payload;

						// Should we manage the device address somewhere?
						//this._devices[0] = response.devAddr;

						this.dispatchEvent( "devicefound", event );

						//FIXME: Should we fulfill or do that on timeout
						//fulfill();
						//clearTimeout( timeout );
						return iotivity.OCStackApplicationResult.OC_STACK_KEEP_TRANSACTION;
					}, this ),
					null,
					0 );

			if ( ret !== iotivity.OCStackResult.OC_STACK_OK ) {
				reject( _.extend( new Error( "findDevices: OCDoResource failed" ), {
					result: ret
				} ) );
				return;
			}
			timeout = setTimeout( function() {
				fulfill();
			}, waitsec * 1000 );
		}, this ) );
	},

	findResources: function( options ) {
		return new Promise( _.bind( function( fulfill, reject ) {
			var i;

			var discoveryURI = iotivity.OC_RSRVD_WELL_KNOWN_URI;
			var waitsec = 5;
			var handle = {};
			var timeout;

			options = options || {};

			if ( options.waitsec ) {
				waitsec = options.waitsec;
			}

			if ( options.deviceId && options.deviceId !== "" ) {

				//FIXME: See how to match SID Array DeviceID
			}

			if ( options.resourceId && options.resourceId !== "" ) {
				var resource = this._resources[ options.resourceId ];
				if ( !resource ) {
					reject( _.extend( new Error( "Unable to find resource" ), {
						result: iotivity
							.OCStackApplicationResult.OC_STACK_NO_RESOURCE
					} ) );
					return;
				}

				var event = new OicResourceFoundEvent();
				event.resource = resource;
				this.dispatchEvent( "resourcefound", event );
				return;
			}

			if ( options.resourceType ) {
				discoveryURI = discoveryURI + "?rt=" + options.resourceType;
			}

			var ret = iotivity.OCDoResource( handle, iotivity.OCMethod.OC_REST_DISCOVER,
				discoveryURI, null, null,
				iotivity.OCConnectivityType.CT_DEFAULT,
				iotivity.OCQualityOfService[
					( this._device._settings.connectionMode === "acked" ?
						"OC_HIGH_QOS" : "OC_LOW_QOS" )
					],
					_.bind( function( handle, response ) {
						if ( !response || response === "" ||
								response.result !== iotivity.OCStackResult.OC_STACK_OK ) {
							reject( _.extend( new Error( "FindResource: OCDoResource failed" ), {
								result: response.result
							} ) );
							return iotivity.OCStackApplicationResult.OC_STACK_KEEP_TRANSACTION;
						}

						var resources = response.payload.resources;
						var devAddr = response.devAddr;
						var resourceCount = resources.length;
						for ( i = 0; i < resourceCount; i++ ) {

							//FIXME: Filter by DeviceID if mentioned
							var resource = new OicResource( resources[ i ] );

							//FIXME Should we copy using _. ?
							// We should create a deviceID and have it referenced that way
							this._devices[ resource.id ] = devAddr;
							this._resources[ resource.id ] = resource;
							var event = new OicResourceFoundEvent();
							event.resource = resource;
							this.dispatchEvent( "resourcefound", event );
						}

						//Should we fulfill ?
						//fulfill();
						//clearTimeout( timeout );
						return iotivity.OCStackApplicationResult.OC_STACK_KEEP_TRANSACTION;
					}, this ),
					null,
					0 );

			if ( ret !== iotivity.OCStackResult.OC_STACK_OK ) {
				reject( _.extend( new Error( "FindResource: OCDoResource failed" ), {
					result: ret
				} ) );
				return;
			}

			timeout = setTimeout( function() {

				//FIXME Should we cancel?
				//iotivity.OCCancel(handle.handle,
				//	iotivity.OCQualityOfService.OC_HIGH_QOS, null, 0);
				fulfill();
			}, waitsec * 1000 );

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
				iotivity.OCQualityOfService[
					( this._device._settings.connectionMode === "acked" ?
						"OC_HIGH_QOS" : "OC_LOW_QOS" )
					],
					function() {
						fulfill();
					},
					null, //FIXME: HeaderOptions?
					0 );

			if ( ret !== iotivity.OCStackResult.OC_STACK_OK ) {
				reject( _.extend( new Error( "createResource: OCDoResource failed" ), {
					result: ret
				} ) );
				return;
			}

		}, this ) );
	},

	retrieveResource: function( id ) {
		return new Promise( _.bind( function( fulfill, reject ) {
			var devAddr = this._devices[ id ];
			var url = id.split( ":" )[ 1 ];

			//FIXME: Do we ahve to do this? What does init has?
			//var resource = this._resources[id];

			var handle = {};

			var ret = iotivity.OCDoResource( handle, iotivity.OCMethod.OC_REST_GET,
				url, devAddr, {
					type: iotivity.OCPayloadType.PAYLOAD_TYPE_REPRESENTATION
				}, // FIXME: Payload what else should it have?
				iotivity.OCConnectivityType.CT_DEFAULT,
				iotivity.OCQualityOfService[
					( this._device._settings.connectionMode === "acked" ?
						"OC_HIGH_QOS" : "OC_LOW_QOS" )
					],
					function( handle, response ) {
						fulfill( response.payload );
					},
					null, //FIXME: HeaderOptions?
					0 );

			if ( ret !== iotivity.OCStackResult.OC_STACK_OK ) {
				reject( _.extend( new Error( "retrieveResource: OCDoResource failed" ), {
					result: ret
				} ) );
				return;
			}
		}, this ) );
	},

	updateResource: function( resource ) {
		return new Promise( _.bind( function( fulfill, reject ) {
			var devAddr = this._devices[ resource.id ];
			var url = resource.id.split( ":" )[ 1 ];

			var handle = {};

			var ret = iotivity.OCDoResource( handle, iotivity.OCMethod.OC_REST_PUT,
				url, devAddr, resource,
				iotivity.OCConnectivityType.CT_DEFAULT,
				iotivity.OCQualityOfService[
					( this._device._settings.connectionMode === "acked" ?
						"OC_HIGH_QOS" : "OC_LOW_QOS" )
					],
					function() {
						fulfill();
					},
					null, //FIXME: HeaderOptions?
					0 );

			if ( ret !== iotivity.OCStackResult.OC_STACK_OK ) {
				reject( _.extend( new Error( "updateResource: OCDoResource failed" ), {
					result: ret
				} ) );
				return;
			}

		}, this ) );
	},

	startObserving: function( id ) {
		return new Promise( _.bind( function( fulfill, reject ) {
			var devAddr = this._devices[ id ];
			var url = this._resources[ id ].uri;

			//FIXME: Do we ahve to do this? What does init has?
			//var resource = this._resources[id];

			var handle = {};

			var ret = iotivity.OCDoResource( handle, iotivity.OCMethod.OC_REST_OBSERVE,
				url, devAddr, null, // FIXME: Payload what else should it have?
				iotivity.OCConnectivityType.CT_DEFAULT,
				iotivity.OCQualityOfService[
					( this._device._settings.connectionMode === "acked" ?
						"OC_HIGH_QOS" : "OC_LOW_QOS" )
					],
					_.bind( function( handle, response ) {
						if ( response.payload ) {
							var oneProperty,
								event = new OicResourceChangedEvent();

							event.resource = this._resources[ id ];
							if ( !event.resource.properties ) {
								event.resource.properties = {};
							}
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

							// FIXME: We should return a resource?
							fulfill( event.resource );
							this.dispatchEvent( "resourcechange", event );
						}

						if ( response.sequenceNumber ===
								iotivity.OCObserveAction.OC_OBSERVE_DEREGISTER ) {

							//Should we do this?
							this._observationhandles[ id ] = null;
							return iotivity.OCStackApplicationResult.OC_STACK_DELETE_TRANSACTION;
						}

						return iotivity.OCStackApplicationResult.OC_STACK_KEEP_TRANSACTION;
					}, this ),
					null, //FIXME: HeaderOptions?
					0 );

			if ( ret !== iotivity.OCStackResult.OC_STACK_OK ) {
				reject( _.extend( new Error( "retrieveResource: OCDoResource failed" ), {
					result: ret
				} ) );
				return;
			}

			this._observationhandles[ id ] = handle;
		}, this ) );
	},

	cancelObserving: function( id ) {
		return new Promise( _.bind( function( fulfill, reject ) {
			var handle = this._observationhandles[ id ];

			var ret = iotivity.OCCancel( handle.handle,
				iotivity.OCQualityOfService.OC_HIGH_QOS,
				null,
				0 );

			if ( ret !== iotivity.OCStackResult.OC_STACK_OK ) {
				reject( _.extend( new Error( "cancelObserving: OCCancel failed" ), {
					result: ret
				} ) );
				return;
			}

			this._observationhandles[ id ] = null;

			fulfill();

		}, this ) );
	}
} );

module.exports = OicClient;
