// su -l root -c 'screen -dr edison || screen -S edison /dev/ttyUSB0 115200'

var intervalId,
	sensorFd,
	counter = 0,
	ocStarted = false,
	handle = {},
	fs = require( "fs" ),
	sensor = require( "LSM9DS0" ),
	iotivity = require( "iotivity" );

function teardown( message ) {
	if ( intervalId ) {
		clearInterval( intervalId );
	}

	if ( ocStarted ) {
		if ( iotivity.OCStackResult.OC_STACK_OK !== iotivity.OCStop() ) {
			message = ( message ? message + "\n" : "" ) + "OCStop() failed";
		}
	}

	if ( sensorFd >= 0 ) {
		fs.closeSync( sensorFd );
	}

	if ( message ) {
		console.error( message );
	}
	process.exit( message ? 1 : 0 );
}

sensorFd = sensor.open(
	"/dev/i2c-1",
	sensor.GyroScale.GYRO_SCALE_245DPS,
	sensor.MagScale.MAG_SCALE_2GS,
	sensor.AccelScale.ACCEL_SCALE_2G );

if ( sensorFd <= 0 ) {
	teardown( "Opening the sensor failed!" );
}

iotivity.OCStackResult.OC_STACK_OK == iotivity.OCInit( null, 0, iotivity.OCMode.OC_SERVER ) ||
	teardown( "OCInit() failed" );

ocStarted = true;

intervalId = setInterval( function() {
	iotivity.OCStackResult.OC_STACK_OK == iotivity.OCProcess() ||
		teardown( "OCProcess() failed" );
	if ( !( counter++ % 10 ) ) {
		iotivity.OCNotifyAllObservers( handle, iotivity.OCQualityOfService.OC_HIGH_QOS );
	}
}, 100 );


iotivity.OCStackResult.OC_STACK_OK == iotivity.OCCreateResource(
	handle,
	"core.9dof",
	iotivity.OC_RSRVD_INTERFACE_DEFAULT,
	"/a/9dof",
	function( flag, request ) {
		var realPayload,
			bias = { x: 0, y: 0, z: 0 },
			m_scale = { x: 0, y: 0, z: 0 },
			payload = {},
			accel = {},
			gyro = {},
			mag = {};

		sensor.read_acc( sensorFd, bias, sensor.AccelScale.ACCEL_SCALE_2G, accel );
		sensor.read_gyro( sensorFd, bias, sensor.GyroScale.GYRO_SCALE_245DPS, gyro );
		sensor.read_mag( sensorFd, bias, m_scale, sensor.MagScale.MAG_SCALE_2GS, mag );

		realPayload = {
			"0": "accel-x", "1": "float", "2": accel.x,
			"3": "accel-y", "4": "float", "5": accel.y,
			"6": "accel-z", "7": "float", "8": accel.z,
			"9": "gyro-x", "10": "float", "11": gyro.x,
			"12": "gyro-y", "13": "float", "14": gyro.y,
			"15": "gyro-z", "16": "float", "17": gyro.z,
			"18": "mag-x", "19": "float", "20": mag.x,
			"21": "mag-x", "22": "float", "23": mag.y,
			"24": "mag-x", "25": "float", "26": mag.z
		};

		console.log( "Sending data:" );
		console.log( realPayload );

		payload[ iotivity.OC_RSRVD_HREF ] = {};
		payload[ iotivity.OC_RSRVD_HREF ][ iotivity.OC_RSRVD_REPRESENTATION ] = realPayload;
		payload = JSON.stringify( payload );

		iotivity.OCDoResponse( {
			requestHandle: request.requestHandle.handle,
			resourceHandle: request.resource.handle,
			ehResult: iotivity.OCEntityHandlerResult.OC_EH_OK,
			payload: payload,
			payloadSize: payload.length,
			numSendVendorSpecificHeaderOptions: 0,
			sendVendorSpecificHeaderOptions: [],
			resourceUri: 0,
			persistentBufferFlag: 0
		} );

		return iotivity.OCEntityHandlerResult.OC_EH_OK;
	},
	iotivity.OCResourceProperty.OC_DISCOVERABLE | iotivity.OCResourceProperty.OC_OBSERVABLE ) ||
		teardown( "OCCreateResource() failed" );

process.on( "SIGINT", function() {
	console.log( "SIGINT: Quitting..." );
	teardown();
} );
