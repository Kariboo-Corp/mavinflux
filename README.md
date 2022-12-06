MAVLink to InfluxDB
========================

This program is used to convert MAVLink serial messages to MQTT publisher to create a live view of parameters from the copter.

Dependances
============

You need to install [InfluxDB-cxx libraries](https://github.com/offa/influxdb-cxx).


Building
========

```
$ cd mavinflux/
$ make
```

Hardware Setup
=========

Connect the USB to serial cable to your Pixhawk / SiK Radio.  

Also Note: Using a UART (serial) connection should be preferred over using the USB port for flying systems.  The reason being that the driver for the USB port is much more complicated, so the UART is a much more trusted port for flight-critical functions.  To learn how this works though the USB port will be fine and instructive.

Execution
=========

You have to pick a port name, try searching for it with 
```bash
ls /dev/ttyACM* 
ls /dev/ttyUSB*
```

Alternatively, plug in Pixhawk USB cable again and issue the command:

```bash
dmesg
```
The device described at the bottom of dmesg's output will be the port on which the Pixhawk is mounted. 

The Pixhawk USB port will show up on a `ttyACM*`, an FTDI cable will show up on a `ttyUSB*`.


Run the example executable on the host shell:

```bash
cd mavlinflux/
 ./mavlinflux -d /dev/ttyACM0
```

To stop the program, use the key sequence `Ctrl-C`.

There is also the possibility to connect this example to the simulator using:

```
$ ./mavlinflux -u 127.0.0.1 -a
```
The -a argument enables arming, takeoff and landing of the copter. Use this argument with care on a real copter!