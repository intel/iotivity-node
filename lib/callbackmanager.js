module.exports = function() {
	return {
		uuid: 0,
		callbacks: {},
		reverseCallbacks: {},

		add: function( callback ) {
			var uuid = this.uuid++;

			this.callbacks[ uuid ] = callback;
			this.reverseCallbacks[ callback ] = uuid;

			return uuid;
		},

		remove: function( which ) {
			var uuid, callback;

			if ( typeof which === "number" ) {
				uuid = which;
				callback = this.callbacks[ uuid ];
			} else {
				callback = which;
				uuid = this.reverseCallbacks[ callback ];
			}

			if ( !( uuid === undefined || callback === undefined ) ) {
				delete this.callbacks[ uuid ];
				delete this.reverseCallbacks[ callback ];
			}
		}
	};
};
