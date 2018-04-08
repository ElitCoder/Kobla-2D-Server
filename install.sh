#!/bin/bash

# get number of cores
cores=`grep --count ^processor /proc/cpuinfo`

# install dependencies
sudo apt-get update && sudo apt-get install g++ libsfml-dev cmake zlib1g-dev

# install sfml-tmxloader (own fork with map animations)
git clone https://github.com/ElitCoder/sfml-tmxloader.git
cd sfml-tmxloader/
./install.sh
cd ../

# move libs to PATH
sudo cp /usr/local/lib/libtmx-loader.so /usr/lib/
sudo cp /usr/local/lib/libpugi.so /usr/lib/

# cleanup
rm -rf sfml-tmxloader