CMAKE_MINIMUM_REQUIRED(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
PROJECT(tranfile_client)
SET(CMAKE_CXX_STANDARD 11)
ADD_COMPILE_OPTIONS(-std=c++11)
AUX_SOURCE_DIRECTORY(. SOURCE_FILES)
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -o0 -Wall -g -ggdb")
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -o3 -Wall ")

INCLUDE_DIRECTORIES(
        ../../basemode/asio/include
        ../../basemode/log4/include
        /usr/local/log4cpp/include/
        /usr/local/boost/include/
        /usr/local/openssl/include/
        /usr/local/msgpack/include/
)

LINK_DIRECTORIES(
        ../../basemode/asio/libs/
        ../../basemode/log4/libs/
        /usr/local/log4cpp/lib/
        /usr/local/boost/lib/
        /usr/local/openssl/lib/
        /usr/local/msgpack/lib/
)

#指的是当前处理的 CMakeLists.txt 所在的路径
SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../bin)
#生成可执行文件路径
ADD_EXECUTABLE(tranfile_client ${SOURCE_FILES})

TARGET_LINK_LIBRARIES(tranfile_client
        hb_asio
        hb_log4
        boost_system
        boost_thread
        msgpackc
        crypto
        ssl
        dl
        )