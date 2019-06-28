//
// Created by huanghao on 19-5-15.
//

#ifndef HBAUDITFLOW_SERVER_H
#define HBAUDITFLOW_SERVER_H

#include <string>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/asio/ssl.hpp>
#include "talk_to_client.h"
#include "common.h"

using namespace std;
using namespace boost;
using namespace boost::asio;

extern array_clients m_clients;
extern boost::recursive_mutex m_cs;
class CServer
{
public:
    CServer(std::string ip,short port,const asio::ssl::context::method& m);
    //CServer(std::string ip,short port);
    ~CServer();

    void start();
    void handle_accept(client_ptr client, const boost::system::error_code &err);
    void handle_talk_to_client_thread();

    void update_clients_changed();
    void stop_client(client_ptr client);
    void stop_client(int  fd);
    void set_receive_data(void* receivedata);
    void send_msg(int fd,std::string &msg);
    asio::ssl::context& context();
private:
    boost::asio::io_service service;//这个不能放成全局变量，因为要是放成全局变量的化,必须要先有io_service,不能先有CService
    ip::tcp::acceptor m_acceptor;
    boost::recursive_mutex m_clients_cs;
    //array_clients m_clients;
    ReceiveData m_receive_data;
    boost::asio::ssl::context m_content;
};

#endif //HBAUDITFLOW_SERVER_H
