var _ = require( "lodash" ),
	commonJSHintOptions = require( "../../jshint-options.json" );

module.exports = {
	nonlib: {
		options: commonJSHintOptions,
		files: {
			src: []
				.concat( require( "../../js-example-and-test-files" ) )
				.concat( require( "../../js-build-files" ) )
		}
	},
	lib: {
		options: _.extend( {}, commonJSHintOptions, { predef: [ "-console" ] } ),
		files: {
			src: require( "../../js-lib-files" )
		}
	}
};
