// Copyright 2016 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

var iotivity = require( "bindings" )( "iotivity" ),
	_ = require( "lodash" ),
	utils = require( "./utils" ),
	querystring = require( "querystring" ),

	// Shorten some references
	keepTransaction = iotivity.OCStackApplicationResult.OC_STACK_KEEP_TRANSACTION,
	deleteTransaction = iotivity.OCStackApplicationResult.OC_STACK_DELETE_TRANSACTION,
	ocStackOk = iotivity.OCStackResult.OC_STACK_OK,

	OicResource = require( "./OicResource" );

module.exports = function( devicePrototype ) {

_.extend( devicePrototype, {
	_construct: ( function( _super ) {
		return function() {

			utils.setPrivate( this, [ "_handles", "_devices" ] );
			this._handles = {};

			// Map of deviceId: { info: OicDeviceInfo, address: OCDevAddr }
			this._devices = {};

			// Define legacy event handlers such that setting them will attach event handlers
			utils.addLegacyEventHandler( this, "resourcefound" );
			utils.addLegacyEventHandler( this, "devicefound" );
			utils.addLegacyEventHandler( this, "discoveryerror" );
			return _super.apply( this, arguments );
		};
	} )( devicePrototype._construct ),

	// Cancel any and all handles upon stopping the stack
	_stopStack: ( function( _super ) {
		return function() {
			var index;

			if ( this._info.device.role === "client" ) {
				for ( index in this._handles ) {
					if ( !this._handles[ index ].stale ) {
						iotivity.OCCancel( this._handles[ index ],
							iotivity.OCQualityOfService.OC_HIGH_QOS, null, 0 );
					}
					delete this._handles[ index ];
				}
			}

			return _super.apply( this, arguments );
		};
	} )( devicePrototype._stopStack ),

	_maybeAddResource: function( resource ) {

		// If we have already seen this resource, as identified by the deviceId and the path,
		// then we do not create a new resource, but return the existing resource instead.
		var returnValue = this._resources[ resource.id.deviceId + ":" + resource.id.path ];

		// If no resource was found above, create a new one, add it to the list, and return it.
		if ( !returnValue ) {
			returnValue = new OicResource( resource );
			this._resources[ returnValue.id.deviceId + ":" + returnValue.id.path ] =
				returnValue;
		}

		return returnValue;
	},

	_errorFromResponse: function( response, message ) {
		var payload,
			extension = {};

		if ( !response ) {
			message = message ? message : "missing response";
		} else {
			if ( response.result !== undefined ) {
				extension.result = response.result;
			}
		}

		if ( response.payload && response.payload.values ) {
			payload = utils.payloadToObject( response.payload.values );
			if ( payload.message ) {
				message = payload.message;
			}
			extension.properties = payload.properties;
		}

		return _.extend( new Error( message ), extension );
	},

	// Process resources from a discovery request. Either @reject() or @fulfill() is called
	// every time this function runs so the first response to a discovery request will
	// determine if the promise associated with that request is fulfilled or rejected.
	// Examples:
	//
	// 1. OCDoResource()...
	//   ... first response: success (response.result === OC_STACK_OK) -> promise is fulfilled
	//   ... second response: failure (response.result !== OC_STACK_OK) -> reject() is called,
	//       but since the the promise has already been fulfilled, it remains fulfilled, so
	//       this failure is absorbed silently.
	//   ...
	//   ... subsequent responses: status continues to be absorbed silently.
	//
	// 2. OCDoResource()...
	//   ... first response: failure (response.result !== OC_STACK_OK) -> promise is rejected
	//   ... second response: success (response.result === OC_STACK_OK) -> fulfill() is called,
	//       but since the the promise has already been rejected, it remains rejected, so this
	//       success is absorbed silently.
	//   ...
	//   ... subsequent responses: status continues to be absorbed silently.
	//
	// The above explanation contains a small simplification: response.result !== OC_STACK_OK
	// does not cause rejection if response.result === OC_STACK_NO_RESOURCE because in the case
	// of discovery that also indicates success by way of the fact that a device on the network
	// reports that it has no resources.
	_processResources: function( response, pathFilter ) {
		var i, resources;
		var newResources = [];
		var resourceCount = 0;
		var devAddr = response.devAddr;

		if ( !response ) {
			return this._errorFromResponse( undefined );
		}

		if ( response.result !== ocStackOk ) {
			return ( response.result === iotivity.OCStackResult.OC_STACK_NO_RESOURCE ) ?
				undefined : this._errorFromResponse( response );
		}

		if ( !response.payload ) {
			return this._errorFromResponse( response, "no payload" );
		}

		if ( !response.payload.resources ) {
			return this._errorFromResponse( response,
				"unexpected absence of resources in payload" );
		}

		resources = ( pathFilter ? _.filter( response.payload.resources, function( resource ) {
			return ( resource.uri === pathFilter );
		} ) : response.payload.resources );
		resourceCount = resources.length;

		if ( resourceCount > 0 ) {
			var deviceId = utils.sidToString( response.payload.sid );

			for ( i = 0; i < resourceCount; i++ ) {
				newResources.push( this._maybeAddResource( {
					id: { path: resources[ i ].uri, deviceId: deviceId },
					resourceTypes: resources[ i ].types,
					interfaces: resources[ i ].interfaces,
					_address: devAddr,
					secure: resources[ i ].secure,
					discoverable: !!( resources[ i ].bitmap &
						iotivity.OCResourceProperty.OC_DISCOVERABLE ),
					observable: !!( resources[ i ].bitmap &
						iotivity.OCResourceProperty.OC_OBSERVABLE ),
					slow: !!( resources[ i ].bitmap &
						iotivity.OCResourceProperty.OC_SLOW ),
					active: !!( resources[ i ].bitmap &
						iotivity.OCResourceProperty.OC_ACTIVE )
				} ) );
			}

			if ( !this._devices[ deviceId ] ) {
				this._addDevice( deviceId, { uuid: deviceId }, devAddr, newResources );
			}

			for ( i = 0; i < resourceCount; i++ ) {
				this.dispatchEvent( "resourcefound", {
					type: "resourcefound",
				resource: newResources[ i ] } );
			}
		}
	},

	// Used for create/retrieve/update/delete
	// options: {
	// 		prefix: string used as the key under which to store the resulting handle
	//      resourceId: hash - the resource ID,
	//      method: string used to retrieve the enum constant from iotivity
	//      payload: hash for the request payload
	//      createAnswer: function for creating the return value of the fulfill() method
	//          from the lower-level response
	//      handler: function for processing the response - used instead of createAnswer() if the
	//          response code (response.result) also needs to be checked
	// }
	_oneShotRequest: function( options ) {

		// Return a promise
		return new Promise( _.bind( function( fulfill, reject ) {
			if ( this._info.device.role === "server" ) {
				reject( new Error( "Not supported" ) );
				return;
			}

			if ( !options.resourceId ) {
				reject( new Error( options.prefix + ": resourceId undefined" ) );
			}

			if ( !this._devices[ options.resourceId.deviceId ] ) {
				reject( new Error( options.prefix + ": device not found" ) );
			}

			var handleReceptacle = {},

				// Call OCDoResource() with the appropriate method
				result = iotivity.OCDoResource( handleReceptacle,
					iotivity.OCMethod[ options.method ],
					options.resourceId.path,
					this._devices[ options.resourceId.deviceId ].address, options.payload || null,
					iotivity.OCConnectivityType.CT_DEFAULT,
					iotivity.OCQualityOfService.OC_HIGH_QOS,
					_.bind( options.handler ?
						function oneShotRequestCallCustomHandler( handle, response ) {
							return options.handler.call( this, handle, response, fulfill, reject );
						} :
						function oneShotRequestHandler( handle, response ) {
							var answer;

							if ( response.result === ocStackOk ) {

								// In case of success, @createAnswer() is responsible for
								// providing the value we pass to fulfill()
								answer = options.createAnswer ?
									options.createAnswer.call( this, handle, response ) :
									undefined;
								fulfill.apply( this, answer !== undefined ? [ answer ] : [] );
							} else {
								reject( this._errorFromResponse( response,
									options.prefix + ": OCDoResource response failed" ) );
							}

							return deleteTransaction;
						}, this ),
					null, 0 );

			if ( result !== ocStackOk ) {
				reject( this._errorFromResponse( { result: result },
					options.prefix + ": OCDoResource failed" ) );
			}
		}, this ) );
	},

	// _openEndedRequest(): Perform an open-ended request
	// @options recognizes the following keys:
	//	prefix: String: Where in this._handles to store the handle
	//	method: String: OCRestMethod key,
	//	query: String: request URL
	//	destination: Optional: OCDevAddr
	//	responseHandler: Function: Handler that returns OCStackApplicationResult (keep/delete)
	//	fulfill: Function: Optional: Function to call upon success
	//	reject: Function: Optional: Function to call upon failure
	_openEndedRequest: function( options ) {
		var result,
			handleReceptacle = {};

		// If we have a previous open-ended handle of type @options.prefix that has not yet
		// been marked as stale by the bindings then cancel and discard it
		if ( this._handles[ options.prefix ] ) {
			if ( !this._handles[ options.prefix ].stale ) {
				result = iotivity.OCCancel( this._handles[ options.prefix ],
					iotivity.OCQualityOfService.OC_HIGH_QOS,
					null, 0 );
				if ( result !== iotivity.OCStackResult.OC_STACK_OK && options.reject ) {
					options.reject(
						_.extend( new Error(
							options.prefix + ": failed to cancel previous handle" ), {
							result: result
						} ) );
					return;
				}
			}
		}
		delete this._handles[ options.prefix ];

		// Issue a new request and store its handle
		result = iotivity.OCDoResource( handleReceptacle, iotivity.OCMethod[ options.method ],
			options.query, options.destination, null,
			iotivity.OCConnectivityType.CT_DEFAULT,
			iotivity.OCQualityOfService.OC_HIGH_QOS,
			options.responseHandler,
			null,
			0 );

		if ( result === ocStackOk ) {
			this._handles[ options.prefix ] = handleReceptacle.handle;
			if ( options.fulfill ) {
				options.fulfill();
			}
		} else if ( options.reject ) {
			options.reject(
				_.extend( new Error( options.prefix + ": " + "OCDoResource failed" ), {
					result: result
				} ) );
		}

		return result;
	},

	findDevices: function() {
		return new Promise( _.bind( function( fulfill, reject ) {
			if ( this._info.device.role === "server" ) {
				reject( new Error( "Not supported" ) );
				return;
			}
			this._openEndedRequest( {
				fulfill: fulfill,
				reject: reject,
				prefix: "findDevices",
				query: iotivity.OC_RSRVD_DEVICE_URI,
				method: "OC_REST_DISCOVER",
				destination: null,
				responseHandler: _.bind( function findDevicesHandler( handle, response ) {
					var deviceId = ( response && response.payload && response.payload.sid ) ?
						utils.sidToString( response.payload.sid ) : "unknown";

					if ( !response || response.result !== ocStackOk ) {
						this.dispatchEvent( "discoveryerror", _.extend( {
							type: "discoveryerror",
							error: this._errorFromResponse( response,
								"findDevices: OCDoResource response failed" )
						}, ( deviceId ? { deviceId: deviceId } : {} ) ) );
						return deleteTransaction;
					}

					if ( response.payload ) {
						this._addDevice( deviceId, {
							uuid: deviceId,
							name: response.payload.deviceName,
							coreSpecVersion: response.payload.specVersion
						}, response.devAddr );
					}

					fulfill();
					return keepTransaction;
				}, this )
			} );
		}, this ) );
	},

	_addDevice: function( deviceId, info, address, resources ) {
		var existingDevice = this._devices[ deviceId ];

		// If we are already aware of this device, update its properties. Otherwise, create a
		// new device.
		this._devices[ deviceId ] = _.extend(
			( existingDevice ? existingDevice : {} ),
			{ address: address },
			( info ? { info: info } : {} ) );

		// Add presence notifications to the newly added device
		this._openEndedRequest( {
			prefix: deviceId,
			method: "OC_REST_PRESENCE",
			query: iotivity.OC_RSRVD_PRESENCE_URI,
			destination: address,
			responseHandler: _.bind( function( handle, response ) {
				var index;

				if ( response && response.payload && response.payload.type ===
						iotivity.OCPayloadType.PAYLOAD_TYPE_PRESENCE ) {
					switch ( response.payload.trigger ) {

					case iotivity.OCPresenceTrigger.OC_PRESENCE_TRIGGER_CREATE:
						this._addDevice( deviceId, { uuid: deviceId }, response.devAddr );
						this.findResources( { deviceId: deviceId } );
						break;

					case iotivity.OCPresenceTrigger.OC_PRESENCE_TRIGGER_DELETE:
						delete this._devices[ deviceId ];
						for ( index in this._resources ) {
							if ( this._resources[ index ].id.deviceId === deviceId ) {
								this._resources[ index ].dispatchEvent( "delete", {
									type: "delete"
								} );
								delete this._resources[ index ];
							}
						}
						break;

					default:

						// FIXME: What is OC_PRESENCE_TRIGGER_CHANGE?
						break;
					}
				}

				return iotivity.OCStackApplicationResult.OC_STACK_KEEP_TRANSACTION;
			}, this )
		} );

		this.dispatchEvent( "devicefound", _.extend( {
			type: "devicefound",
			device: info
		}, ( resources ? { resources: resources } : {} ) ) );
	},

	getDeviceInfo: function( id ) {
		return this._oneShotRequest( {
			prefix: "getDeviceInfo",
			resourceId: { deviceId: id, path: "/oic/d" },
			method: "OC_REST_GET",
			createAnswer: function createAnswerForGetDeviceInfo( handle, response ) {
				this._devices[ id ] = _.extend( this._devices[ id ] || {}, {
					info: {
						uuid: id,
						name: response.payload.deviceName,
						coreSpecVersion: response.payload.specVersion
					}
				} );
				return this._devices[ id ].info;
			}
		} );
	},

	getPlatformInfo: function( id ) {
		return this._oneShotRequest( {
			prefix: "getPlatformInfo",
			resourceId: { deviceId: id, path: "/oic/p" },
			method: "OC_REST_GET",
			createAnswer: function createAnswerForGetPlatformInfo( handle, response ) {
				return {
					id: response.payload.info.platformID,
					osVersion: response.payload.info.operatingSystemVersion,
					model: response.payload.info.modelNumber,
					manufacturerName: response.payload.info.manufacturerName,
					manufacturerUrl: response.payload.info.manufacturerUrl,
					manufactureDate: new Date( response.payload.info.dateOfManufacture ),
					platformVersion: response.payload.info.platformVersion,
					firmwareVersion: response.payload.info.firmwareVersion,
					supportUrl: response.payload.info.supportUrl
				};
			}
		} );
	},

	findResources: function( options ) {
		options = options || {};

		// We request resources optionally filtered by type
		var requestUrl = iotivity.OC_MULTICAST_DISCOVERY_URI +
			( options.resourceType ? ( "?rt=" + options.resourceType ) : "" );

		return options.deviceId ?
			this._oneShotRequest( {
				prefix: "findResources",
				resourceId: { path: requestUrl, deviceId: options.deviceId },
				method: "OC_REST_DISCOVER",
				handler: function handleOneShotDiscovery( handle, response, fulfill, reject ) {
					var error = this._processResources( response, options.resourcePath );
					if ( error ) {
						reject( error );
					} else {
						fulfill();
					}
					return deleteTransaction;
				}
			} ) :
			new Promise( _.bind( function( fulfill, reject ) {
				if ( this._info.device.role === "server" ) {
					reject( new Error( "Not supported" ) );
					return;
				}

				this._openEndedRequest( {
					prefix: "findResources",

					method: "OC_REST_DISCOVER",

					// We request resources optionally filtered by type
					query: requestUrl,

					destination: null,

					responseHandler: _.bind( function openEndedDiscovery( handle, response ) {
						var error = this._processResources( response, options.resourcePath );
						if ( error ) {
							this.dispatchEvent( "discoveryerror", {
								type: "discoveryerror",
								error: error
							} );
						}
						return keepTransaction;
					}, this ),
					fulfill: fulfill,
					reject: reject
				} );
			}, this ) );
	},

	create: function( resource ) {
		return this._oneShotRequest( {
			prefix: "create",
			resourceId: resource.id,
			method: "OC_REST_POST",
			payload: _.extend( utils.objectToPayload( resource.properties ), {
				types: resource.resourceTypes || [],
				interfaces: resource.interfaces || [],
				uri: resource.id.path
			} ),
			createAnswer: function createResourceFromResponse() {
				return this._maybeAddResource( resource );
			}
		} );
	},

	retrieve: function( id, queryParameters ) {
		return this._oneShotRequest( {
			prefix: "retrieve",
			resourceId: _.extend( {}, id, queryParameters ? {
				path: id.path + "?" + querystring.stringify( queryParameters )
			} : {} ),
			method: "OC_REST_GET",
			createAnswer: function retrieveResourceFromResponse( handle, response ) {
				var resource = this._resources[ id.deviceId + ":" + id.path ];
				_.extend( resource.properties,
					utils.payloadToObject( response.payload.values ) );
				return resource;
			}
		} );
	},

	update: function( resource ) {
		return this._oneShotRequest( {
			prefix: "update",
			resourceId: resource.id,
			method: "OC_REST_PUT",
			payload: utils.objectToPayload( resource.properties )
		} );
	},

	delete: function( id ) {
		return this._oneShotRequest( {
			prefix: "delete",
			resourceId: id,
			method: "OC_REST_DELETE",
			createAnswer: function deleteResourceFromResponse() {
				delete this._resources[ id.deviceId + ":" + id.path ];
			}
		} );
	}
} );

};
