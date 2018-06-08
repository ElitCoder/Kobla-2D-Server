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

# cleanup
rm -rf sfml-tmxloader