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

// Launch a JS file in a child process with certain provisions:
// 1. Pass a single command line argument: options=<JSON string>
// 2. Make sure that when this process exits the child process is TERMinated
// 3. Read stdout from the child process, parse it as JSON, and call @outputHandler with the result
// 4. Return a function that will terminate the child process
function testApp( filename, options, outputHandler ) {

		// Launch the child process and store the resulting process object (1)
		var testAppProcess = spawn( "node",
			[ path.join( __dirname, filename ) ]
				.concat( options ? [ "options=" + JSON.stringify( options ) ] : [] ) ),

			// Function that stops the child process
			stopTestAppProcess = function() {
				testAppProcess.kill( "SIGTERM" );
				process.removeListener( "exit", stopTestAppProcess );
			}

		// If this process should exit for any reason, kill the child process (2)
		process.on( "exit", stopTestAppProcess );

		// Read stdout from the child process, turn each line into a JSON object, and call the
		// callback with it (3)
		testAppProcess.stdout.on( "data", function testAppProcessStdoutData( data ) {
			_.each( data.toString().split( "\n" ), function( value ) {
				if ( value ) {
					outputHandler( JSON.parse( value ) );
				}
			} );
		} );

		// Return to the caller the function that will kill the child process (4)
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
		return testApp( "test-server.js", options,
			_.bind( function testServerOutputHandler( jsonObject ) {
				if ( jsonObject.result !== this._iotivity.OCStackResult.OC_STACK_OK ) {
					teardown();
				}

				if ( jsonObject.call == "OCCreateResource" ) {
					whenReady();
				}
			}, this ) );
	},

	startTestClient: function( teardown, options, dataHandler ) {
		return testApp( "test-client.js", options, dataHandler ? dataHandler :
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
