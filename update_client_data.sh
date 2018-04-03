#!/bin/bash

# fetch client data from client git
git clone https://github.com/ElitCoder/Kobla-2D-Client.git
mkdir -p bin/client_data/
cp -r Kobla-2D-Client/bin/data/* bin/client_data

# cleanup
rm -rf Kobla-2D-Client