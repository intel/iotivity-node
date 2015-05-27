var result, payload, argumentIndex,
	resourceHandle = {},
	processLoop = null,
	iotivity = require( "../index" );

for ( argumentIndex in process.argv ) {
	if ( process.argv[ argumentIndex ].substr( 0, 9 ) === "response=" ) {
		payload = process.argv[ argumentIndex ].substr( 9 );
	}
}

result = iotivity.OCInit( null, 0, iotivity.OCMode.OC_SERVER );

console.log( JSON.stringify( {
	call: "OCInit",
	result: result
} ) );

if ( result === iotivity.OCStackResult.OC_STACK_OK ) {
	processLoop = setInterval( function() {
		var processResult = iotivity.OCProcess();
		if ( processResult !== iotivity.OCStackResult.OC_STACK_OK ) {
			console.log( JSON.stringify( {
				call: "OCProcess",
				result: processResult
			} ) );
			process.exit( 1 );
		}
	}, 100 );

	result = iotivity.OCCreateResource(
		resourceHandle,
		"light",
		"oc.mi.def",
		"/light/1",
		function( flag, request ) {
			if ( payload ) {
				console.log( JSON.stringify( {
					call: "OCDoResponse",
					result: iotivity.OCDoResponse( {
						requestHandle: request.requestHandle.handle,
						resourceHandle: request.resource.handle,
						ehResult: iotivity.OCEntityHandlerResult.OC_EH_OK,
						payload: payload,
						payloadSize: payload.length,
						numSendVendorSpecificHeaderOptions: 0,
						sendVendorSpecificHeaderOptions: [],
						resourceUri: 0,
						persistentBufferFlag: 0
					} )
				} ) );
			}
			return iotivity.OCEntityHandlerResult.OC_EH_OK;
		},
		iotivity.OCResourceProperty.OC_DISCOVERABLE );

	console.log( JSON.stringify( {
		call: "OCCreateResource",
		result: result
	} ) );
}
