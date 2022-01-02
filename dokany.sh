#!/bin/bash
rm -rf external/dokany/dokan_fuse/build
mkdir external/dokany/dokan_fuse/build
cmake -G 'MinGW Makefiles' -Sexternal/dokany/dokan_fuse -Bexternal/dokany/dokan_fuse/build -DCMAKE_SHARED_LINKER_FLAGS="-static -static-libgcc -static-libstdc++" -DCMAKE_INSTALL_PREFIX='./build'
mingw32-make -C external/dokany/dokan_fuse/build
cp -f external/dokany/dokan_fuse/build/libdokanfuse*.dll build/
