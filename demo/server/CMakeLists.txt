CMAKE_MINIMUM_REQUIRED(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
PROJECT(server)
SET(CMAKE_CXX_STANDARD 11)
ADD_COMPILE_OPTIONS(-std=c++11)
AUX_SOURCE_DIRECTORY(../server/ SOURCE_FILES)
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -o0 -Wall -g -ggdb")
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -o3 -Wall ")
INCLUDE_DIRECTORIES(
        ../../basemode/asio/include
        ../../basemode/log4/include
        /usr/local/log4cpp/include/
        /usr/local/boost/include/
        /usr/local/openssl/include/
)
LINK_DIRECTORIES(
        ../../basemode/asio/libs/
        ../../basemode/log4/libs/
        /usr/local/log4cpp/lib
        /usr/local/boost/lib/
        /usr/local/openssl/lib/
)

#指的是当前处理的 CMakeLists.txt 所在的路径
SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../bin)
#SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/build)
#生成可执行文件路径
ADD_EXECUTABLE(server ${SOURCE_FILES})

TARGET_LINK_LIBRARIES(server
        hb_asio
        hb_log4
        log4cpp
        boost_thread
        boost_system
        crypto
        ssl
        dl
        )