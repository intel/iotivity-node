var result,
	options = {
		path: "/a/light",
		logResponse: false,
		request: null
	},
	doHandle = {},
	processLoop = null,
	_ = require( "underscore" ),
	iotivity = require( "../lowlevel" );

// Merge options from the command line
for ( argumentIndex in process.argv ) {
	if ( process.argv[ argumentIndex ].substr( 0, 8 ) === "options=" ) {
		options = _.extend( {}, options, JSON.parse( process.argv[ argumentIndex ].substr( 8 ) ) );
	}
}

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
		options.path,
		null,
		JSON.stringify( options.request ),
		iotivity.OCConnectivityType.OC_ALL,
		iotivity.OCQualityOfService.OC_LOW_QOS,
		function( handle, response ) {
			if ( options.logResponse ) {
				console.log( JSON.stringify( {
					"OCDoResource response": response
				} ) );
			}
			return iotivity.OCStackApplicationResult.OC_STACK_DELETE_TRANSACTION;
		},
		null,
		0 );

	console.log( JSON.stringify( {
		call: "OCDoResource",
		result: result
	} ) );
}
