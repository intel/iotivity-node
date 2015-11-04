var sidToString = require( "./utils" ).sidToString,
	_ = require( "lodash" ),
	OicResource = function( init ) {
		if ( !this._isOicResource ) {
			return new OicResource( init );
		}

		if ( init ) {
			_.extend( this,

				// The resource will have a "properties" key, even if it has no actual properties
				{ properties: {} },
				init );
		}

		// Create a new UUID for this resource
		this.id = sidToString();

		// Convert the device ID to a string
		this.deviceId = sidToString( init.sid );
	};

_.extend( OicResource.prototype, {
	_isOicResource: true
} );

module.exports = OicResource;
