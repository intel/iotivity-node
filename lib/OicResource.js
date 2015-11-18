var _ = require( "lodash" ),
	EventEmitter = require( "events" ).EventEmitter,

	// Create a version of listenerCount() that works in 0.12 as well
	listenerCount = function( emitter, event ) {
		return ( ( typeof emitter.listenerCount === "undefined" ) ?
			EventEmitter.listenerCount( emitter, event ) :
			emitter.listenerCount( event ) );
	},
	OicResource = function( init ) {
		if ( !this._isOicResource ) {
			return new OicResource( init );
		}

		if ( !( init.id && init.id.deviceId && init.id.path ) ) {
			throw new Error( "Constructing OicResource: malformed id" );
		}

		if ( init ) {
			_.extend( this,

				// The resource will have a "properties" key, even if it has no actual properties
				{ properties: {} },
				init );
		}

		this.on( "newListener", _.bind( function( event ) {
			var result,
				observationHandleReceptacle = {};

			if ( !( event === "update" && listenerCount( this, event ) === 0 ) ) {
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
					this.dispatchEvent( "update", {
						type: "update",
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

			if ( !( event === "update" && this._observationHandle &&
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
	};

require( "util" ).inherits( OicResource, EventEmitter );

_.extend( OicResource.prototype, {
	_isOicResource: true,
	onupdate: null,
	addEventListener: OicResource.prototype.addListener,

	removeEventListener: OicResource.prototype.removeListener,

	dispatchEvent: function( eventName, event ) {
		this.emit( eventName, event );
		if ( typeof this[ "on" + eventName ] === "function" ) {
			this[ "on" + eventName ]( event );
		}
	}
} );

module.exports = OicResource;
