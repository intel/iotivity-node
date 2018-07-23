var util = require( "../assert-to-console" );

var iotivity = require(process.argv[3]+"/lowlevel");

const uuids = ["af000000-000f-000f-000f-000000000001",
               "AF000000-000f-000f-000f-000000000002",
               "AF000000-000F-000F-000F-000000000003",
              ];

console.log("Launch unit test for the OCServer Identity configuration")
setImmediate(() => {
    iotivity.OCInit( null, 0, iotivity.OCMode.OC_SERVER);
    /* TEST SUITE */

    for (var i =0; i < uuids.length; ++i)
    {
        iotivity.OCSetDeviceId(uuids[i]);
        util.assert("strictEqual", uuids[i], iotivity.OCGetServerInstanceIDString(),
                    "OCSetDeviceId failed, still have ServerInstanceIDString : "+iotivity.OCGetServerInstanceIDString() );
        console.log("");
        if (i+1 == uuids.length) {
            i = -1;
            console.log("Register persistent storage handler.")
            console.log("");
            iotivity.OCRegisterPersistentStorageHandler(require( process.argv[3]+"lib/StorageHandler" )());
        }
        iotivity.OCProcess();
    }
    iotivity.OCStop();
});
