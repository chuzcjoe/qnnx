#==============================================================================
#
#  Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
#  All rights reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#
#==============================================================================

set(APP "qnn-sample-app")
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_FLAGS "-std=c++20 -Wall -Werror -O3")

file(GLOB APP_SOURCES 
    *.cpp
    Log/*.cpp
    PAL/src/linux/*.cpp
    PAL/src/common/*.cpp
    Utils/*.cpp
    WrapperUtils/*.cpp)

add_executable(${APP} ${APP_SOURCES})

include_directories(. Log PAL/include Utils WrapperUtils ../include/QNN)


