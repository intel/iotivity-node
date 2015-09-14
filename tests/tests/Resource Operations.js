var result, countReceptacle, initialResourceCount,
	childResourceHandleReceptacle = {},
	resourceHandleReceptacle = {},
	uuid = process.argv[ 2 ],
	iotivity = require( "../../index" ),
	testUtils = require( "../utils" )( iotivity ),
	initialEntityHandler = function( flag, request ) {
		return iotivity.OCEntityHandlerResult.OC_EH_ERROR;
	},
	alternateEntityHandler = function( flag, request ) {
		return iotivity.OCEntityHandlerResult.OC_EH_ERROR;
	};

// Report assertion count
console.log( JSON.stringify( { assertionCount: 33 } ) );

// Init
result = iotivity.OCInit( null, 0, iotivity.OCMode.OC_SERVER );
testUtils.stackOKOrDie( "OCInit", result );

// Set device info
result = iotivity.OCSetDeviceInfo( { deviceName: "resource-operations-" + uuid } );
testUtils.stackOKOrDie( "OCSetDeviceInfo", result );

// Set platform info
result = iotivity.OCSetPlatformInfo( {
	platformID: "server." + uuid,
	manufacturerName: "resource-ops"
} );
testUtils.stackOKOrDie( "OCSetPlatformInfo", result );

// Perform an initial count of resources
countReceptacle = {};
result = iotivity.OCGetNumberOfResources( countReceptacle );
testUtils.stackOKOrDie( "OCGetNumberOfResources(to establish initial count)", result );
initialResourceCount = countReceptacle.count;

// Create a resource
result = iotivity.OCCreateResource(
	resourceHandleReceptacle,
	"core.fan",
	iotivity.OC_RSRVD_INTERFACE_DEFAULT,
	"/a/" + uuid,
	initialEntityHandler,
	iotivity.OCResourceProperty.OC_SLOW |
	iotivity.OCResourceProperty.OC_ACTIVE );
testUtils.stackOKOrDie( "OCCreateResource(parent)", result );

// Count resources again
countReceptacle = {};
result = iotivity.OCGetNumberOfResources( countReceptacle );
testUtils.stackOKOrDie( "OCGetNumberOfResources", result );
testUtils.assert( "strictEqual", countReceptacle.count, initialResourceCount + 1,
	"OCGetNumberOfResources reports an increase by one resource after OCCreateResource" );

// Make sure the initial entity handler is reported
testUtils.assert( "ok",
	iotivity.OCGetResourceHandler( resourceHandleReceptacle.handle ) === initialEntityHandler,
	"OCGetResourceHandler initially returns the initial entity handler" );

// Change the entity handler
result = iotivity.OCBindResourceHandler( resourceHandleReceptacle.handle, alternateEntityHandler );
testUtils.stackOKOrDie( "OCBindResourceHandler", result );

// Make sure the alternate entity handler is reported
testUtils.assert( "ok",
	iotivity.OCGetResourceHandler( resourceHandleReceptacle.handle ) === alternateEntityHandler,
	"OCGetResourceHandler returns the alternate entity handler after setting it" );

// Make sure the stack returns the correct URI
testUtils.assert( "strictEqual",
	iotivity.OCGetResourceUri( resourceHandleReceptacle.handle ),
	"/a/" + uuid,
	"OCGetResourceUri returns the correct URI" );

// *** Resource interfaces

// Initially there should only be one interface associated with our resource.
countReceptacle = {};
result = iotivity.OCGetNumberOfResourceInterfaces(
	resourceHandleReceptacle.handle,
	countReceptacle );
testUtils.stackOKOrDie( "OCGetNumberOfResourceInterfaces", result );
testUtils.assert( "strictEqual", countReceptacle.count, 1,
	"Initially there is only one interface on the resource" );

// Add another interface
result = iotivity.OCBindResourceInterfaceToResource( resourceHandleReceptacle.handle, "a.b.c" );
testUtils.stackOKOrDie( "OCBindResourceInterfaceToResource", result );

// After adding one interface there should be two interfaces associated with our resource.
countReceptacle = {};
result = iotivity.OCGetNumberOfResourceInterfaces(
	resourceHandleReceptacle.handle,
	countReceptacle );
testUtils.stackOKOrDie( "OCGetNumberOfResourceInterfaces", result );
testUtils.assert( "strictEqual", countReceptacle.count, 2,
	"After adding an interface there are two interfaces on the resource" );

testUtils.assert( "strictEqual",
	iotivity.OCGetResourceInterfaceName( resourceHandleReceptacle.handle, 0 ),
	iotivity.OC_RSRVD_INTERFACE_DEFAULT,
	"First interface is '" + iotivity.OC_RSRVD_INTERFACE_DEFAULT + "'" );

testUtils.assert( "strictEqual",
	iotivity.OCGetResourceInterfaceName( resourceHandleReceptacle.handle, 1 ),
	"a.b.c",
	"Second interface is 'a.b.c'" );

// *** Resource types

// Add another type
result = iotivity.OCBindResourceTypeToResource( resourceHandleReceptacle.handle, "core.led" );
testUtils.stackOKOrDie( "OCBindResourceTypeToResource", result );

// After adding one type there should be two types associated with our resource.
countReceptacle = {};
result = iotivity.OCGetNumberOfResourceTypes(
	resourceHandleReceptacle.handle,
	countReceptacle );
testUtils.stackOKOrDie( "OCGetNumberOfResourceTypes", result );
testUtils.assert( "strictEqual", countReceptacle.count, 2,
	"After adding a type there are two types on the resource" );

testUtils.assert( "strictEqual",
	iotivity.OCGetResourceTypeName( resourceHandleReceptacle.handle, 0 ),
	"core.fan",
	"First type is correct" );

testUtils.assert( "strictEqual",
	iotivity.OCGetResourceTypeName( resourceHandleReceptacle.handle, 1 ),
	"core.led",
	"Second type is correct" );

// Make sure properties are retrieved correctly
testUtils.assert( "deepEqual",
	testUtils.lookupBitfieldValueNames( "OCResourceProperty",
		iotivity.OCGetResourceProperties( resourceHandleReceptacle.handle ) ),
	{ OC_SLOW: true, OC_ACTIVE: true },
	"OCGetResourceProperties correctly returns properties set on resource" );

testUtils.assert( "ok",
	iotivity.__compareResourceHandles( resourceHandleReceptacle.handle,
		iotivity.OCGetResourceHandle( initialResourceCount ) ),
	"OCGetResourceHandle for the index of the new resource returns the original handle" );

// Create a child resource to test the collection APIs
result = iotivity.OCCreateResource(
	childResourceHandleReceptacle,
	"core.fan",
	iotivity.OC_RSRVD_INTERFACE_DEFAULT,
	"/a/" + uuid + "-child",
	initialEntityHandler,
	iotivity.OCResourceProperty.OC_SLOW |
	iotivity.OCResourceProperty.OC_ACTIVE );
testUtils.stackOKOrDie( "OCCreateResource(child)", result );

testUtils.assert( "strictEqual",
	iotivity.OCGetResourceHandleFromCollection( resourceHandleReceptacle.handle, 0 ),
	null,
	"The first child resource handle is initially null" );

result = iotivity.OCBindResource(
	resourceHandleReceptacle.handle,
	childResourceHandleReceptacle.handle );
testUtils.stackOKOrDie( "OCBindResource", result );

// After having attached the child resource to the parent resource, it should be returned as the
// first child handle in the collection
testUtils.assert( "ok",
	iotivity.__compareResourceHandles(
		iotivity.OCGetResourceHandleFromCollection( resourceHandleReceptacle.handle, 0 ),
		childResourceHandleReceptacle.handle ),
	"Retrieving the child resource handle from the collection returns the child resource handle" );

result = iotivity.OCUnBindResource(
	resourceHandleReceptacle.handle,
	childResourceHandleReceptacle.handle );
testUtils.stackOKOrDie( "OCUnBindResource", result );

// After having detached the child resource from the parent resource, the parent resource should
// return null as the first child handle in the collection
testUtils.assert( "strictEqual",
	iotivity.OCGetResourceHandleFromCollection( resourceHandleReceptacle.handle, 0 ),
	null,
	"The first child resource handle is null after having detached the child from the parent" );

// Delete the child resource
result = iotivity.OCDeleteResource( childResourceHandleReceptacle.handle );
testUtils.stackOKOrDie( "OCDeleteResource(child)", result );

// Delete the resource
result = iotivity.OCDeleteResource( resourceHandleReceptacle.handle );
testUtils.stackOKOrDie( "OCDeleteResource(parent)", result );
