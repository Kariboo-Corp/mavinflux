/**
 * @author Julien Geneste
 * @brief pushData mavlink messages into an influx DB.
 * 
 * 08/12/2022 - v.1
*/
#ifndef INFLUXDB_INTERFACE_H
#define INFLUXDB_INTERFACE_H

#include <string>
#include <InfluxDBFactory.h>
#include "autopilot_interface.h"

#define INFLUX_IMU_DB 0
#define INFLUX_ALTITUDE_DB 1
#define INFLUX_ATTITUDE_DB 2
#define INFLUX_BATTERY_DB  3
#define INFLUX_ODOMETRY_DB 4
#define INFLUX_VIBRATION_DB 5
#define INFLUX_GPS_DB 6


class InfluxDB_Interface
{
private:
    int port;
    std::string server_addr;

    std::string databases[7] = {
        "imu_db",
        "altitude_db",
        "attitude_db",
        "battery_db",
        "odometry_db",
        "vibration_db",
        "gps_db"
    };

    std::unique_ptr<influxdb::InfluxDB> influx[6];

public:
    InfluxDB_Interface(std::string server_addr, int port);
    ~InfluxDB_Interface();

    void init();
    void pushData(Mavlink_Messages messages);
};


#endif