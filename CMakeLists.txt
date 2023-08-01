cmake_minimum_required(VERSION 3.16)

set(CMAKE_C_STANDARD 11)

set(VER_MAJOR 0)
set(VER_MINOR 2)
set(VER_PATCH 0)
set(VER_STRING ${VER_MAJOR}.${VER_MINOR}.${VER_PATCH})

project(c_can
        VERSION ${VER_STRING}
        DESCRIPTION "CAN  logging in Linux."
        HOMEPAGE_URL github.com/AGCO-EFG/linux-can.com
        LANGUAGES C)

include_directories(include)
add_library(c_can SHARED src/bus.c src/message.c src/io/blf.c src/logger.c)

set_target_properties(c_can PROPERTIES VERSION ${VER_STRING} SOVERSION ${VER_STRING})

# Math library is required
find_library(MATH_LIBRARY m REQUIRED)
if(MATH_LIBRARY)
    target_link_libraries(c_can PUBLIC ${MATH_LIBRARY})
endif()

# If a valid zlib include/ directory is provided, then add the directory to list of included directories
set(ZLIB_INCLUDE_DIR "" CACHE PATH "Path to zlib include directory.")
if ( EXISTS ${ZLIB_INCLUDE_DIR}/zlib.h )
    include_directories(${ZLIB_INCLUDE_DIR})
endif ()
# zlib is required
find_library(ZLIB_LIBRARY z REQUIRED)
if(ZLIB_LIBRARY)
    target_link_libraries(c_can PUBLIC ${ZLIB_LIBRARY})
endif()

# Scripts
add_executable(lcandump scripts/candump.c)
target_link_libraries(lcandump PRIVATE c_can)
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/lcandump DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

add_executable(write_blf scripts/write_blf.c)
target_link_libraries(write_blf PRIVATE c_can)
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/write_blf DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

add_executable(log scripts/log.c)
target_link_libraries(log PRIVATE c_can)
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/log DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

add_executable(r_logger scripts/r_logger.c)
target_link_libraries(r_logger PRIVATE c_can)
set_target_properties(c_can PROPERTIES RUNTIME r_logger)
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/r_logger DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

install(TARGETS c_can
        LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
)
