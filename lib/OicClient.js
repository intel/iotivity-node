var iotivity = require( "bindings" )( "iotivity" ),
	_ = require( "underscore" ),
	OicResource = require( "./OicResource" ).OicResource,
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

	addEventListener: function( event, callback ) {
		this.addListener( event, callback );
	},

	removeEventListener: function( event, callback ) {
		this.removeListener( event, callback );
	},

	dispatchEvent: function( event, request ) {
		if ( typeof this[ "on" + event ] === "function" ) {
			this[ "on" + event ]( request );
		}
		this.emit( event, request );
	},

	findResources: function( options ) {}
} );

module.exports = {
	OicClient: OicClient
};
