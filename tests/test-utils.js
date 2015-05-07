var testUtils = function( iotivity, assert ) {
	if ( !this._isTestUtils ) {
		return new testUtils( iotivity, assert );
	}

	this._iotivity = iotivity;
	this._assert = assert;
};
testUtils.prototype._isTestUtils = true;

testUtils.prototype.testStartup = function( mode ) {
	var result = this._iotivity.OCInit( null, 0, mode );

	this._assert.deepEqual(
		this.lookupEnumValueName( "OCStackResult", result ),
		"OC_STACK_OK",
		"OCInit in " + this.lookupEnumValueName( "OCMode", mode ) + " mode has succeeded" );

	return result;
};

testUtils.prototype.testProcessing = function( interval, teardown ) {
	var that = this,
		callCount = 0,
		result = that._iotivity.OCStackResult.OC_STACK_OK,
		id = setInterval( function() {
			result = that._iotivity.OCProcess();
			if ( result === that._iotivity.OCStackResult.OC_STACK_OK ) {
				callCount++;
			} else {
				teardown();
			}
		}, interval );

	return function stopProcessing() {
		clearInterval( id );
		that._assert.deepEqual(
			that.lookupEnumValueName( "OCStackResult", result ),
			"OC_STACK_OK",
			"OCProcess has not failed once in " + callCount + " calls" );
	};
};

testUtils.prototype.testShutdown = function() {
	var result = this._iotivity.OCStop();

	this._assert.deepEqual(
		this.lookupEnumValueName( "OCStackResult", result ),
		"OC_STACK_OK",
		"OCStop has succeeded" );
};

testUtils.prototype.lookupEnumValueName = function( enumName, value ) {
	var index,
		enumeration = this._iotivity[ enumName ];

	for ( index in enumeration ) {
		if ( enumeration[ index ] === value ) {
			return index;
		}
	}
}


module.exports = testUtils;
