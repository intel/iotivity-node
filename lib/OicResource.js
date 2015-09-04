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
	// Is this needed?
	type: iotivity.OCPayloadType.PAYLOAD_TYPE_REPRESENTATION,
	_server: null,
} );

module.exports = {
	OicResource: OicResource
}