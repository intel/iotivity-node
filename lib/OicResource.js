var iotivity = require( "bindings" )( "iotivity" ),
	createUuid = function( source ) {
		var result,
			idReceptacle = {};

		// If we get an sid convert it to a string
		if ( source ) {
			result = iotivity.OCConvertUuidToString( source, idReceptacle );
			if ( result === iotivity.OCRandomUuidResult.RAND_UUID_OK ) {
				return idReceptacle.sid;
			}
		}

		// Otherwise generate a Uuid string
		result = iotivity.OCGenerateUuidString( idReceptacle );
		if ( result === iotivity.OCRandomUuidResult.RAND_UUID_OK ) {
			return idReceptacle.sid;
		}

		// Throw up our hands. Note: We Should Never Get Here(TM)
		return "It's unique, OK?";
	},
	_ = require( "underscore" ),
	OicResource = function( init ) {
		if ( !this._isOicResource ) {
			return new OicResource( init );
		}

		if ( init ) {
			_.extend( this, init );
		}

		// Create an id from the native sid
		this.id = createUuid( init.sid );
	};

_.extend( OicResource.prototype, {
	_isOicResource: true,

	// Is this needed?
	type: iotivity.OCPayloadType.PAYLOAD_TYPE_REPRESENTATION,
	_server: null
} );

module.exports = OicResource;
