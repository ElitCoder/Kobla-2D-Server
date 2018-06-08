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

# cleanup
rm -rf sfml-tmxloader