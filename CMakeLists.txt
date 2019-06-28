CMAKE_MINIMUM_REQUIRED(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
PROJECT(hb_asio)
SET(CMAKE_CXX_STANDARD 11)
ADD_COMPILE_OPTIONS(-std=c++11)

AUX_SOURCE_DIRECTORY(../asio SOURCE_FILES)

SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -o0 -Wall -g -ggdb")
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -o3 -Wall ")

SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
SET(CMAKE_INSTALL_RPATH
        "\$ORIGIN/third_party/log4cpp/lib"
        "\$ORIGIN/third_party/openssl/lib"
        "\$ORIGIN/third_party/curl/lib"
        "\$ORIGIN/third_party/boost/lib"
        "\$ORIGIN/third_party/uuid/lib"
        "\$ORIGIN/third_party/SQLiteCpp/lib"
        "\$ORIGIN/third_party/system/lib"
        )
SET(CMAKE_BUILD_WITH_INSTALL_RPATH ON)

INCLUDE_DIRECTORIES(
        ./include
        /usr/local/boost/include
        ../log4/include
        /usr/local/log4cpp/include
        )


SET(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/libs/)

SET(CMAKE_CXX_FLAGS "-fPIC")

ADD_LIBRARY(hb_asio STATIC ${SOURCE_FILES})

TARGET_LINK_LIBRARIES(hb_asio
        boost_thread
        boost_system
        hb_log4
        log4cpp)