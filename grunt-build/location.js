var path = require( "path" );

module.exports = function( grunt ) {
	var result = grunt.option( "ci" ) ?
		path.dirname( require.resolve( "iotivity-node" ) ) :
		process.cwd();
  return result;
};
