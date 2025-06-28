cmake_minimum_required(VERSION 3.15)
project(dualsensitive VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE SOURCE_FILES
    ${PROJECT_SOURCE_DIR}/src/core/*.cpp
    ${PROJECT_SOURCE_DIR}/src/core/udp/*.cpp
    ${PROJECT_SOURCE_DIR}/src/dualsensitive.cpp
)

# Create the static lib
add_library(dualsensitive STATIC ${SOURCE_FILES})

target_include_directories(dualsensitive PUBLIC
    ${PROJECT_SOURCE_DIR}/src/core
    ${PROJECT_SOURCE_DIR}/src/core/udp
    ${PROJECT_SOURCE_DIR}/include
)

# to avoid dllimport conflicts
target_compile_definitions(dualsensitive PUBLIC DS5W_BUILD_LIB)

# link necessary Windows libs
target_link_libraries(dualsensitive
    setupapi
    hid
    cfgmgr32
)

# skip MSVC warnings
if (MSVC)
    target_compile_options(dualsensitive PRIVATE /W4)
else()
    target_compile_options(dualsensitive PRIVATE -Wall -Wextra -pedantic -Werror)
endif()

# solo test exe
add_executable(solo-test test/solo/main.cpp)
target_include_directories(solo-test PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(solo-test PRIVATE dualsensitive)

# server executable (runs the UDP server)
add_executable(dualsensitive-service src/service/main.cpp)
# Mark as Windows GUI app to suppress console window
set_target_properties(dualsensitive-service PROPERTIES
    WIN32_EXECUTABLE TRUE
)
target_link_libraries(dualsensitive-service PRIVATE dualsensitive)
target_include_directories(dualsensitive-service PRIVATE ${PROJECT_SOURCE_DIR}/include)

# client executable (launches server and sends triggers)
add_executable(client test/client/main.cpp)
target_link_libraries(client PRIVATE dualsensitive)
target_include_directories(client PRIVATE ${PROJECT_SOURCE_DIR}/include)
