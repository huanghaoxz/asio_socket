//
// Created by huanghao on 19-5-15.
//
#include <iostream>
#include <boost/thread.hpp>
#include <pthread.h>
#include <boost/lexical_cast.hpp>
#include "talk_to_server.h"
#include "hb_log4def.h"


pthread_mutex_t send_msg_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t send_msg_cont = PTHREAD_COND_INITIALIZER;

//CTalk_to_server::CTalk_to_server() : m_socket(m_service),m_timer(m_service,boost::bind(&CTalk_to_server::start_timer,this)){
#ifdef ASIO_SSL
CTalk_to_server::CTalk_to_server(boost::asio::io_service &ios, boost::asio::ssl::context &m,
                                 boost::asio::ip::tcp::resolver::iterator endpoint_iterator) : m_service(ios),
                                                                                               m_socket(ios,m),
                                                                                               m_timer(ios) {
#else
    CTalk_to_server::CTalk_to_server(boost::asio::io_service &ios, boost::asio::ssl::context &m,
                                 boost::asio::ip::tcp::resolver::iterator endpoint_iterator) : m_service(ios),
                                                                                               m_socket(ios),
                                                                                               m_timer(ios) {
#endif
    m_write_count = 0;
    m_send_count = 0;
    m_read_count = 0;
    m_bStart = false;
    m_endpoint_iterator = endpoint_iterator;
#ifdef ASIO_SSL
    //m_socket(ios, m);
#else
    //m_socket(ios);
#endif
    //m_timer.expires_from_now(boost::posix_time::seconds(3));
    //m_timer.async_wait(boost::bind(&CTalk_to_server::start_timer, this, boost::asio::placeholders::error));
}

CTalk_to_server::~CTalk_to_server() {

}

void CTalk_to_server::start_timer(const boost::system::error_code &err) {

    if (!err) {
        if (!m_bStart) {
#ifdef ASIO_SSL
            if (!m_socket.lowest_layer().is_open()) {
                m_socket.lowest_layer().async_connect(m_ep,
                                                      boost::bind(&CTalk_to_server::handle_connect, shared_from_this(),
                                                                  boost::asio::placeholders::error));
#else
            if (!m_socket.is_open()) {
                m_socket.async_connect(m_ep,
                                       boost::bind(&CTalk_to_server::handle_connect, shared_from_this(),
                                                   boost::asio::placeholders::error));
#endif
            }
        } else {
            //hbla_log_info("start_timer %s", err.message().c_str());
            //close();
        }
        m_timer.expires_at(m_timer.expires_at() + posix_time::millisec(2000));
        m_timer.async_wait(boost::bind(&CTalk_to_server::start_timer, this, boost::asio::placeholders::error));
    }
}

talk_to_server_ptr CTalk_to_server::create_client(boost::asio::ip::tcp::endpoint ep, boost::asio::io_service &ios,
                                                  boost::asio::ssl::context &m,
                                                  boost::asio::ip::tcp::resolver::iterator endpoint_iterator) {
    talk_to_server_ptr new_(new CTalk_to_server(ios, m, endpoint_iterator));
    new_->start(ep);
    return new_;
}

void CTalk_to_server::start(boost::asio::ip::tcp::endpoint ep) {
    m_ep = ep;
#ifdef ASIO_SSL
    m_socket.lowest_layer().async_connect(ep, boost::bind(&CTalk_to_server::handle_connect, shared_from_this(),
                                                         boost::asio::placeholders::error));
#else
    m_socket.async_connect(ep, boost::bind(&CTalk_to_server::handle_connect, shared_from_this(),
                                           boost::asio::placeholders::error));
#endif

#if 0
    boost::asio::async_connect(m_socket.lowest_layer(), m_endpoint_iterator,
                               boost::bind(&CTalk_to_server::handle_connect, shared_from_this(),
                                           boost::asio::placeholders::error));
#endif

}

void CTalk_to_server::handle_connect(const boost::system::error_code &err) {
    if (!err) {
#ifdef ASIO_SSL
        m_socket.async_handshake(boost::asio::ssl::stream_base::client,
                                 boost::bind(&CTalk_to_server::handle_handshake, shared_from_this(),
                                             boost::asio::placeholders::error));
#else
        hbla_log_info("%d connect server sucess", m_socket.native());
        m_bStart = true;
        do_read();
        send_msg();
#endif


    } else {
        hbla_log_error("handle_connect err %s", err.message().c_str())
        close();
    }
}

#ifdef ASIO_SSL
void CTalk_to_server::handle_handshake(const boost::system::error_code &error) {
    if (!error) {
        m_bStart = true;
        hbla_log_info("%d handle_handshake success", m_socket.lowest_layer().native());
        do_read();
        send_msg();
    } else {
        std::cout << "Handshake failed: " << error.message() << "\n";
        close();
    }
}
#endif


void CTalk_to_server::stop() {
#ifdef ASIO_SSL
    hbla_log_info("socket id:%d close", m_socket.lowest_layer().native());
#else
    hbla_log_info("socket id:%d close", m_socket.native());
#endif
    if (!m_bStart)
        return;
    close();
}

bool CTalk_to_server::started() const {
    return m_bStart;
}

void CTalk_to_server::do_read() {
    boost::shared_ptr<std::vector<char>> read_ptr(new std::vector<char>(max_msg, 0));
    m_socket.async_read_some(boost::asio::buffer(*read_ptr),
                             boost::bind(&CTalk_to_server::handle_read, shared_from_this(), read_ptr,
                                         boost::asio::placeholders::error,
                                         boost::asio::placeholders::bytes_transferred()));
}

void
CTalk_to_server::handle_read(boost::shared_ptr<std::vector<char>> read_ptr, const boost::system::error_code &err,
                             size_t bytes) {
    if (!err)//没有错误
    {
        string message = "";
        message.assign(read_ptr->begin(), read_ptr->begin() + bytes);
        //cout << "handle read " << message << endl;
#ifdef ASIO_SSL
        m_receive_data(message, bytes, m_socket.lowest_layer().native(),m_read_count);
#else
        m_receive_data(message, bytes, m_socket.native(),m_read_count);
        //cout << "nstatis:"<<m_read_count<<endl;
#endif
        do_read();
    } else {
        //当bytes =0时表示，非阻塞套接字,读取时没有数据返回0,服务端断开连接 bytes =0
        hbla_log_error("bytes:%d err value:%d %s", bytes,err.value(),err.message().c_str());
        close();
    }
}


void CTalk_to_server::do_write(const std::string &msg) {

#if 1
    pthread_mutex_lock(&send_msg_lock);
    //cout << "m_write_count:"<<m_write_count++ <<endl;
    m_vSendMsg.push(msg);
    pthread_mutex_unlock(&send_msg_lock);
    pthread_cond_signal(&send_msg_cont);
#endif

#if 0
    cout << "write:" << msg << endl;
    m_socket.async_write_some(buffer(msg), boost::bind(&CTalk_to_server::handle_write, shared_from_this(),
                                                       boost::asio::placeholders::error,
                                                      boost::asio::placeholders::bytes_transferred()));
#endif
}

void CTalk_to_server::handle_write(const boost::system::error_code &err, size_t bytes) {
    if (err) {
        hbla_log_error("handle_write err %s", err.message().c_str());
        close();
    } else {
        //do_read();
    }
}

void CTalk_to_server::set_receive_data(void *receivedata) {
    m_receive_data = (ReceiveData) (receivedata);
}

void CTalk_to_server::close() {
#ifdef ASIO_SSL
    if (m_socket.lowest_layer().is_open()) {
    m_socket.lowest_layer().close();
    //m_socket.lowest_layer().shutdown();
    //m_socket.shutdown();
}
#else
    if (m_socket.is_open()) {
        m_socket.close();
        //m_socket.lowest_layer().shutdown();
        //m_socket.shutdown();
    }
#endif
    m_bStart = false;
}

void *CTalk_to_server::threadFunc(void *arg) {
#if 1
    CTalk_to_server *client = (CTalk_to_server *) arg;
    while (1) {
        if (!client->m_bStart) {
            hbla_log_info("client is not started");
            continue;
        }
        pthread_mutex_lock(&send_msg_lock);
        while (client->m_vSendMsg.size() == 0) {
            //hbla_log_info("message num is zero");
            pthread_cond_wait(&send_msg_cont, &send_msg_lock);
        }

        string body = client->m_vSendMsg.front();
        //auto head_len = lexical_cast<string>(body.size);

        auto total_len = body.size();
        auto head_len = htons(body.size());
        string msg;
        msg.reserve(total_len);
        msg.append((const char *) &head_len, sizeof(uint16_t));
        msg.append(body);
        //cout <<"pid_t:"<< pthread_self()<<" write:" << body ;
        //cout << " m_send_count:"<<client->m_send_count++ << endl;
#if 1
        client->m_socket.async_write_some(buffer(msg), boost::bind(&CTalk_to_server::handle_write, client,
                                                                   boost::asio::placeholders::error,
                                                                   boost::asio::placeholders::bytes_transferred()));
#endif

#if 0
        asio::async_write(client->m_socket, buffer(msg, msg.size()),
                          boost::bind(&CTalk_to_server::handle_write, client,
                                      boost::asio::placeholders::error,
                                      boost::asio::placeholders::bytes_transferred()));
#endif

        client->m_vSendMsg.pop();
        pthread_mutex_unlock(&send_msg_lock);
    }

#endif
    return NULL;
}

void CTalk_to_server::send_msg() {

    pthread_t m_tid;
    //pthread_create(&m_tid, NULL, threadFunc, (void*)this);
    pthread_create(&m_tid, NULL, threadFunc, (void *) this);


}



/*
void CTalk_to_server::start_listen() {
    m_service.run();
    //return;
    //hh 假如用下面的代码,有问题,定时器时而管用，时而不管用,现在想不清为什么,难道给 boost::asio::strand有关
    for (int i = 0; i < THREAD_NUM; ++i) {
        boost::thread(boost::bind(&CTalk_to_server::handle_talk_to_server_thread, this));
    }
}

void CTalk_to_server::handle_talk_to_server_thread() {
    boost::asio::io_service::work work(m_service);
    m_service.run();
}
*/
