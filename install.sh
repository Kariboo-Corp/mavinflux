#! /bin/bash
sudo apt install curl cmake libcurl4 libcurl4-openssl-dev libboost1.74-all-dev influxdb -y
cd ~
wget https://github.com/libcpr/cpr/archive/refs/tags/1.10.1.zip
unzip 1.10.1.zip
cd cpr-1.10.1/
mkdir build && cd build
cmake .. -DCPR_USE_SYSTEM_CURL=ON
cmake --build .
sudo cmake --install .
cd ~
git clone https://github.com/offa/influxdb-cxx
cd ~/influxdb-cxx/
mkdir build && cd build
cmake -D INFLUXCXX_TESTING:BOOL=OFF ..
sudo make install
cd ~
rm -rf ~/influxdb-cxx
rm -rf ~/cpr-1.10.1 && rm -rf 1.10.1.zip