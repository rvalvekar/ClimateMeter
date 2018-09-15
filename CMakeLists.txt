cmake_minimum_required(VERSION 2.8.9)
project(ClimateMeter)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

# check c++11 support
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
    message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

OPTION(ENABLE_OPENSSL "Enable use of OpenSSL" ON)
OPTION(DAEMON "Build thinger client as daemon" OFF)
OPTION(EDISON "Enable build and install for Intel Edison" OFF)
OPTION(RASPBERRY "Enable build and isntall for Raspberry Pi" ON)

# Find OpenSSL
IF(ENABLE_OPENSSL)
  find_package(OpenSSL REQUIRED)
      if(OPENSSL_FOUND)
          SET(OPEN_SSL 1)
          include_directories(${OPENSSL_INCLUDE_DIR})
          list(APPEND ADDITIONAL_LIBS ${OPENSSL_LIBRARIES})
          message(STATUS "OpenSSL Version: ${OPENSSL_VERSION}")
      else()
          SET(OPEN_SSL 0)
      endif()
 endif()


set(SOURCE_FILES main.cpp)

# set OpenSSL if available
add_definitions( -DOPEN_SSL=${OPEN_SSL} )

# Support for WiringPi on Raspberry
#if(RASPBERRY)
    #find_package(WiringPi)
   # if(WiringPi_FOUND)
   #     include_directories(${WiringPi_INCLUDE_DIR})
   #     list(APPEND ADDITIONAL_LIBS ${WiringPi_LIBRARIES})
  #  endif()
#endif()

add_executable(ClimateMeter ${SOURCE_FILES})
target_link_libraries(ClimateMeter ${ADDITIONAL_LIBS} -lwiringPi)
set_target_properties(ClimateMeter PROPERTIES COMPILE_DEFINITIONS "DAEMON=0")

