project(main)
cmake_minimum_required(VERSION 3.10)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_BUILD_TYPE DEBUG)
# file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/subscriber.xml
#     ${CMAKE_CURRENT_SOURCE_DIR}/publisher.xml
#     DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
#     )
# 添加XML宏定义
add_definitions(-DXML)

add_library(
DDSBASE
SHARED
DataDefinePubSubTypes.cxx
DataDefine.cxx
Publisher.cpp
Subscriber.cpp
Discover.cpp
)
link_directories(/usr/local/lib)
add_executable(${PROJECT_NAME} Main.cpp)
target_link_libraries(${PROJECT_NAME} DDSBASE pthread libfastrtps.so libfastcdr.so)