
cmake_minimum_required(VERSION 3.10)
add_library( fakeps2sdk STATIC
    thsemap.c
    atad.c
)

include_directories(include)
