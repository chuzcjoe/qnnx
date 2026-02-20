set(APP "qnnx-sample-app")
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_FLAGS "-std=c++20 -Wall -Werror -O3")

file(GLOB APP_SOURCES 
    main.cpp
    )

add_executable(${APP} ${APP_SOURCES})

include_directories(include ../include/QNN)
