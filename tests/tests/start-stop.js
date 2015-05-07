require( "../setup" );

var iotivity = require( "../../index" ),
	testUtils = require( "../test-utils" )( iotivity, QUnit.assert );

test( "Stack start/stop", function( assert ) {
	if ( testUtils.testStartup( iotivity.OCMode.OC_CLIENT_SERVER ) ===
			iotivity.OCStackResult.OC_STACK_OK ) {
		testUtils.testShutdown();
	}
} );
