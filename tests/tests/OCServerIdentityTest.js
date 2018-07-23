if ( process.argv.length > 4 && process.argv[ 4 ] === "child" ) {
	var assert = require( "assert" );
	var iotivity = require( process.argv[ 3 ] + "/lowlevel" );
	assert(
		iotivity.OCRegisterPersistentStorageHandler( require( "../../lib/StorageHandler" )() ) ===
			iotivity.OCStackResult.OC_STACK_OK );
	assert( iotivity.OCInit( null, 0, iotivity.OCMode.OC_SERVER ) ===
		iotivity.OCStackResult.OC_STACK_OK );
	if ( process.argv[ 5 ] === "get" ) {
		console.log( iotivity.OCGetServerInstanceIDString() );
	} else {
		iotivity.OCSetDeviceId( process.argv[ 6 ] );
	assert( iotivity.OCProcess() === iotivity.OCStackResult.OC_STACK_OK );
	assert( iotivity.OCStop() === iotivity.OCStackResult.OC_STACK_OK );
	}
	return;
}

var util = require( "../assert-to-console" );

var iotivity = require( process.argv[ 3 ] + "/lowlevel" );

var spawn = require( "child_process" ).spawn;

var uuids = [
	"af000000-000f-000f-000f-000000000001",
	"AF000000-000f-000f-000f-000000000002",
	"AF000000-000F-000F-000F-000000000003"
];

// Report assertion count
console.log( JSON.stringify( { assertionCount: 6 } ) );

// Init
iotivity.OCInit( null, 0, iotivity.OCMode.OC_SERVER );
for ( var i = 0; i < uuids.length; ++i ) {
	iotivity.OCSetDeviceId( uuids[ i ] );
	util.assert( "strictEqual", iotivity.OCGetServerInstanceIDString( ), uuids[ i ].toLowerCase(),
		"Set device ID" );

	iotivity.OCProcess( );
}
iotivity.OCStop( );

function launchChild( uuid ) {
	return new Promise( function( resolve, reject ) {
		var output = "";
		var child = spawn( process.argv[ 0 ], [
				process.argv[ 1 ],
				process.argv[ 2 ],
				process.argv[ 3 ],
				"child"
			].concat( !!uuid ? [ "set", uuid ] : [ "get" ] ), {
				stdio: [ process.stdin, "pipe", process.stderr ]
			} );
		child.on( "close", function( code ) {
			if ( code === null || code !== 0 || !!uuid && output !== "" ) {
				reject();
			} else {
				resolve( output );
			}
		} );
		child.stdout.on( "data", function( data ) {
			output += data.toString();
		} );
	} );
}

launchChild( uuids[ 0 ] )
	.then( function() {
		return launchChild();
	} )
	.then( function( result ) {
		util.assert( "strictEqual", result.match( /\S+/g )[ 0 ], uuids[ 0 ].toLowerCase(),
			"Set device ID persistently" );
		return launchChild( uuids[ 1 ] );
	} )
	.then( function() {
		return launchChild();
	} )
	.then( function( result ) {
		util.assert( "strictEqual", result.match( /\S+/g )[ 0 ], uuids[ 1 ].toLowerCase(),
			"Set device ID persistently" );
		return launchChild( uuids[ 2 ] );
	} )
	.then( function() {
		return launchChild();
	} )
	.then( function( result ) {
		util.assert( "strictEqual", result.match( /\S+/g )[ 0 ], uuids[ 2 ].toLowerCase(),
			"Set device ID persistently" );
	} );
