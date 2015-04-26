// The native code
var iotivity = require( "bindings" )( "iotivity" );

module.exports = iotivity;

// Javascript callback hash used to retrieve JS callbacks from the native side
iotivity._callbacks = require( "./callbackmanager" )();

// Map JS callbacks to unique hash keys via the callbackmanager for C APIs that take callbacks
iotivity.OCCreateResource = function(
		resourceTypeName,
		resourceInterfaceName,
		uri,
		entityHandler,
		resourceProperties ) {

	return this._partial_OCCreateResource(
		resourceTypeName,
		resourceInterfaceName,
		uri,
		this._callbacks.add( entityHandler ),
		resourceProperties );

};

// Remove JS callback from callbackmanager and then do the native stuff
iotivity.OCDeleteResource = function( handle ) {
	this._callbacks.remove( handle[ 2 ] );
	return this._partial_OCDeleteResource( handle );
};
