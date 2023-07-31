cmake_minimum_required(VERSION 3.16)

set(CMAKE_C_STANDARD 11)

set(VER_MAJOR 0)
set(VER_MINOR 1)
set(VER_PATCH 1)
set(VER_STRING ${VER_MAJOR}.${VER_MINOR}.${VER_PATCH})

project(c_can
        VERSION ${VER_STRING}
        DESCRIPTION "CAN  logging in Linux."
        HOMEPAGE_URL github.com/diligentcode/c-can
        LANGUAGES C)

include_directories(include)
add_library(c_can SHARED src/bus.c src/message.c src/blf.c src/logger.c)

set_target_properties(c_can PROPERTIES VERSION ${VER_STRING} SOVERSION ${VER_STRING})

# Math library is required
find_library(MATH_LIBRARY m REQUIRED)
if(MATH_LIBRARY)
    target_link_libraries(c_can PUBLIC ${MATH_LIBRARY})
endif()

# zlib is required
find_library(ZLIB_LIBRARY z REQUIRED)
if(ZLIB_LIBRARY)
    target_link_libraries(c_can PUBLIC ${ZLIB_LIBRARY})
endif()

# Scripts
add_executable(lcandump scripts/candump.c)
target_link_libraries(lcandump PRIVATE c_can)

add_executable(write_blf scripts/write_blf.c)
target_link_libraries(write_blf PRIVATE c_can)

add_executable(log scripts/log.c)
target_link_libraries(log PRIVATE c_can)

add_executable(r_logger scripts/r_logger.c)
target_link_libraries(r_logger PRIVATE c_can)
