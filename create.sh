#!/bin/bash

function build {
	if [ $# -ne 0 ]; then
		make clean
	fi
	
	make -j 5
	
	if [ $? -ne 0 ]; then
		exit 1
	fi
}

if [ $# -eq 0 ]; then
	build
else
	if [ $1 = "clean" ]; then
		build clean
	elif [ $1 = "run" ]; then
		build
		
		cd bin/
		./Kobla-2D-Server-Rebased
		cd ../
	fi
fi