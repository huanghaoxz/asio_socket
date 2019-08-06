//
// Created by huanghao on 19-5-13.
//

#include <iostream>
#include <boost/thread/recursive_mutex.hpp>
#include "talk_to_client.h"
#include "hb_log4def.h"

int CTalk_to_client::clientnum = 0;

pthread_mutex_t recv_msg_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t recv_msg_cont = PTHREAD_COND_INITIALIZER;
boost::recursive_mutex g_mutex;

#if defined(ASIO_SSL)
CTalk_to_client::CTalk_to_client(boost::asio::io_service &service, boost::asio::ssl::context &m): m_socket(service,m){
#elif defined(ASIO_TCP)
    CTalk_to_client::CTalk_to_client(boost::asio::io_service &service) : m_socket(service) {
#elif defined(ASIO_LOCAL)
CTalk_to_client::CTalk_to_client(boost::asio::io_service &service) : m_socket(service) {
#endif
    m_bStart = false;
    m_read_count = 0;
    headlen = 0;
    memset(m_read_buf, 0, MAX_MSG);
}

CTalk_to_client::~CTalk_to_client() {

}

void *CTalk_to_client::threadFunc(void *arg) {
    return NULL;
}

#if defined(ASIO_SSL)
client_ptr CTalk_to_client::new_client(boost::asio::io_service &service, asio::ssl::context &m) {

    client_ptr client(new CTalk_to_client(service, m));
    return client;
}
#elif defined(ASIO_TCP)
client_ptr CTalk_to_client::new_client(boost::asio::io_service &service) {

    client_ptr client(new CTalk_to_client(service));
    return client;
}
#elif defined(ASIO_LOCAL)
client_ptr CTalk_to_client::new_client(boost::asio::io_service &service) {

    client_ptr client(new CTalk_to_client(service));
    return client;
}
#endif




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
#ifdef ASIO_SSL
    if (m_socket.lowest_layer().is_open()) {
        m_socket.lowest_layer().close();
    }
#else
    m_socket.close();
#endif
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

int CTalk_to_client::read_completion(const boost::system::error_code &ec, size_t bytes_transferred) {
    //cout << "bytes_transferred" << bytes_transferred << endl;
    if (ec) {
        if (ec.value() != error::eof/*2*/ && ec.value() != 125) {
            hbla_log_error("read_completion error value=%d, message=%s", ec.value(), ec.message().c_str());
        }
        return 0;
    } else {
        if (bytes_transferred == sizeof(uint16_t)) {
            uint16_t head;
            memcpy(&head, m_read_buf, sizeof(uint16_t));
            headlen = ntohs(head);
        }
        if (headlen + sizeof(uint16_t) == bytes_transferred) {
            headlen = 0;
            return 0;
        }
    }

    return 1;
}

void CTalk_to_client::do_read() {
    if (!m_bStart) {
        hbla_log_error("server is not started");
        return;
    }
    //每个客户端读取自己的
    memset(m_read_buf, 0, MAX_MSG);
#if defined(LEN_BODY)
    async_read(m_socket, boost::asio::buffer(m_read_buf), [this](const boost::system::error_code &ec,
                                                                 size_t bytes_transferred) -> size_t {
                   return this->read_completion(ec, bytes_transferred);
               },
               boost::bind(&CTalk_to_client::handle_read, shared_from_this(), _1, _2));
#elif defined(FIX_LEN)
    async_read(m_socket,boost::asio::buffer(m_read_buf),transfer_exactly(MAX_MSG),
                             boost::bind(&CTalk_to_client::handle_read, shared_from_this(), _1, _2));
#else
        async_read(m_socket,boost::asio::buffer(m_read_buf),[this](const boost::system::error_code & ec, size_t bytes_transferred)->size_t {return this->read_completion(ec, bytes_transferred);},
                             boost::bind(&CTalk_to_client::handle_read, shared_from_this(), _1, _2));
#endif

}

int CTalk_to_client::get_native_fd() {
#if defined(ASIO_SSL)
    return m_socket.lowest_layer().native();
#elif defined(ASIO_TCP)
    return m_socket.native();
#elif defined(ASIO_LOCAL)
    return  m_socket.native();
#else

#endif
}

void CTalk_to_client::handle_read(const boost::system::error_code &err,
                                  size_t bytes) {
    if (!err)//没有错误
    {
        std::cout << " handle read count " << m_read_count++ << "     read bytes: " << bytes << std::endl;
        string message = "";
#if defined(LEN_BODY)
        message = m_read_buf + 2;
#elif defined(FIX_LEN)
        message = m_read_buf;
#else
        message = m_read_buf+2;
#endif
        m_receive_data(message, bytes, get_native_fd());
        //hbla_log_info("handle read count = %d", m_read_count++);
        do_read();
    } else {
        //可以将保存的数组减少1
        //当bytes =0时表示，非阻塞套接字,读取时没有数据返回0,服务端断开连接 bytes = 0
        //客户端断开也是一个错误,可根据协议来解决正常退出
        if (err.value() != error::eof/*2*/ && err.value() != 125) {
            hbla_log_error("socket id:%d bytes:%d  err code:%d  error:%s", get_native_fd(), bytes, err.value(),
                           err.message().c_str());
        }
        close();
    }
}

void CTalk_to_client::set_receive_data(void *receivedata) {
    m_receive_data = (ReceiveData) (receivedata);
}


#if defined(ASIO_SSL)

ssl_socket::lowest_layer_type &CTalk_to_client::get_socket() {
    return m_socket.lowest_layer();
#elif defined(ASIO_TCP)
    boost::asio::ip::tcp::socket &CTalk_to_client::get_socket() {
    return m_socket;
#elif defined(ASIO_LOCAL)
    local_socket & CTalk_to_client::get_socket(){
    return m_socket;
#endif
}


void CTalk_to_client::do_write(std::string &messsage) {
    if (!m_bStart) {
        hbla_log_info("do_write: server is not started");
        return;
    }
    if (messsage.size() > MAX_MSG) {
        hbla_log_error("msg size is too big");
    } else {
        packer m_packer;
#if  defined(LEN_BODY)
        string msg = m_packer.pack_msg_len_body(messsage);
        async_write(m_socket, buffer(msg),
                    boost::bind(&CTalk_to_client::handle_write, shared_from_this(), _1, _2));
#elif defined(FIX_LEN)
        string msg = m_packer.pack_msg_fix_length(messsage,MAX_MSG);
        async_write(m_socket,buffer(msg,MAX_MSG),
                                  boost::bind(&CTalk_to_client::handle_write, shared_from_this(), _1, _2));
#else
         string msg = m_packer.pack_msg_len_body(messsage);
         m_socket.async_write_some(buffer(msg),
                                  boost::bind(&CTalk_to_client::handle_write, shared_from_this(), _1, _2));
#endif

#if 0
        char write_buffer[max_msg] = {0};
        std::copy(messsage.begin(),messsage.end(),write_buffer);
        async_write(m_socket,buffer(write_buffer,max_msg),
                                  boost::bind(&CTalk_to_client::handle_write, shared_from_this(), _1, _2));
#endif
    }
}

int ntest = 0;

void CTalk_to_client::handle_write(const boost::system::error_code &err, size_t bytes) {
    if (!err) {
        hbla_log_info("server success send ntestcount = %d   ,%d", ntest++, bytes);
    } else {
        hbla_log_error("write_handler err:%s", err.message().c_str());
        close();
    }
}


void CTalk_to_client::set_client_changed() {
    m_client_changed = true;
}

void CTalk_to_client::close() {
    boost::recursive_mutex::scoped_lock lk(g_mutex);
    if (!m_bStart) {
        return;
    }
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

bool CTalk_to_client::started() {
    return m_bStart;
}