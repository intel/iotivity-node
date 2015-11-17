var _ = require( "lodash" ),
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
	};

_.extend( OicResource.prototype, {
	_isOicResource: true
} );

module.exports = OicResource;
