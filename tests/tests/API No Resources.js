var device = require( "../../index" )(),
	utils = require( "../assert-to-console" ),
	iotivity = require( "../../lowlevel" );

console.log( JSON.stringify( { assertionCount: 1 } ) );

device.configure( {
	role: "client"
} ).then(
	function() {
		utils.assert( "strictEqual",
			device._processResources( {
				result: iotivity.OCStackResult.OC_STACK_NO_RESOURCE
			} ),
			undefined,
			"_processResources() handles OC_STACK_NO_RESOURCE" );
		process.exit( 0 );
	},
	function() {
		utils.die( "device.configure() failed" );
	} );
