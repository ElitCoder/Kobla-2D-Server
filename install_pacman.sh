#!/bin/bash

# get number of cores
cores=`grep --count ^processor /proc/cpuinfo`

# install dependencies
sudo pacman -Syy && sudo pacman -S --needed gcc sfml

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