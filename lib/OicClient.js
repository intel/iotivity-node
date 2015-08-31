var iotivity = require( "bindings" )( "iotivity" ),
	_ = require( "underscore" ),
	OicResource = require( "./OicResource" ).OicResource,
	OicClient = function() {
		if ( !this._isOicClient ) {
			return new OicClient();
		}

	};

_.extend( OicClient.prototype, {
		_isOicClient: true,
		onresourcechange: null,
		onresourcefound: null,
		ondevicefound: null,
		listeners: {},

		addEventListener: function( event, callback ) {
			if ( !( event in this.listeners ) ) {
				this.listeners[ event ] = [];
			}
			// Currently allows duplicate callbacks. Should it?
			this.listeners[ event ].push( callback );
		},

		removeEventListener: function( event, callback ) {
			if ( event in this.listeners ) {
				this.listeners [ event ] = this.listeners [ event ].filter ( function ( ev ) {
					return ev !== callback;
				});
			}
		},

		dispatchEvent: function ( event, request ) {
			if ( typeof this [ "on" + event ] === "function" ) {
				this [ "on" + event ] ( request );
			}
			if ( event in this.listeners ) {
				for ( var i = 0, len = this.listeners [ event ].length; i < len; i++ ) {
					this.listeners [ event ] [ i ].call( this, request );
				}
			}
		},

		findResources: function( options ) {

		},
});

module.exports = {
	OicClient: OicClient
}