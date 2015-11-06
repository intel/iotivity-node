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
	};

_.extend( OicResource.prototype, {
	_isOicResource: true
} );

module.exports = OicResource;
