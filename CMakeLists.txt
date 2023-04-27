cmake_minimum_required(VERSION 3.2)

project(ws_and_http)

set(CMAKE_CXX_STANDARD 11)

if (WIN32)

else()
set(CMAKE_CXX_FLAGS " -std=c++11  -Wformat  -Wall ")
endif()


#set(CMAKE_NO_SYSTEM_FROM_IMPORTED true)
include_directories(
    include
)


add_executable(${PROJECT_NAME}_demo
    src/main.cpp
)
target_link_libraries(${PROJECT_NAME}_demo
    ${catkin_LIBRARIES}
    curl
    boost_system
    pthread
 )
