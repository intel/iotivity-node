var iotivity = require( "bindings" )( "iotivity" );

module.exports = {
	sidToString: function( sid ) {
		var result,
			idReceptacle = {};

		// If we get an sid convert it to a string
		if ( sid ) {
			result = iotivity.OCConvertUuidToString( sid, idReceptacle );
			if ( result === iotivity.OCRandomUuidResult.RAND_UUID_OK ) {
				return idReceptacle.sid;
			}
		}

		// Otherwise generate a Uuid string
		result = iotivity.OCGenerateUuidString( idReceptacle );
		if ( result === iotivity.OCRandomUuidResult.RAND_UUID_OK ) {
			return idReceptacle.sid;
		}

		throw new Error( "OicResource: Failed to generate UUID" );
	}
};
