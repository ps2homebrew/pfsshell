
cmake_minimum_required(VERSION 3.10)
add_library( pfs STATIC
    "_init.c"
    "irx/pfs_fio.c"
    "irx/pfs_fioctl.c"
    "irx/pfs.c"
    "src/bitmap.c"
    "src/block.c"
    "src/blockWrite.c"
    "src/cache.c"
    "src/dir.c"
    "src/inode.c"
    "src/journal.c"
    "src/misc.c"
    "src/super.c"
    "src/superWrite.c"
)

include_directories("../fakeps2sdk/include")
include_directories("include")
include_directories("irx")
