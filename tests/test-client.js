var result,
	doHandle = {},
	processLoop = null,
	iotivity = require( "../index" );

result = iotivity.OCInit( null, 0, iotivity.OCMode.OC_CLIENT );

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

	result = iotivity.OCDoResource(
		doHandle,
		iotivity.OCMethod.OC_REST_GET,
		"/a/light",
		null,
		null,
		iotivity.OCConnectivityType.OC_ALL,
		iotivity.OCQualityOfService.OC_LOW_QOS,
		function( handle, response ) {
			return iotivity.OCStackApplicationResult.OC_STACK_DELETE_TRANSACTION;
		},
		null,
		0 );

	console.log( JSON.stringify( {
		call: "OCDoResource",
		result: result
	} ) );
}
