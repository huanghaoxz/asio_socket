//
// Created by huanghao on 19-5-13.
//

#include <iostream>
#include <boost/thread/recursive_mutex.hpp>
#include "talk_to_client.h"
#include "hb_log4def.h"

int CTalk_to_client::clientnum = 0;
//array_clients m_clients;
//boost::recursive_mutex m_cs;
#ifdef ASIO_SSL
CTalk_to_client::CTalk_to_client(boost::asio::io_service &service, boost::asio::ssl::context &m) : m_socket(service,m) {
#else
    CTalk_to_client::CTalk_to_client(boost::asio::io_service &service, boost::asio::ssl::context &m) : m_socket(service) {
#endif
    m_bStart = false;
    m_read_count = 0;
}

CTalk_to_client::~CTalk_to_client() {

}

client_ptr CTalk_to_client::new_client(boost::asio::io_service &service, asio::ssl::context &m) {

    client_ptr client(new CTalk_to_client(service, m));
    return client;
}

void CTalk_to_client::start() {
    //boost::recursive_mutex::scoped_lock lk(m_cs);
#ifdef ASIO_SSL
    m_socket.async_handshake(boost::asio::ssl::stream_base::server,
                             boost::bind(&CTalk_to_client::handle_handshake, shared_from_this(),
                                         boost::asio::placeholders::error));
#else
    hbla_log_info("client count %d is connected", ++CTalk_to_client::clientnum);
    m_bStart = true;
    do_read();
#endif
}

#ifdef ASIO_SSL
void CTalk_to_client::handle_handshake(const boost::system::error_code &error) {
    if (!error) {
        m_bStart = true;
        hbla_log_info("handle_handshake success,client count %d is connected", ++CTalk_to_client::clientnum);
        do_read();
    } else {
        hbla_log_error("handle_handshake error :%s", error.message().c_str());
        close();
    }
}
#endif

void CTalk_to_client::stop() {
    if (!m_bStart) {
        return;
    }
    close();
}

/*
void CTalk_to_client::del_client() {
    boost::recursive_mutex::scoped_lock lk(m_cs);
    array_clients::iterator it = std::find(m_clients.begin(), m_clients.end(), shared_from_this());
    if (it != m_clients.end()) {
        m_clients.erase(it);
        hbla_log_info("client count %d is connected", --CTalk_to_client::clientnum);
    }
}*/

void CTalk_to_client::do_read() {
    if (!m_bStart) {
        hbla_log_error("server is not started");
        return;
    }
    //每个客户端读取自己的
    boost::shared_ptr<std::vector<char>> read_ptr(new std::vector<char>(max_msg, 0));
    m_socket.async_read_some(boost::asio::buffer(*read_ptr),
                             boost::bind(&CTalk_to_client::handle_read, shared_from_this(), read_ptr, _1, _2));
}

void CTalk_to_client::handle_read(boost::shared_ptr<std::vector<char>> read_ptr, const boost::system::error_code &err,
                                  size_t bytes) {
    if (!err)//没有错误
    {
        //std::cout << "read bytes:" << bytes << std::endl;
        string message = "";
        message.assign(read_ptr->begin(), read_ptr->begin() + bytes);
#ifdef ASIO_SSL
        m_receive_data(message, bytes, m_socket.lowest_layer().native(),m_read_count);
#else
        m_receive_data(message, bytes, m_socket.native(),m_read_count);
        //cout <<"nstatis:"<<m_read_count<<endl;
#endif
        //hbla_log_info("handle_read do_read");
        do_read();
    } else {
        //可以将保存的数组减少1
        //当bytes =0时表示，非阻塞套接字,读取时没有数据返回0,服务端断开连接 bytes = 0
        //客户端断开也是一个错误,可根据协议来解决正常退出
#ifdef ASIO_SSL
        hbla_log_error("socket id:%d bytes:%d  err code:%d  error:%s",m_socket.lowest_layer().native(),bytes, err.value(), err.message().c_str());
#else
        hbla_log_error("socket id:%d bytes:%d  err code:%d  error:%s", m_socket.native(), bytes, err.value(),
                       err.message().c_str());
#endif
        close();
    }
}

void CTalk_to_client::set_receive_data(void *receivedata) {
    m_receive_data = (ReceiveData) (receivedata);
}


//boost::asio::ip::tcp::socket &CTalk_to_client::get_socket() {
ssl_socket::lowest_layer_type &CTalk_to_client::get_socket() {
#ifdef ASIO_SSL
    return m_socket.lowest_layer();
#else
    return m_socket;
#endif
}


void CTalk_to_client::do_write(std::string &messsage) {
    if (messsage.size() > max_msg) {
        hbla_log_error("msg size is too big");
    } else {
        auto total_len = messsage.size();
        auto head_len = htons(messsage.size());
        string msg;
        msg.reserve(total_len);
        msg.append((const char *) &head_len, sizeof(uint16_t));
        msg.append(messsage);
        m_socket.async_write_some(buffer(msg),
                                  boost::bind(&CTalk_to_client::handle_write, shared_from_this(), _1, _2));
    }
}

void CTalk_to_client::handle_write(const boost::system::error_code &err, size_t bytes) {
    if (!err) {
        //do_read();
    } else {
        hbla_log_error("write_handler err:%s", err.message().c_str());
        close();
    }
}


void CTalk_to_client::set_client_changed() {
    m_client_changed = true;
}

void CTalk_to_client::close() {
#ifdef ASIO_SSL
    if (m_socket.lowest_layer().is_open()) {
        hbla_log_info("socket id %d closse", m_socket.lowest_layer().native());
        m_socket.lowest_layer().close();
        hbla_log_info("client count %d is connected", --CTalk_to_client::clientnum);
    }
#else
    if (m_socket.is_open()) {
        hbla_log_info("socket id %d closse", m_socket.native());
        m_socket.close();
        hbla_log_info("client count %d is connected", --CTalk_to_client::clientnum);
    }
#endif
    m_bStart = false;
}