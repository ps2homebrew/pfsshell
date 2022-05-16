cmake_minimum_required(VERSION 3.10)
project(pfsshell)

include_directories(include)

add_library(pfsshell SHARED 
	"startup.c"
	"hl.c"
	"util.c"
	"shell.c"
)

target_link_libraries(pfsshell PUBLIC 
	"${PROJECT_SOURCE_DIR}/subprojects/apa/build/libapa.a"
	"${PROJECT_SOURCE_DIR}/subprojects/iomanX/build/libiomanX.a"
	"${PROJECT_SOURCE_DIR}/subprojects/pfs/build/libpfs.a"
	"${PROJECT_SOURCE_DIR}/subprojects/fakeps2sdk/build/libfakeps2sdk.a"
	"${PROJECT_SOURCE_DIR}/subprojects/hdlfs/build/libhdlfs.a"
)
message(STATUS "EEEEEEEEEEEEEEEEE ${PROJECT_SOURCE_DIR}")
message(STATUS "EEEEEEEEEEEEEEEEE ${PROJECT_BINARY_DIR}")
add_subdirectory(${PROJECT_SOURCE_DIR}/subprojects/apa 			${PROJECT_SOURCE_DIR}/subprojects/apa/build/)
add_subdirectory(${PROJECT_SOURCE_DIR}/subprojects/iomanX 		${PROJECT_SOURCE_DIR}/subprojects/iomanX/build/)
add_subdirectory(${PROJECT_SOURCE_DIR}/subprojects/pfs 			${PROJECT_SOURCE_DIR}/subprojects/pfs/build/)
add_subdirectory(${PROJECT_SOURCE_DIR}/subprojects/fakeps2sdk 	${PROJECT_SOURCE_DIR}/subprojects/fakeps2sdk/build/)
add_subdirectory(${PROJECT_SOURCE_DIR}/subprojects/hdlfs 		${PROJECT_SOURCE_DIR}/subprojects/hdlfs/build/)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")