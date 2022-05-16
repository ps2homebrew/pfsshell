#!/bin/bash

for d in */ ; do
	echo "$d ---------------------------------------------------------"
    cd $d
	rm -rf build/
	mkdir  build/
	cd build/
	cmake ..
	cmake --build .
	cd ..
	cd ..
	echo "$d ---------------------------------------------------------"
done

dir -R