var iotivity = require( "bindings" )( "iotivity" ),
	util = require( "util" );

function objectToPayload( theObject, visitedObjects ) {
	var index, theValue,
		payload = { type: iotivity.OCPayloadType.PAYLOAD_TYPE_REPRESENTATION, values: {} };

	if ( visitedObjects === undefined ) {
		visitedObjects = {};
	}

	for ( index in theObject ) {
		theValue = theObject[ index ];
		if ( typeof theValue === "object" ) {
			if ( visitedObjects[ theValue ] ) {
				throw new Error( "objectToPayload: Cannot handle circular object references" );
			}
			visitedObjects[ theValue ] = true;
			if ( !util.isArray( theValue ) ) {
				payload.values[ index ] = objectToPayload( theValue, visitedObjects );
				continue;
			}
		}
		payload.values[ index ] = theValue;
	}

	return payload;
}

function payloadToObject( values ) {
	var index, theValue,
		result = {};

	for ( index in values ) {
		theValue = values[ index ];
		if ( typeof theValue === "object" && !util.isArray( theValue ) ) {
			result[ index ] = payloadToObject( theValue.values );
		} else {
			result[ index ] = theValue;
		}
	}

	return result;
}

module.exports = {
	sidToString: function( sid ) {
		var result,
			idReceptacle = {};

		// If we get a sid convert it to a string
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
	},
	setPrivate: function( object, props ) {
		var index;

		for ( index in props ) {
			Object.defineProperty( object, props[ index ], {
				writable: true,
				configurable: true
			} );
		}
	},
	objectToPayload: objectToPayload,
	payloadToObject: payloadToObject
};
