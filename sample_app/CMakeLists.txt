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

target_include_directories(${APP} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Log
    ${CMAKE_CURRENT_SOURCE_DIR}/PAL/include
    ${CMAKE_CURRENT_SOURCE_DIR}/Utils
    ${CMAKE_CURRENT_SOURCE_DIR}/WrapperUtils
)

target_link_libraries(${APP} qnn)


