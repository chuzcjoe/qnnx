set(APP "qnnx-app")
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_FLAGS "-std=c++20 -Wall -Wextra -Wpedantic -Wunused-variable -Wunused-function -Wshadow -Werror -O3")

file(GLOB APP_SOURCES 
    main.cpp
    src/*.cpp
    )

add_executable(${APP} ${APP_SOURCES})

target_include_directories(${APP} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(${APP} qnn log)
