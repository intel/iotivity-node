var result,
	options = {
		method: "OC_REST_GET",
		path: "/a/light",
		logResponse: false,
		request: null
	},
	doHandle = {},
	processLoop = null,
	_ = require( "underscore" ),
	iotivity = require( "../index" );

// Merge options from the command line
for ( argumentIndex in process.argv ) {
	if ( process.argv[ argumentIndex ].substr( 0, 8 ) === "options=" ) {
		options = _.extend( {}, options, JSON.parse( process.argv[ argumentIndex ].substr( 8 ) ) );
	}
}

result = iotivity.OCInit( null, 0, iotivity.OCMode.OC_CLIENT );

console.log( JSON.stringify( {
	call: "OCInit",
	options: options,
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

	try {
		result = iotivity.OCDoResource(
			doHandle,
			iotivity.OCMethod[ options.method ],
			options.path,
			null,
			options.request ? options.request : null,
			iotivity.OCConnectivityType.CT_DEFAULT,
			iotivity.OCQualityOfService.OC_HIGH_QOS,
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
	} catch( e ) {
		result = e.message;
	}

	console.log( JSON.stringify( {
		call: "OCDoResource",
		result: result
	} ) );
}
