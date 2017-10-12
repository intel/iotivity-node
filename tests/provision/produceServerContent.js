var produceClientContent = require( "./produceClientContent" );

// Create two ACEs from the given resources - one for plain text and one for encrypted
function generateACEList( resources ) {
	return [
		{ subject: { "conntype": "anon-clear" }, resources: resources, "permission": 31 },
		{ subject: { "conntype": "auth-crypt" }, resources: resources, "permission": 31 }
	];
}

module.exports = function produceServerContent( item, creds, resourceUuid ) {
	var index;
	var result = produceClientContent( item, creds );

	result.acl.aclist2 = result.acl.aclist2.concat( generateACEList(
		Array.isArray( resourceUuid ) ? resourceUuid : [

			// Resource names used during testing
			// 1. iotivity-node
			{
				href: "/a/" + resourceUuid + "-xyzzy"
			},

			// 2. iot-js-api
			{
				href: "/a/" + resourceUuid
			},
			{
				href: "/direct"
			},
			{
				href: "/target-resource"
			},
			{
				href: "/disable-presence"
			},
			{
				href: "/some/new/resource"
			}
		] ) );

	// Finalize aceid property
	for ( index = 0; index < result.acl.aclist2.length; index++ ) {
		result.acl.aclist2[ index ].aceid = index + 1;
	}

	return result;
};
