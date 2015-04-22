// The native code
var iotivity = require( "bindings" )( "iotivity" );

module.exports = iotivity;

iotivity._callbacks = require( "./callbackmanager" );

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
