var _ = require( "underscore" ),
/*	OicResource = require( "./OicResource" ).OicResource,
	iotivity = require( "bindings" )( "iotivity" ), */
	OicClient = function() {
		if ( !this._isOicClient ) {
			return new OicClient();
		}

	};

require( "util" ).inherits( OicClient, require( "events" ).EventEmitter );

_.extend( OicClient.prototype, {
	_isOicClient: true,
	onresourcechange: null,
	onresourcefound: null,
	ondevicefound: null,

	addEventListener: OicClient.prototype.addListener,

	removeEventListener: OicClient.prototype.removeEventListener,

	dispatchEvent: function( event, request ) {
		if ( typeof this[ "on" + event ] === "function" ) {
			this[ "on" + event ]( request );
		}
		this.emit( event, request );
	},

	// Takes one parameter: @options
	findResources: function() {}
} );

module.exports = {
	OicClient: OicClient
};
