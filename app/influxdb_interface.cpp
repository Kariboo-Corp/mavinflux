/**
 * @author Julien Geneste
 * @brief pushData into an influx DB.
 * 
 * 08/12/2022 - v.1
*/

#include "influxdb_interface.h"

InfluxDB_Interface::InfluxDB_Interface(std::string server_addr, int port)
{
    this->port = port;
    this->server_addr = server_addr;
}

InfluxDB_Interface::~InfluxDB_Interface()
{
    ;
}

void InfluxDB_Interface::init() 
{
    try
    {
        for (int i = 0; i < 6; i++)
        {
            this->influx[i] = influxdb::InfluxDBFactory::Get("http://" + this->server_addr + ":" + std::to_string(port) + "?db=" + this->databases[i]);
            this->influx[i]->createDatabaseIfNotExists();
        }
    }
    catch(const std::exception& e)
    {
        printf("[ERROR] Unable to initialise InfluxDB connexion ...\n");
    }

    return;
    
}

void InfluxDB_Interface::pushData(Mavlink_Messages messages)
{

    try
    {
        this->influx[INFLUX_IMU_DB]->write(influxdb::Point{"temperature"}.addTag("category", "imu").addField("value", messages.highres_imu.temperature));
        this->influx[INFLUX_IMU_DB]->write(influxdb::Point{"xacc"}.addTag("category", "imu").addField("value", messages.highres_imu.xacc));
        this->influx[INFLUX_IMU_DB]->write(influxdb::Point{"yacc"}.addTag("category", "imu").addField("value", messages.highres_imu.yacc));
        this->influx[INFLUX_IMU_DB]->write(influxdb::Point{"zacc"}.addTag("category", "imu").addField("value", messages.highres_imu.zacc));
        this->influx[INFLUX_IMU_DB]->write(influxdb::Point{"xgyro"}.addTag("category", "imu").addField("value", messages.highres_imu.xgyro));
        this->influx[INFLUX_IMU_DB]->write(influxdb::Point{"ygyro"}.addTag("category", "imu").addField("value", messages.highres_imu.ygyro));
        this->influx[INFLUX_IMU_DB]->write(influxdb::Point{"zgyro"}.addTag("category", "imu").addField("value", messages.highres_imu.zgyro));
        this->influx[INFLUX_IMU_DB]->write(influxdb::Point{"xmag"}.addTag("category", "imu").addField("value", messages.highres_imu.xmag));
        this->influx[INFLUX_IMU_DB]->write(influxdb::Point{"ymag"}.addTag("category", "imu").addField("value", messages.highres_imu.ymag));
        this->influx[INFLUX_IMU_DB]->write(influxdb::Point{"zmag"}.addTag("category", "imu").addField("value", messages.highres_imu.zmag));
        this->influx[INFLUX_IMU_DB]->write(influxdb::Point{"abs_pressure"}.addTag("category", "imu").addField("value", messages.highres_imu.abs_pressure));
    }
    catch(const std::exception& e)
    {
        printf("[ERROR] Can't push imu data. Dropping record.\n");
    }
    try
    {
        this->influx[INFLUX_ALTITUDE_DB]->write(influxdb::Point{"altitude_local"}.addTag("category", "altitudes").addField("value", messages.altitude.altitude_local));
        this->influx[INFLUX_ALTITUDE_DB]->write(influxdb::Point{"altitude_relative"}.addTag("category", "altitudes").addField("value", messages.altitude.altitude_relative));
        this->influx[INFLUX_ALTITUDE_DB]->write(influxdb::Point{"altitude_terrain"}.addTag("category", "altitudes").addField("value", messages.altitude.altitude_terrain));
        this->influx[INFLUX_ALTITUDE_DB]->write(influxdb::Point{"bottom_clearance"}.addTag("category", "altitudes").addField("value", messages.altitude.bottom_clearance));
    }
    catch(const std::exception& e)
    {
        printf("[ERROR] Can't push altitude data. Dropping record.\n");
    }
    try
    {
        this->influx[INFLUX_ATTITUDE_DB]->write(influxdb::Point{"roll"}.addTag("category", "attitude").addField("value", messages.attitude.roll));
        this->influx[INFLUX_ATTITUDE_DB]->write(influxdb::Point{"pitch"}.addTag("category", "attitude").addField("value", messages.attitude.pitch));
        this->influx[INFLUX_ATTITUDE_DB]->write(influxdb::Point{"yaw"}.addTag("category", "attitude").addField("value", messages.attitude.yaw));
        this->influx[INFLUX_ATTITUDE_DB]->write(influxdb::Point{"rollspeed"}.addTag("category", "attitude").addField("value", messages.attitude.rollspeed));
        this->influx[INFLUX_ATTITUDE_DB]->write(influxdb::Point{"pitchspeed"}.addTag("category", "attitude").addField("value", messages.attitude.pitchspeed));
        this->influx[INFLUX_ATTITUDE_DB]->write(influxdb::Point{"yawspeed"}.addTag("category", "attitude").addField("value", messages.attitude.yawspeed));
    }
    catch(const std::exception& e)
    {
        printf("[ERROR] Can't push attitude data. Dropping record.\n");
    }
    try
    {
        this->influx[INFLUX_BATTERY_DB]->write(influxdb::Point{"temperature"}.addTag("category", "battery").addField("value", (double)messages.battery_status.temperature));
        this->influx[INFLUX_BATTERY_DB]->write(influxdb::Point{"charge_state"}.addTag("category", "battery").addField("value", (double)messages.battery_status.charge_state));
        this->influx[INFLUX_BATTERY_DB]->write(influxdb::Point{"current_battery"}.addTag("category", "battery").addField("value", (double)messages.battery_status.current_battery));
    }
    catch(const std::exception& e)
    {
        printf("[ERROR] Can't push batterie data. Dropping record.\n");
    }
    try
    {
        this->influx[INFLUX_ODOMETRY_DB]->write(influxdb::Point{"x"}.addTag("category", "estimator").addField("value", messages.odometry.x));
        this->influx[INFLUX_ODOMETRY_DB]->write(influxdb::Point{"y"}.addTag("category", "estimator").addField("value", messages.odometry.y));
        this->influx[INFLUX_ODOMETRY_DB]->write(influxdb::Point{"z"}.addTag("category", "estimator").addField("value", messages.odometry.z));
        this->influx[INFLUX_ODOMETRY_DB]->write(influxdb::Point{"vx"}.addTag("category", "estimator").addField("value", messages.odometry.vx));
        this->influx[INFLUX_ODOMETRY_DB]->write(influxdb::Point{"vy"}.addTag("category", "estimator").addField("value", messages.odometry.vy));
        this->influx[INFLUX_ODOMETRY_DB]->write(influxdb::Point{"vz"}.addTag("category", "estimator").addField("value", messages.odometry.vz));
        this->influx[INFLUX_ODOMETRY_DB]->write(influxdb::Point{"rollspeed"}.addTag("category", "estimator").addField("value", messages.odometry.rollspeed));
        this->influx[INFLUX_ODOMETRY_DB]->write(influxdb::Point{"pitchspeed"}.addTag("category", "estimator").addField("value", messages.odometry.pitchspeed));
        this->influx[INFLUX_ODOMETRY_DB]->write(influxdb::Point{"yawspeed"}.addTag("category", "estimator").addField("value", messages.odometry.yawspeed));
    }
    catch(const std::exception& e)
    {
        printf("[ERROR] Can't push odometry data. Dropping record.\n");
    }
    try
    {
        this->influx[INFLUX_VIBRATION_DB]->write(influxdb::Point{"vibration_x"}.addTag("category", "estimator").addField("value", messages.vibration.vibration_x));
        this->influx[INFLUX_VIBRATION_DB]->write(influxdb::Point{"vibration_y"}.addTag("category", "estimator").addField("value", messages.vibration.vibration_y));
        this->influx[INFLUX_VIBRATION_DB]->write(influxdb::Point{"vibration_z"}.addTag("category", "estimator").addField("value", messages.vibration.vibration_z));
        this->influx[INFLUX_VIBRATION_DB]->write(influxdb::Point{"clipping_0"}.addTag("category", "estimator").addField("value", messages.vibration.clipping_0));
        this->influx[INFLUX_VIBRATION_DB]->write(influxdb::Point{"clipping_1"}.addTag("category", "estimator").addField("value", messages.vibration.clipping_1));
        this->influx[INFLUX_VIBRATION_DB]->write(influxdb::Point{"clipping_2"}.addTag("category", "estimator").addField("value", messages.vibration.clipping_2));

    }
    catch(const std::exception& e)
    {
        printf("[ERROR] Can't push vibrations data. Dropping record.\n");
    }
    
    try 
    {
        if (messages.gps_raw.satellites_visible > 0)
        {

            double alt = messages.gps_raw.alt / 1000.00;
            double lat = messages.gps_raw.lat / 10000000.00;
            double lon = messages.gps_raw.lon / 10000000.00;
            
            this->influx[INFLUX_ATTITUDE_DB]->write(influxdb::Point{"position"}.addTag("category", "estimator")
            .addField("latitude", lat)
            .addField("longitude", lon)
            .addField("altitude", alt));

        }
    }
    catch(const std::exception& e)
    {
        printf("[ERROR] Can't push gps data. Dropping record.\n");
    }

    return;
}