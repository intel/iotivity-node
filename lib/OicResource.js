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

var _ = require( "lodash" ),
	EventEmitter = require( "events" ).EventEmitter,

	// Create a version of listenerCount() that works in 0.12 as well
	listenerCount = function( emitter, event ) {
		return ( ( typeof emitter.listenerCount === "undefined" ) ?
			EventEmitter.listenerCount( emitter, event ) :
			emitter.listenerCount( event ) );
	},
	utils = require( "./utils" ),
	iotivity = require( "bindings" )( "iotivity" ),
	OicResource = function OicResource( init ) {
		if ( !this._isOicResource ) {
			return new OicResource( init );
		}

		// Will not create an object without a deviceId and path
		if ( !( init.id && init.id.deviceId && init.id.path ) ) {
			throw new Error( "Constructing OicResource: malformed id" );
		}

		utils.setPrivate( this, [
			"_isOicResource", "_events", "_eventsCount", "_address", "_observationHandle",
			"_handle"
		] );

		// Copy values from the initializer
		if ( init ) {
			_.extend( this,

				// The resource will have a "properties" key, even if it has no actual properties
				{ properties: {} },
				init );
		}

		this.on( "newListener", _.bind( function( event ) {
			var result,
				observationHandleReceptacle = {};

			if ( !( event === "change" && listenerCount( this, event ) === 0 ) ) {
				return;
			}

			// Start observing this resource
			result = iotivity.OCDoResource( observationHandleReceptacle,
				iotivity.OCMethod.OC_REST_OBSERVE,
				this.id.path,
				this._address, null,
				iotivity.OCConnectivityType.CT_DEFAULT,
				iotivity.OCQualityOfService.OC_HIGH_QOS,
				_.bind( function( handle, response ) {
					if ( response.payload ) {
						_.extend( this.properties,
							utils.payloadToObject( response.payload.values ) );
					}
					this.dispatchEvent( "change", {
						type: "change",
						resource: this
					} );
					return iotivity.OCStackApplicationResult.OC_STACK_KEEP_TRANSACTION;
				}, this ),
				null, 0 );

			if ( result === iotivity.OCStackResult.OC_STACK_OK ) {
				this._observationHandle = observationHandleReceptacle.handle;
			} else {
				throw _.extend( new Error(
					"OicResource: Failed to set up observation using OCDoResource" ), {
					result: result
				} );
			}
		}, this ) );
		this.on( "removeListener", _.bind( function( event ) {
			var result;

			if ( !( event === "change" && this._observationHandle &&
					listenerCount( this, event ) === 0 ) ) {
				return;
			}

			result = iotivity.OCCancel( this._observationHandle,
				iotivity.OCQualityOfService.OC_HIGH_QOS,
				null, 0 );

			if ( result === iotivity.OCStackResult.OC_STACK_OK ) {
				delete this._observationHandle;
			} else {
				throw _.extend( new Error(
					"OicResource: Failed to cancel observation using OCDoResource" ), {
					result: result
				} );
			}
		}, this ) );

		// Define property "onchange" such that writing to it will result in handlers being
		// added to/removed from the "change" event
		utils.addLegacyEventHandler( this, "change" );
	};

utils.makeEventEmitter( OicResource ).prototype._isOicResource = true;

module.exports = OicResource;
