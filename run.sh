#!/bin/bash

if [ $# -ne 0 ]; then
	make clean
fi

make -j 5

if [ $? -ne 0 ]; then
	exit 1
fi

cd bin/
./Kobla-2D-Server-Rebased
cd ../