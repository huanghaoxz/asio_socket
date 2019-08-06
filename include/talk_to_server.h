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
#include "packer.h"
#include <deque>

using namespace std;
using namespace boost;
using namespace boost::asio;
//1.io_service 同操作系统的输入输出服务进行交互
//2.通常一个io_service的实例就足够了
//3.创建连接地址和端口
//4.创建socket

class CTalk_to_server : public boost::enable_shared_from_this<CTalk_to_server>, boost::noncopyable {
public:
#if defined(ASIO_SSL)

    CTalk_to_server(boost::asio::io_service &ios, boost::asio::ssl::context &m,
                    boost::asio::ip::tcp::resolver::iterator endpoint_iterator);

#elif defined(ASIO_TCP)
    CTalk_to_server(boost::asio::io_service &ios,
                    boost::asio::ip::tcp::resolver::iterator endpoint_iterator);
#elif defined(ASIO_LOCAL)
    CTalk_to_server(boost::asio::io_service &ios,const std::string filename);
#endif

    ~CTalk_to_server();

#if defined(ASIO_SSL)

    static talk_to_server_ptr
    create_client(boost::asio::ip::tcp::endpoint ep, boost::asio::io_service &ios, boost::asio::ssl::context &m,
                  boost::asio::ip::tcp::resolver::iterator endpoint_iterator);
    void start(boost::asio::ip::tcp::endpoint ep);

#elif defined(ASIO_TCP)
    static talk_to_server_ptr create_client(boost::asio::ip::tcp::endpoint
                                     ep,
                                     boost::asio::io_service &ios, boost::asio::ip::tcp::resolver::iterator
                                     endpoint_iterator);
    void start(boost::asio::ip::tcp::endpoint ep);

#elif defined(ASIO_LOCAL)
    static talk_to_server_ptr create_client(boost::asio::io_service &ios,const std::string filename);
    void start();
#else
#endif



    void handle_connect(const boost::system::error_code &err);

    void stop();

    bool started() const;

    void do_read();

    void do_send(const std::string &msg);

    void do_write(const string msg);

    void handle_write(const boost::system::error_code &err, size_t bytes);

    void handle_read(const boost::system::error_code &err, size_t bytes);

    void set_receive_data(void *receivedata);

    void handle_handshake(const boost::system::error_code &error);

    void close();

    void send_msg();

    //void start_listen();
    //void handle_talk_to_server_thread();
    void start_timer(const boost::system::error_code &err);

    static void *write_func(void *arg);

    int read_completion(const boost::system::error_code &ec, size_t bytes_transferred);

    int get_read_count();

    int get_native_fd();

private:
    boost::asio::io_service &m_service;
#if defined(ASIO_SSL)
    ssl_socket m_socket;
#elif defined(ASIO_TCP)
    boost::asio::ip::tcp::socket m_socket;
#elif defined(ASIO_LOCAL)
    local_socket m_socket;
#else
#endif
    deadline_timer m_timer;
#ifndef ASIO_LOCAL
    boost::asio::ip::tcp::endpoint m_ep;
#endif
    bool m_bStart;
    char m_read_buffer[MAX_MSG];
    char m_write_buffer[MAX_MSG];
    ReceiveData m_receive_data;
    boost::asio::ip::tcp::resolver::iterator m_endpoint_iterator;
    queue<string> m_vSendMsg;
    std::deque<string> m_message_queue;
    int m_write_count;
    int m_send_count;
    int m_read_count;
    char m_read_buf[MAX_MSG];
    size_t headlen;
#ifdef ASIO_LOCAL
    std::string m_filename;
#endif
};

#endif //HBAUDITFLOW_CTALK_TO_SERVER_H
