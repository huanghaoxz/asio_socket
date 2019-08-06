//
// Created by huanghao on 19-5-13.
//

#ifndef HBAUDITFLOW_CTALK_TO_CLIENT_H
#define HBAUDITFLOW_CTALK_TO_CLIENT_H

#include <string>
#include <vector>
#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <queue>
#include "common.h"
#include "packer.h"


using namespace std;
using namespace boost;
using namespace boost::asio;


class CTalk_to_client : public boost::enable_shared_from_this<CTalk_to_client>, boost::noncopyable {
public:
#if defined(ASIO_SSL)
    CTalk_to_client(boost::asio::io_service &service, boost::asio::ssl::context &m);
#elif defined(ASIO_TCP)
    CTalk_to_client(boost::asio::io_service &service);
#elif defined(ASIO_LOCAL)
    CTalk_to_client(boost::asio::io_service &service);
#endif

    ~CTalk_to_client();

#if defined(ASIO_SSL)
    static client_ptr new_client(boost::asio::io_service &service, asio::ssl::context &m);
#elif defined(ASIO_TCP)
    static client_ptr new_client(boost::asio::io_service &service);
#elif defined(ASIO_LOCAL)
    static client_ptr new_client(boost::asio::io_service &service);
#endif

    void start();

    void stop();

    bool started();

    void do_read();

    void handle_read(const boost::system::error_code &err, size_t bytes);


    void do_write(std::string &messsage);

    void handle_write(const boost::system::error_code &err, size_t bytes);


    //boost::asio::ip::tcp::socket &get_socket();
#if defined(ASIO_SSL)
    ssl_socket::lowest_layer_type &get_socket();
#elif defined(ASIO_TCP)
    boost::asio::ip::tcp::socket &get_socket();
#elif defined(ASIO_LOCAL)
    local_socket &get_socket();
#endif

    void set_client_changed();

    void set_receive_data(void *receivedata);

    static int clientnum;

    void del_client();

    void handle_handshake(const boost::system::error_code &error);

    void close();

    static void *threadFunc(void *arg);

    int read_completion(const boost::system::error_code &ec, size_t bytes);

    int get_native_fd();

private:
#if defined(ASIO_SSL)
    ssl_socket m_socket;
#elif defined(ASIO_TCP)
    boost::asio::ip::tcp::socket m_socket;
#elif defined(ASIO_LOCAL)
    local_socket m_socket;
#else
#endif
    bool m_bStart;
    ReceiveData m_receive_data;
    boost::asio::ip::tcp::endpoint m_ep;
    bool m_client_changed;
    int m_read_count;
    size_t headlen;
    char m_read_buf[MAX_MSG];
};

#endif //HBAUDITFLOW_CTALK_TO_CLIENT_H
