//
// Created by huanghao on 19-5-15.
//

#ifndef HBAUDITFLOW_CTALK_TO_SERVER_H
#define HBAUDITFLOW_CTALK_TO_SERVER_H

#include <string>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <vector>
#include <queue>
#include "common.h"
#include "a_timer.h"


using namespace std;
using namespace boost;
using namespace boost::asio;


class CTalk_to_server : public boost::enable_shared_from_this<CTalk_to_server>, boost::noncopyable {
public:
    CTalk_to_server(boost::asio::io_service &ios, boost::asio::ssl::context &m,boost::asio::ip::tcp::resolver::iterator endpoint_iterator);

    ~CTalk_to_server();

    static talk_to_server_ptr
    create_client(boost::asio::ip::tcp::endpoint ep, boost::asio::io_service &ios, boost::asio::ssl::context &m,boost::asio::ip::tcp::resolver::iterator endpoint_iterator);

    void start(boost::asio::ip::tcp::endpoint ep);

    void handle_connect(const boost::system::error_code &err);

    void stop();

    bool started() const;

    void do_read();

    void do_write(const std::string &msg);

    void handle_write(const boost::system::error_code &err, size_t bytes);

    void handle_read(const boost::system::error_code &err, size_t bytes);

    void set_receive_data(void *receivedata);

    void handle_handshake(const boost::system::error_code &error);

    void close();

    void send_msg();

    //void start_listen();
    //void handle_talk_to_server_thread();
    void start_timer(const boost::system::error_code &err);

    static void* write_func(void *arg);
    int read_completion(const boost::system::error_code& ec, size_t bytes_transferred);
    int get_read_count();
private:
    boost::asio::io_service &m_service;
#ifdef ASIO_SSL
    ssl_socket m_socket;
#else
    boost::asio::ip::tcp::socket m_socket;
#endif
    deadline_timer m_timer;
    boost::asio::ip::tcp::endpoint m_ep;
    bool m_bStart;
    char m_read_buffer[MAX_MSG];
    char m_write_buffer[MAX_MSG];
    ReceiveData m_receive_data;
    boost::asio::ip::tcp::resolver::iterator m_endpoint_iterator;
    queue<string> m_vSendMsg;
    int m_write_count;
    int m_send_count;
    int m_read_count;
    char m_read_buf[MAX_MSG];
    size_t headlen;
};

#endif //HBAUDITFLOW_CTALK_TO_SERVER_H
