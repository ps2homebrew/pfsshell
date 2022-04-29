
cmake_minimum_required(VERSION 3.10)
add_library( hdlfs STATIC
    "_init.c"
    "main.c"
)

include_directories("../fakeps2sdk/include")
