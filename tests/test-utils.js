var _ = require( "underscore" ),
	path = require( "path" ),
	spawn = require( "child_process" ).spawn,
	testUtils = function( iotivity, assert ) {
	if ( !this._isTestUtils ) {
		return new testUtils( iotivity, assert );
	}

	this._iotivity = iotivity;
	this._assert = assert;
};

function testApp( filename, teardown, options, outputHandler ) {
		var testAppProcess = spawn( "node",
			[ path.join( __dirname, filename ) ]
				.concat( options ? [ "options=" + JSON.stringify( options ) ] : [] ) ),
			stopTestAppProcess = function() {
				testAppProcess.kill( "SIGTERM" );
				process.removeListener( "exit", stopTestAppProcess );
			}

		process.on( "exit", stopTestAppProcess );

		testAppProcess.stdout.on( "data", function testAppProcessStdoutData( data ) {
			_.each( data.toString().split( "\n" ), function( value ) {
				if ( value ) {
					outputHandler( JSON.parse( value ) );
				}
			} );
		} );

		return stopTestAppProcess;
}

_.extend( testUtils.prototype, {
	_isTestUtils: true,

	testStartup: function( mode ) {
		var result = this._iotivity.OCInit( null, 0, mode );

		this._assert.deepEqual(
			this.lookupEnumValueName( "OCStackResult", result ),
			"OC_STACK_OK",
			"OCInit in " + this.lookupEnumValueName( "OCMode", mode ) + " mode has succeeded" );

		return result;
	},

	testProcessing: function( interval, teardown ) {
		var callCount = 0,
			result = this._iotivity.OCStackResult.OC_STACK_OK,
			id = setInterval( _.bind( function() {
				result = this._iotivity.OCProcess();
				if ( result === this._iotivity.OCStackResult.OC_STACK_OK ) {
					callCount++;
				} else {
					teardown();
				}
			}, this ), interval );

		return _.bind( function stopProcessing() {
			clearInterval( id );
			this._assert.deepEqual(
				this.lookupEnumValueName( "OCStackResult", result ),
				"OC_STACK_OK",
				"OCProcess has not failed once in " + callCount + " calls" );
		}, this );
	},

	startTestServer: function( whenReady, teardown, options ) {
		return testApp( "test-server.js", teardown, options,
			_.bind( function testServerOutputHandler( jsonObject ) {
				if ( jsonObject.result !== this._iotivity.OCStackResult.OC_STACK_OK ) {
					teardown();
				}

				if ( jsonObject.call == "OCCreateResource" ) {
					whenReady();
				}
			}, this ) );
	},

	startTestClient: function( teardown, options ) {
		return testApp( "test-client.js", teardown, options,
			_.bind( function testClientOutputHandler( jsonObject ) {
				if ( jsonObject.result !== this._iotivity.OCStackResult.OC_STACK_OK ) {
					teardown();
				}
			}, this ) );
	},

	testShutdown: function() {
		var result = this._iotivity.OCStop();

		this._assert.deepEqual(
			this.lookupEnumValueName( "OCStackResult", result ),
			"OC_STACK_OK",
			"OCStop has succeeded" );
	},

	lookupEnumValueName: function( enumName, value ) {
		var index,
			enumeration = this._iotivity[ enumName ];

		for ( index in enumeration ) {
			if ( enumeration[ index ] === value ) {
				return index;
			}
		}
	}

} );

module.exports = testUtils;
