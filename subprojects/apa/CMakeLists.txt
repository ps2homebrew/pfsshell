
cmake_minimum_required(VERSION 3.10)
add_library( apa STATIC
    "_init.c"
    "irx/hdd_fio.c"
    "irx/hdd.c"
    "src/apa.c"
    "src/cache.c"
    "src/journal.c"
    "src/misc.c"
    "src/password.c"
    "src/free.c"
)

include_directories("include")
include_directories("irx")
include_directories("../fakeps2sdk/include")
