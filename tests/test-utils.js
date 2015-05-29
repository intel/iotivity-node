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
		var testServer = spawn( "node",
			[ path.join( __dirname, "test-server.js" ) ]
				.concat( options ?
					[ "options=" + JSON.stringify( options ) ] : [] ) );

		testServer.stdout.on( "data", _.bind( function testServerStdoutData( data ) {
			_.each( data.toString().split( "\n" ), _.bind( function( value ) {
				var jsonObject;

				if ( value ) {

					jsonObject = JSON.parse( value );

					if ( jsonObject.result !== this._iotivity.OCStackResult.OC_STACK_OK ) {
						teardown();
					}

					if ( jsonObject.call == "OCCreateResource" ) {
						whenReady();
					}
				}
			}, this ) );
		}, this ) );

		return function stopTestServer() {
			testServer.kill( "SIGTERM" );
		};
	},

	startTestClient: function( teardown ) {
		var testClient = spawn( "node", [ path.join( __dirname, "test-client.js" ) ] );

		testClient.stdout.on( "data", _.bind( function testServerStdoutData( data ) {
			_.each( data.toString().split( "\n" ), _.bind( function( value ) {
				var jsonObject;

				if ( value ) {
					jsonObject = JSON.parse( value );

					if ( jsonObject.result !== this._iotivity.OCStackResult.OC_STACK_OK ) {
						teardown();
					}
				}
			}, this ) );
		}, this ) );

		return function stopTestClient() {
			testClient.kill( "SIGTERM" );
		};
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
