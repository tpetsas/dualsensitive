cmake_minimum_required(VERSION 3.15)
project(dualsense VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE SOURCE_FILES
    ${PROJECT_SOURCE_DIR}/src/*.cpp
    ${PROJECT_SOURCE_DIR}/src/*.h
)

# Create the static lib
add_library(dualsense STATIC ${SOURCE_FILES})

target_include_directories(dualsense PUBLIC
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/include
)

# to avoid dllimport conflicts
target_compile_definitions(dualsense PUBLIC DS5W_BUILD_LIB)

# link necessary Windows libs
target_link_libraries(dualsense
    setupapi
    hid
    cfgmgr32
)

# skip MSVC warnings
if (MSVC)
    target_compile_options(dualsense PRIVATE /W4)
else()
    target_compile_options(dualsense PRIVATE -Wall -Wextra -pedantic -Werror)
endif()

# solo test exe
add_executable(solo-test test/solo/main.cpp)
target_include_directories(solo-test PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(solo-test PRIVATE dualsense)

# server executable (runs the UDP server)
add_executable(dualsense-service test/server/main.cpp)
# Mark as Windows GUI app to suppress console window
set_target_properties(dualsense-service PROPERTIES
    WIN32_EXECUTABLE TRUE
)
target_link_libraries(dualsense-service PRIVATE dualsense)
target_include_directories(dualsense-service PRIVATE ${PROJECT_SOURCE_DIR}/include)

# client executable (launches server and sends triggers)
add_executable(client test/client/main.cpp)
target_link_libraries(client PRIVATE dualsense)
target_include_directories(client PRIVATE ${PROJECT_SOURCE_DIR}/include)

