var device = require( "../../index" )(),
	utils = require( "../assert-to-console" ),
	iotivity = require( "../../lowlevel" );

console.log( JSON.stringify( { assertionCount: 1 } ) );

device.configure( {
	role: "client",
	connectionMode: "acked"
} ).then(
	function() {
		try {
			utils.assert( "strictEqual",
				device.client._processFindResourceResponse( null,
					{ result: iotivity.OCStackResult.OC_STACK_NO_RESOURCE } ),
				iotivity.OCStackApplicationResult.OC_STACK_KEEP_TRANSACTION,
				"_processFindResourceRequest() handles OC_STACK_NO_RESOURCE" );
			process.exit( 0 );
		} catch ( error ) {
			utils.assert( "strictEqual", error, undefined,
				"_processFindResourceRequest throws no errors" );
		}
	},
	function() {
		utils.die( "device.configure() failed" );
	} );
