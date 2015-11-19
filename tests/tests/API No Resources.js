var device = require( "../../index" )(),
	utils = require( "../assert-to-console" ),
	iotivity = require( "../../lowlevel" );

console.log( JSON.stringify( { assertionCount: 2 } ) );

device.configure( {
	role: "client",
	connectionMode: "acked"
} ).then(
	function() {
		utils.assert( "strictEqual",
			device.client._processResources( null,
				{ result: iotivity.OCStackResult.OC_STACK_NO_RESOURCE },
				function() {
					utils.assert( "ok", true, "_processResources() fulfills promise" );
				}, function() {} ),
			iotivity.OCStackApplicationResult.OC_STACK_KEEP_TRANSACTION,
			"_processResources() handles OC_STACK_NO_RESOURCE" );
		process.exit( 0 );
	},
	function() {
		utils.die( "device.configure() failed" );
	} );
