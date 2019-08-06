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

class CServer {
public:
#if defined(ASIO_SSL)
    CServer(std::string ip,short port,const asio::ssl::context::method& m);
#elif defined(ASIO_TCP)
    CServer(std::string ip,short port);
#elif defined(ASIO_LOCAL)
CServer(const std::string & filename);
#endif

    //CServer(std::string ip,short port);
    ~CServer();

    void start();

    void stop();

    void handle_accept(client_ptr client, const boost::system::error_code &err);

    void handle_talk_to_client_thread();

    void update_clients_changed();

    void stop_client(client_ptr client);

    void stop_client(int fd);

    void set_receive_data(void *receivedata);

    void send_msg(int fd, std::string &msg);

    void start_listen();

    asio::ssl::context &context();

private:
    boost::asio::io_service service;//这个不能放成全局变量，因为要是放成全局变量的化,必须要先有io_service,不能先有CService
#if defined(ASIO_LOCAL)
    boost::asio::local::stream_protocol::acceptor m_acceptor;
#elif defined(ASIO_SSL)
    boost::asio::ip::tcp::acceptor m_acceptor;
#elif defined(ASIO_TCP)
    boost::asio::ip::tcp::acceptor m_acceptor;
#else
    boost::asio::ip::tcp::acceptor m_acceptor;
#endif

    boost::recursive_mutex m_clients_cs;
    ReceiveData m_receive_data;
#ifdef ASIO_SSL
    boost::asio::ssl::context m_content;
#endif
};

#endif //HBAUDITFLOW_SERVER_H
