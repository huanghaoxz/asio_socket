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


using namespace std;
using namespace boost;
using namespace boost::asio;


class CTalk_to_client:public boost::enable_shared_from_this<CTalk_to_client>,boost::noncopyable
 {
public:
    CTalk_to_client(boost::asio::io_service &service,boost::asio::ssl::context& m);

    ~CTalk_to_client();

    static client_ptr new_client(boost::asio::io_service &service,asio::ssl::context& m);

    void start();

    void stop();

    void do_read();
    void handle_read(const boost::system::error_code& err,size_t bytes);


    void do_write(std::string &messsage);
    void handle_write(const boost::system::error_code& err,size_t bytes);

    //boost::asio::ip::tcp::socket &get_socket();
    ssl_socket::lowest_layer_type &get_socket();

    void set_client_changed();

    void set_receive_data(void* receivedata);
    static int clientnum;
    void del_client();
    void handle_handshake(const boost::system::error_code& error);
    void close();
    static void* threadFunc(void *arg);
    int read_completion(const boost::system::error_code & ec,size_t bytes);
private:
#ifdef ASIO_SSL
    ssl_socket m_socket;
#else
    boost::asio::ip::tcp::socket m_socket;
#endif
    bool m_bStart;
    //enum {max_msg = MAX_MSG_NUM};
    ReceiveData m_receive_data;
    boost::asio::ip::tcp::endpoint m_ep;
    bool m_client_changed;
    //queue<string> m_receive_msg_queue;
    int m_read_count;
    size_t headlen;
    char m_read_buf[MAX_MSG];
};

#endif //HBAUDITFLOW_CTALK_TO_CLIENT_H
