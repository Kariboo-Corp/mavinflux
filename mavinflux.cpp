// ------------------------------------------------------------------------------
//   Includes
// ------------------------------------------------------------------------------

#include "mavinflux.h"


// ------------------------------------------------------------------------------
//   TOP
// ------------------------------------------------------------------------------
int
top (int argc, char **argv)
{

	// --------------------------------------------------------------------------
	//   PARSE THE COMMANDS
	// --------------------------------------------------------------------------

	// Default input arguments
#ifdef __APPLE__
	char *uart_name = (char*)"/dev/tty.usbmodem1";
#else
	char *uart_name = (char*)"/dev/ttyUSB0";
#endif
	int baudrate = 921600;

	bool use_udp = false;
	char *udp_ip = (char*)"127.0.0.1";
	int udp_port = 14540;
	bool autotakeoff = false;

	// do the parse, will throw an int if it fails
	parse_commandline(argc, argv, uart_name, baudrate, use_udp, udp_ip, udp_port, autotakeoff);


	// --------------------------------------------------------------------------
	//   PORT and THREAD STARTUP
	// --------------------------------------------------------------------------

	/*
	 * Instantiate a generic port object
	 *
	 * This object handles the opening and closing of the offboard computer's
	 * port over which it will communicate to an autopilot.  It has
	 * methods to read and write a mavlink_message_t object.  To help with read
	 * and write in the context of pthreading, it gaurds port operations with a
	 * pthread mutex lock. It can be a serial or an UDP port.
	 *
	 */
	Generic_Port *port;
	if(use_udp)
	{
		port = new UDP_Port(udp_ip, udp_port);
	}
	else
	{
		port = new Serial_Port(uart_name, baudrate);
	}


	/*
	 * Instantiate an autopilot interface object
	 *
	 * This starts two threads for read and write over MAVlink. The read thread
	 * listens for any MAVlink message and pushes it to the current_messages
	 * attribute.  The write thread at the moment only streams a position target
	 * in the local NED frame (mavlink_set_position_target_local_ned_t), which
	 * is changed by using the method update_setpoint().  Sending these messages
	 * are only half the requirement to get response from the autopilot, a signal
	 * to enter "offboard_control" mode is sent by using the enable_offboard_control()
	 * method.  Signal the exit of this mode with disable_offboard_control().  It's
	 * important that one way or another this program signals offboard mode exit,
	 * otherwise the vehicle will go into failsafe.
	 *
	 */
	Autopilot_Interface autopilot_interface(port);

	/*
	 * Setup interrupt signal handler
	 *
	 * Responds to early exits signaled with Ctrl-C.  The handler will command
	 * to exit offboard mode if required, and close threads and the port.
	 * The handler in this example needs references to the above objects.
	 *
	 */
	port_quit         = port;
	autopilot_interface_quit = &autopilot_interface;
	signal(SIGINT,quit_handler);

	/*
	 * Start the port and autopilot_interface
	 * This is where the port is opened, and read and write threads are started.
	 */
	port->start();
	autopilot_interface.start();


	// --------------------------------------------------------------------------
	//   RUN COMMANDS
	// --------------------------------------------------------------------------

	/*
	 * Now we can implement the algorithm we want on top of the autopilot interface
	 */
	while (true)
	{
		commands(autopilot_interface);
		usleep(10000); // 100 Hz
	}


	// --------------------------------------------------------------------------
	//   THREAD and PORT SHUTDOWN
	// --------------------------------------------------------------------------

	/*
	 * Now that we are done we can stop the threads and close the port
	 */
	autopilot_interface.stop();
	port->stop();

	delete port;

	// --------------------------------------------------------------------------
	//   DONE
	// --------------------------------------------------------------------------

	// woot!
	return 0;

}


// ------------------------------------------------------------------------------
//   COMMANDS
// ------------------------------------------------------------------------------

void
commands(Autopilot_Interface &api)
{

	// copy current messages
	Mavlink_Messages messages = api.current_messages;

	auto imu_db = influxdb::InfluxDBFactory::Get("http://localhost:8086?db=imu_db");
	imu_db->createDatabaseIfNotExists();
	imu_db->write(influxdb::Point{"temperature"}.addTag("category", "imu").addField("value", messages.highres_imu.temperature));
	imu_db->write(influxdb::Point{"xacc"}.addTag("category", "imu").addField("value", messages.highres_imu.xacc));
	imu_db->write(influxdb::Point{"yacc"}.addTag("category", "imu").addField("value", messages.highres_imu.yacc));
	imu_db->write(influxdb::Point{"zacc"}.addTag("category", "imu").addField("value", messages.highres_imu.zacc));
	imu_db->write(influxdb::Point{"xgyro"}.addTag("category", "imu").addField("value", messages.highres_imu.xgyro));
	imu_db->write(influxdb::Point{"ygyro"}.addTag("category", "imu").addField("value", messages.highres_imu.ygyro));
	imu_db->write(influxdb::Point{"zgyro"}.addTag("category", "imu").addField("value", messages.highres_imu.zgyro));
	imu_db->write(influxdb::Point{"xmag"}.addTag("category", "imu").addField("value", messages.highres_imu.xmag));
	imu_db->write(influxdb::Point{"ymag"}.addTag("category", "imu").addField("value", messages.highres_imu.ymag));
	imu_db->write(influxdb::Point{"zmag"}.addTag("category", "imu").addField("value", messages.highres_imu.zmag));
	imu_db->write(influxdb::Point{"abs_pressure"}.addTag("category", "imu").addField("value", messages.highres_imu.abs_pressure));

	auto altitude_db = influxdb::InfluxDBFactory::Get("http://localhost:8086?db=altitude_db");
	altitude_db->createDatabaseIfNotExists();
	altitude_db->write(influxdb::Point{"altitude_local"}.addTag("category", "altitudes").addField("value", messages.altitude.altitude_local));
	altitude_db->write(influxdb::Point{"altitude_relative"}.addTag("category", "altitudes").addField("value", messages.altitude.altitude_relative));
	altitude_db->write(influxdb::Point{"altitude_terrain"}.addTag("category", "altitudes").addField("value", messages.altitude.altitude_terrain));
	altitude_db->write(influxdb::Point{"bottom_clearance"}.addTag("category", "altitudes").addField("value", messages.altitude.bottom_clearance));

	auto attitude_db = influxdb::InfluxDBFactory::Get("http://localhost:8086?db=attitude_db");
	attitude_db->createDatabaseIfNotExists();
	attitude_db->write(influxdb::Point{"roll"}.addTag("category", "attitude").addField("value", messages.attitude.roll));
	attitude_db->write(influxdb::Point{"pitch"}.addTag("category", "attitude").addField("value", messages.attitude.pitch));
	attitude_db->write(influxdb::Point{"yaw"}.addTag("category", "attitude").addField("value", messages.attitude.yaw));
	attitude_db->write(influxdb::Point{"rollspeed"}.addTag("category", "attitude").addField("value", messages.attitude.rollspeed));
	attitude_db->write(influxdb::Point{"pitchspeed"}.addTag("category", "attitude").addField("value", messages.attitude.pitchspeed));
	attitude_db->write(influxdb::Point{"yawspeed"}.addTag("category", "attitude").addField("value", messages.attitude.yawspeed));

	auto battery_db = influxdb::InfluxDBFactory::Get("http://localhost:8086?db=battery_db");
	battery_db->createDatabaseIfNotExists();
	battery_db->write(influxdb::Point{"temperature"}.addTag("category", "battery").addField("value", (double)messages.battery_status.temperature));
	battery_db->write(influxdb::Point{"charge_state"}.addTag("category", "battery").addField("value", (double)messages.battery_status.charge_state));
	battery_db->write(influxdb::Point{"current_battery"}.addTag("category", "battery").addField("value", (double)messages.battery_status.current_battery));

	// auto estimator_db = influxdb::InfluxDBFactory::Get("http://localhost:8086?db=estimator_db");
	// estimator_db->createDatabaseIfNotExists();
	// estimator_db->write(influxdb::Point{"vel_ratio"}.addTag("category", "estimator").addField("value", (double)messages.estimator_status.vel_ratio));
	// estimator_db->write(influxdb::Point{"pos_horiz_ratio"}.addTag("category", "estimator").addField("value", (double)messages.estimator_status.pos_horiz_ratio));
	// estimator_db->write(influxdb::Point{"pos_vert_ratio"}.addTag("category", "estimator").addField("value", (double)messages.estimator_status.pos_vert_ratio));
	// estimator_db->write(influxdb::Point{"mag_ratio"}.addTag("category", "estimator").addField("value", (double)messages.estimator_status.mag_ratio));
	// estimator_db->write(influxdb::Point{"hagl_ratio"}.addTag("category", "estimator").addField("value", (double)messages.estimator_status.hagl_ratio));
	// estimator_db->write(influxdb::Point{"tas_ratio"}.addTag("category", "estimator").addField("value", (double)messages.estimator_status.tas_ratio));
	// estimator_db->write(influxdb::Point{"pos_horiz_accuracy"}.addTag("category", "estimator").addField("value", (double)messages.estimator_status.pos_horiz_accuracy));
	// estimator_db->write(influxdb::Point{"pos_vert_accuracy"}.addTag("category", "estimator").addField("value", (double)messages.estimator_status.pos_vert_accuracy));

	auto odometry_db = influxdb::InfluxDBFactory::Get("http://localhost:8086?db=odometry_db");
	odometry_db->createDatabaseIfNotExists();
	odometry_db->write(influxdb::Point{"x"}.addTag("category", "estimator").addField("value", messages.odometry.x));
	odometry_db->write(influxdb::Point{"y"}.addTag("category", "estimator").addField("value", messages.odometry.y));
	odometry_db->write(influxdb::Point{"z"}.addTag("category", "estimator").addField("value", messages.odometry.z));
	odometry_db->write(influxdb::Point{"vx"}.addTag("category", "estimator").addField("value", messages.odometry.vx));
	odometry_db->write(influxdb::Point{"vy"}.addTag("category", "estimator").addField("value", messages.odometry.vy));
	odometry_db->write(influxdb::Point{"vz"}.addTag("category", "estimator").addField("value", messages.odometry.vz));
	odometry_db->write(influxdb::Point{"rollspeed"}.addTag("category", "estimator").addField("value", messages.odometry.rollspeed));
	odometry_db->write(influxdb::Point{"pitchspeed"}.addTag("category", "estimator").addField("value", messages.odometry.pitchspeed));
	odometry_db->write(influxdb::Point{"yawspeed"}.addTag("category", "estimator").addField("value", messages.odometry.yawspeed));

	auto vibration_db = influxdb::InfluxDBFactory::Get("http://localhost:8086?db=vibration_db");
	vibration_db->createDatabaseIfNotExists();
	vibration_db->write(influxdb::Point{"vibration_x"}.addTag("category", "estimator").addField("value", messages.vibration.vibration_x));
	vibration_db->write(influxdb::Point{"vibration_y"}.addTag("category", "estimator").addField("value", messages.vibration.vibration_y));
	vibration_db->write(influxdb::Point{"vibration_z"}.addTag("category", "estimator").addField("value", messages.vibration.vibration_z));
	vibration_db->write(influxdb::Point{"clipping_0"}.addTag("category", "estimator").addField("value", messages.vibration.clipping_0));
	vibration_db->write(influxdb::Point{"clipping_1"}.addTag("category", "estimator").addField("value", messages.vibration.clipping_1));
	vibration_db->write(influxdb::Point{"clipping_2"}.addTag("category", "estimator").addField("value", messages.vibration.clipping_2));

	auto gps_db = influxdb::InfluxDBFactory::Get("http://localhost:8086?db=gps_db");
	gps_db->createDatabaseIfNotExists();
	gps_db->write(influxdb::Point{"origin_altitude"}.addTag("category", "estimator")
		.addField("altitude", messages.gps_global_origin.altitude)
		.addField("longitude", messages.gps_global_origin.longitude)
		.addField("value", 1)
		.addField("latitude", messages.gps_global_origin.latitude));
	
	gps_db->write(influxdb::Point{"raw_gps"}.addTag("category", "estimator")
		.addField("latitude", messages.global_position_int.lat)
		.addField("longitude", messages.global_position_int.lon)
		.addField("altitude", messages.global_position_int.alt)
		.addField("hdg", messages.global_position_int.hdg)
		.addField("relative_altitude", messages.global_position_int.relative_alt)
		.addField("vx", messages.global_position_int.vx)
		.addField("vy", messages.global_position_int.vy)
		.addField("vz", messages.global_position_int.vz)
		.addField("value", 1)
		.addField("visibles", messages.gps_raw.satellites_visible));

	// --------------------------------------------------------------------------
	//   END OF COMMANDS
	// --------------------------------------------------------------------------

	return;

}


// ------------------------------------------------------------------------------
//   Parse Command Line
// ------------------------------------------------------------------------------
// throws EXIT_FAILURE if could not open the port
void
parse_commandline(int argc, char **argv, char *&uart_name, int &baudrate,
		bool &use_udp, char *&udp_ip, int &udp_port, bool &autotakeoff)
{

	// string for command line usage
	const char *commandline_usage = "usage: mavlink_control [-d <devicename> -b <baudrate>] [-u <udp_ip> -p <udp_port>] [-a ]";

	// Read input arguments
	for (int i = 1; i < argc; i++) { // argv[0] is "mavlink"

		// Help
		if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
			printf("%s\n",commandline_usage);
			throw EXIT_FAILURE;
		}

		// UART device ID
		if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--device") == 0) {
			if (argc > i + 1) {
				i++;
				uart_name = argv[i];
			} else {
				printf("%s\n",commandline_usage);
				throw EXIT_FAILURE;
			}
		}

		// Baud rate
		if (strcmp(argv[i], "-b") == 0 || strcmp(argv[i], "--baud") == 0) {
			if (argc > i + 1) {
				i++;
				baudrate = atoi(argv[i]);
			} else {
				printf("%s\n",commandline_usage);
				throw EXIT_FAILURE;
			}
		}

		// UDP ip
		if (strcmp(argv[i], "-u") == 0 || strcmp(argv[i], "--udp_ip") == 0) {
			if (argc > i + 1) {
				i++;
				udp_ip = argv[i];
				use_udp = true;
			} else {
				printf("%s\n",commandline_usage);
				throw EXIT_FAILURE;
			}
		}

		// UDP port
		if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--port") == 0) {
			if (argc > i + 1) {
				i++;
				udp_port = atoi(argv[i]);
			} else {
				printf("%s\n",commandline_usage);
				throw EXIT_FAILURE;
			}
		}

		// Autotakeoff
		if (strcmp(argv[i], "-a") == 0 || strcmp(argv[i], "--autotakeoff") == 0) {
			autotakeoff = true;
		}

	}
	// end: for each input argument

	// Done!
	return;
}


// ------------------------------------------------------------------------------
//   Quit Signal Handler
// ------------------------------------------------------------------------------
// this function is called when you press Ctrl-C
void
quit_handler( int sig )
{
	printf("\n");
	printf("TERMINATING AT USER REQUEST\n");
	printf("\n");

	// autopilot interface
	try {
		autopilot_interface_quit->handle_quit(sig);
	}
	catch (int error){}

	// port
	try {
		port_quit->stop();
	}
	catch (int error){}

	// end program here
	exit(0);

}


// ------------------------------------------------------------------------------
//   Main
// ------------------------------------------------------------------------------
int
main(int argc, char **argv)
{
	// This program uses throw, wrap one big try/catch here
	try
	{
		int result = top(argc,argv);
		return result;
	}

	catch ( int error )
	{
		fprintf(stderr,"mavlink_control threw exception %i \n" , error);
		return error;
	}

}


