all: git_submodule mavlink_control

mavlink_control: mavinflux.cpp app/serial_port.cpp app/udp_port.cpp app/autopilot_interface.cpp app/influxdb_interface.cpp
	g++ -std=c++17 -g -Wall -I lib/mavlink/ -I lib/ -L lib/ mavinflux.cpp app/serial_port.cpp app/udp_port.cpp app/autopilot_interface.cpp app/influxdb_interface.cpp -o mavinflux -lpthread -linfluxdb

git_submodule:
	git submodule update --init --recursive

clean:
	 rm -rf *o mavlink
