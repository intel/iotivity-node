var _ = require( "lodash" ),
	TestUtils = function( iotivity ) {
		if ( !this._isTestUtils ) {
			return new TestUtils( iotivity );
		}

		this._iotivity = iotivity;
	};

_.extend( TestUtils.prototype, require( "./assert-to-console" ), {
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

	stackOKOrDie: function( module, nameOfStep, result ) {

		// Two-argument configuration means module was skipped
		if ( arguments.length === 2 ) {
			result = nameOfStep;
			nameOfStep = module;
			module = "";
		}

		console.log( JSON.stringify( {
			assertion: "strictEqual",
			arguments: [
				this.lookupEnumValueName( "OCStackResult", result ),
				"OC_STACK_OK",
				( module ? module + ": " : "" ) + nameOfStep + " successful"
			]
		} ) );
		if ( result === this._iotivity.OCStackResult.OC_STACK_OK ) {
			return true;
		} else {
			this.die( module + ": " + nameOfStep + " failed" );
			return false;
		}
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

module.exports = TestUtils;
