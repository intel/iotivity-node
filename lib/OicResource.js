var iotivity = require( "bindings" )( "iotivity" ),
	_ = require( "underscore" ),
	OicResource = function( init ) {
		if ( !this._isOicResource ) {
			return new OicResource( init );
		}

		if ( init )
			_.extend( this, init );

	};

_.extend( OicResource.prototype, {
	_isOicResource: true,
	_server: null,
} );

module.exports = {
	OicResource: OicResource
}