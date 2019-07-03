//
// Created by huanghao on 19-5-21.
//

#ifndef HBAUDITFLOW_COMMON_H
#define HBAUDITFLOW_COMMON_H

#include <boost/asio/ssl.hpp>

//#define MAX_MSG_NUM 2048
#define THREAD_NUM 1
#define MAX_MSG 2048
//#define ASIO_SSL
#define LEN_BODY
//#define FIX_LEN

class CTalk_to_client;
typedef boost::shared_ptr<CTalk_to_client> client_ptr;
//定义一个回调函数将数据传输到应用层
typedef void(*ReceiveData)(std::string & message,int size,int fd);

class CTalk_to_server;
typedef boost::shared_ptr<CTalk_to_server>  talk_to_server_ptr;


typedef std::vector<client_ptr> array_clients;

typedef boost::asio::ssl::stream<boost::asio::ip::tcp::socket> ssl_socket;

#endif //HBAUDITFLOW_COMMON_H
