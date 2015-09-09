var _ = require( "underscore" ),
	testUtils = function( iotivity ) {
		if ( !this._isTestUtils ) {
			return new testUtils( iotivity );
		}

		this._iotivity = iotivity;
	};

_.extend( testUtils.prototype, {
	_isTestUtils: true,

	lookupEnumValueName: function( enumName, value ) {
		var index,
			enumeration = this._iotivity[ enumName ];

		for ( index in enumeration ) {
			if ( enumeration[ index ] === value ) {
				return index;
			}
		}
	},
	lookupBitfieldValueNames: function( enumName, value ) {
		var index,
			enumeration = this._iotivity[ enumName ],
			bits = {};

		for ( index in enumeration ) {
			if ( value & enumeration[ index ] ) {
				bits[ index ] = true;
			}
		}
		return bits;
	},

	// Create an assertion and pass it to the parent process via stdout
	assert: function( assertion ) {
		var copyOfArguments;

		// Copy the arguments and remove the assertion
		copyOfArguments = Array.prototype.slice.call( arguments, 0 );
		copyOfArguments.shift();

		console.log( JSON.stringify( {
			assertion: assertion,
			arguments: copyOfArguments
		} ) );
	},
	stackOKOrDie: function( module, nameOfStep, result ) {
		console.log( JSON.stringify( {
			assertion: "strictEqual",
			arguments: [
				this.lookupEnumValueName( "OCStackResult", result ),
				"OC_STACK_OK",
				module + ": " + nameOfStep + " successful"
			]
		} ) );
		if ( result === this._iotivity.OCStackResult.OC_STACK_OK ) {
			return true;
		} else {
			this.die( module + ": " + nameOfStep + " failed" );
			return false;
		}
	},
	die: function( message ) {
		console.log( JSON.stringify( { teardown: true, message: message, isError: true } ) );
		process.exit( 1 );
	},
	findResource: function( response, uuid ) {
		var index,
			resources,
			returnValue = false;

		if ( response &&
				response.payload &&
				response.payload.resources &&
				response.payload.resources.length ) {

			resources = response.payload.resources;

			for ( index in resources ) {
				if ( resources[ index ].uri === "/a/" + uuid ) {
					return response.addr;
				}
			}
		}

		return returnValue;
	}
} );

module.exports = testUtils;
