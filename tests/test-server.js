var result,
	resourceHandle = {},
	processLoop = null,
	iotivity = require( "../index" );

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
			return iotivity.OCEntityHandlerResult.OC_EH_OK;
		},
		iotivity.OCResourceProperty.OC_DISCOVERABLE );

	console.log( JSON.stringify( {
		call: "OCCreateResource",
		result: result
	} ) );
}
