//
// Created by huanghao on 19-5-15.
//
#include <iostream>
#include <boost/thread.hpp>
#include <pthread.h>
#include <boost/lexical_cast.hpp>
#include <boost/thread/mutex.hpp>
#include <net/if.h>
#include "talk_to_server.h"
#include "hb_log4def.h"
#include "packer.h"

using boost::asio::local::stream_protocol;
pthread_mutex_t send_msg_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t send_msg_cont = PTHREAD_COND_INITIALIZER;
boost::recursive_mutex g_mutex;

//CTalk_to_server::CTalk_to_server() : m_socket(m_service),m_timer(m_service,boost::bind(&CTalk_to_server::start_timer,this)){
#if defined(ASIO_SSL)

CTalk_to_server::CTalk_to_server(boost::asio::io_service &ios, boost::asio::ssl::context &m,
                                 boost::asio::ip::tcp::resolver::iterator endpoint_iterator) : m_service(ios),
                                                                                               m_socket(ios, m),
                                                                                               m_timer(ios)
                                                                                               {
   m_write_count = 0;
    m_send_count = 0;
    m_read_count = 0;
    m_bStart = false;
    headlen = 0;
    m_endpoint_iterator = endpoint_iterator;
    memset(m_read_buf, 0, MAX_MSG);
    //m_timer.expires_from_now(boost::posix_time::seconds(3));
    //m_timer.async_wait(boost::bind(&CTalk_to_server::start_timer, this, boost::asio::placeholders::error));
}

#elif defined(ASIO_TCP)

CTalk_to_server::CTalk_to_server(boost::asio::io_service &ios,
                                 boost::asio::ip::tcp::resolver::iterator endpoint_iterator) : m_service(ios),
                                                                                               m_socket(ios),
                                                                                               m_timer(ios) {
   m_write_count = 0;
    m_send_count = 0;
    m_read_count = 0;
    m_bStart = false;
    headlen = 0;
    m_endpoint_iterator = endpoint_iterator;
    memset(m_read_buf, 0, MAX_MSG);
    //m_timer.expires_from_now(boost::posix_time::seconds(3));
    //m_timer.async_wait(boost::bind(&CTalk_to_server::start_timer, this, boost::asio::placeholders::error));
}

#elif defined(ASIO_LOCAL)

CTalk_to_server::CTalk_to_server(boost::asio::io_service &ios,const std::string filename):m_service(ios),m_socket(ios),m_timer(ios) {
    m_write_count = 0;
    m_send_count = 0;
    m_read_count = 0;
    m_bStart = false;
    headlen = 0;
    //m_endpoint_iterator = endpoint_iterator;
    memset(m_read_buf, 0, MAX_MSG);
    m_filename = filename;
    //m_timer.expires_from_now(boost::posix_time::seconds(3));
    //m_timer.async_wait(boost::bind(&CTalk_to_server::start_timer, this, boost::asio::placeholders::error));
}

#else
#endif

CTalk_to_server::~CTalk_to_server() {

}

void CTalk_to_server::start_timer(const boost::system::error_code &err) {

    if (!err) {
        if (!m_bStart) {
#if defined(ASIO_SSL)
            if (!m_socket.lowest_layer().is_open()) {
                m_socket.lowest_layer().async_connect(m_ep,
                                                      boost::bind(&CTalk_to_server::handle_connect, shared_from_this(),
                                                                  boost::asio::placeholders::error));
            }
#elif defined(ASIO_TCP)
            if (!m_socket.is_open()) {
                m_socket.async_connect(m_ep,
                                       boost::bind(&CTalk_to_server::handle_connect, shared_from_this(),
                                                   boost::asio::placeholders::error));
            }
#elif defined(ASIO_LOCAL)
            if (!m_socket.is_open()) {
                m_socket.async_connect(boost::asio::local::stream_protocol::endpoint(LINUX_DOMAIN),
                                       boost::bind(&CTalk_to_server::handle_connect, shared_from_this(),
                                                   boost::asio::placeholders::error));
            }
#else
#endif
            else {
                //hbla_log_info("start_timer %s", err.message().c_str());
                //close();
            }
            m_timer.expires_at(m_timer.expires_at() + posix_time::millisec(2000));
            m_timer.async_wait(boost::bind(&CTalk_to_server::start_timer, this, boost::asio::placeholders::error));
        }
    }
}

#if defined(ASIO_SSL)
talk_to_server_ptr CTalk_to_server::create_client(boost::asio::ip::tcp::endpoint ep, boost::asio::io_service &ios,
                                                  boost::asio::ssl::context &m,
                                                  boost::asio::ip::tcp::resolver::iterator endpoint_iterator) {
    talk_to_server_ptr new_(new CTalk_to_server(ios, m, endpoint_iterator));
    new_->start(ep);
    return new_;
}
#elif defined(ASIO_TCP)

talk_to_server_ptr CTalk_to_server::create_client(boost::asio::ip::tcp::endpoint ep, boost::asio::io_service &ios,
                                                  boost::asio::ip::tcp::resolver::iterator endpoint_iterator) {
    talk_to_server_ptr new_(new CTalk_to_server(ios, endpoint_iterator));
    new_->start(ep);
    return new_;
}

#elif defined(ASIO_LOCAL)

talk_to_server_ptr CTalk_to_server::create_client(boost::asio::io_service &ios,const std::string filename) {
    talk_to_server_ptr new_(new CTalk_to_server(ios,filename));
    new_->start();
    return new_;
}

#else
#endif



#if defined(ASIO_SSL)
void CTalk_to_server::start(boost::asio::ip::tcp::endpoint ep) {
    m_ep = ep;
    m_socket.lowest_layer().async_connect(ep, boost::bind(&CTalk_to_server::handle_connect, shared_from_this(),
                                                          boost::asio::placeholders::error));
    cout << m_socket.lowest_layer().native() << endl;
    }
#elif defined(ASIO_TCP)
void CTalk_to_server::start(boost::asio::ip::tcp::endpoint ep) {
    m_ep = ep;
    m_socket.async_connect(ep, boost::bind(&CTalk_to_server::handle_connect, shared_from_this(),
                                           boost::asio::placeholders::error));
    cout << m_socket.native() << endl;
    }

#elif defined(ASIO_LOCAL)
void CTalk_to_server::start() {

    m_socket.async_connect(stream_protocol::endpoint(LINUX_DOMAIN),
                           boost::bind(&CTalk_to_server::handle_connect, shared_from_this(),
                                       boost::asio::placeholders::error));
    cout << m_socket.native() << endl;
    }
#endif



int CTalk_to_server::get_native_fd() {
#if defined(ASIO_SSL)
    return m_socket.lowest_layer().native();
#elif defined(ASIO_TCP)
    return m_socket.native();
#elif defined(ASIO_LOCAL)
    return m_socket.native();
#else

#endif
}

void CTalk_to_server::handle_connect(const boost::system::error_code &err) {
    if (!err) {
#ifdef ASIO_SSL
        m_socket.async_handshake(boost::asio::ssl::stream_base::client,
                                 boost::bind(&CTalk_to_server::handle_handshake, shared_from_this(),
                                             boost::asio::placeholders::error));
#else
        hbla_log_info("%d connect server sucess", get_native_fd());
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
        hbla_log_info("%d handle_handshake success", get_native_fd());
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

int CTalk_to_server::read_completion(const boost::system::error_code &ec, size_t bytes_transferred) {
    if (ec) {
        cout << "ec.value" << ec.message() << endl;
        return 0;
    } else {
        if (bytes_transferred == sizeof(uint16_t)) {
            uint16_t head;
            memcpy(&head, m_read_buf, sizeof(uint16_t));
            headlen = ntohs(head);
            cout << "headlen " << headlen << endl;
        }
        if (headlen + sizeof(uint16_t) == bytes_transferred) {
            headlen = 0;
            return 0;
        }
    }
    return 1;
}

void CTalk_to_server::do_read() {

    memset(m_read_buf, 0, MAX_MSG);
#if defined(LEN_BODY)
    async_read(m_socket, boost::asio::buffer(m_read_buf), [this](const boost::system::error_code &ec,
                                                                 size_t bytes_transferred) -> size_t {
                   return this->read_completion(ec, bytes_transferred);
               },
               boost::bind(&CTalk_to_server::handle_read, shared_from_this(),
                           boost::asio::placeholders::error,
                           boost::asio::placeholders::bytes_transferred()));
#elif defined(FIX_LEN)
    async_read(m_socket, boost::asio::buffer(m_read_buf), transfer_exactly(MAX_MSG),
               boost::bind(&CTalk_to_server::handle_read, shared_from_this(),
                           boost::asio::placeholders::error,
                           boost::asio::placeholders::bytes_transferred()));
#else
    async_read(m_socket, boost::asio::buffer(m_read_buf), [this](const boost::system::error_code & ec, size_t bytes_transferred)->size_t {return this->read_completion(ec, bytes_transferred);},
               boost::bind(&CTalk_to_server::handle_read, shared_from_this(),
                           boost::asio::placeholders::error,
                           boost::asio::placeholders::bytes_transferred()));
#endif
}


void
CTalk_to_server::handle_read(const boost::system::error_code &err,
                             size_t bytes) {
#if 1
    if (!err)//没有错误
    {
        string message = "";
#if defined(LEN_BODY)
        message = m_read_buf + 2;
#elif defined(FIX_LEN)
        message = m_read_buf;
#else
        message = m_read_buf+2;
#endif
        hbla_log_info("handle read count %d", m_read_count++);
        m_receive_data(message, bytes, get_native_fd());
        do_read();
    } else {
        if (err.value() != boost::system::errc::operation_canceled && err.value() != error::eof/*2*/) {
            hbla_log_error("bytes:%d err value:%d %s", bytes, err.value(), err.message().c_str());
        }
        //当bytes =0时表示，非阻塞套接字,读取时没有数据返回0,服务端断开连接 bytes =0
        close();
    }
#endif

}


void CTalk_to_server::do_send(const std::string &msg) {

#if 1
    pthread_mutex_lock(&send_msg_lock);
    //cout << "m_write_count:"<<m_write_count++ <<endl;
    m_vSendMsg.push(msg);
    pthread_mutex_unlock(&send_msg_lock);
    pthread_cond_signal(&send_msg_cont);
#endif

#if 0
    packer m_packer;
#if defined(LEN_BODY)
    string result = m_packer.pack_msg_len_body(msg);
        m_socket.async_write_some(buffer(result), boost::bind(&CTalk_to_server::handle_write, shared_from_this(),
                                                                   boost::asio::placeholders::error,
                                                                   boost::asio::placeholders::bytes_transferred()));
#elif defined(FIX_LEN)
    string result = m_packer.pack_msg_fix_length(msg,MAX_MSG);
    //cout << "msg.size()" << msg.size() << endl;
    async_write(m_socket,buffer(result,MAX_MSG),boost::bind(&CTalk_to_server::handle_write, shared_from_this(),
                                                                 boost::asio::placeholders::error,
                                                                 boost::asio::placeholders::bytes_transferred()));
#else
    string result = m_packer.pack_msg_len_body(msg);
        m_socket.async_write_some(buffer(result), boost::bind(&CTalk_to_server::handle_write, shared_from_this(),
                                                                   boost::asio::placeholders::error,
                                                                   boost::asio::placeholders::bytes_transferred()));
#endif
#endif
}

int nsend = 0;

void *CTalk_to_server::write_func(void *arg) {
#if 1
    CTalk_to_server *client = (CTalk_to_server *) arg;
    while (1) {
        if (!client->m_bStart) {
            //hbla_log_info("client is not started");
            continue;
        }
        pthread_mutex_lock(&send_msg_lock);
        while (client->m_vSendMsg.size() == 0) {
            //hbla_log_info("message num is zero");
            pthread_cond_wait(&send_msg_cont, &send_msg_lock);
        }

        string msg = client->m_vSendMsg.front();
        //sleep(1);
        client->m_service.post(boost::bind(&CTalk_to_server::do_write, client, msg));
        client->m_vSendMsg.pop();
        pthread_mutex_unlock(&send_msg_lock);
    }

#endif
    return NULL;
}


void CTalk_to_server::do_write(const string msg) {
    bool write_in_progress = !m_message_queue.empty();
    m_message_queue.push_back(msg);
    if (!write_in_progress) {
        packer m_packer;
#if defined(LEN_BODY)
        cout << "body ";
        string result = m_packer.pack_msg_len_body(m_message_queue.front());
        cout << result.size() << endl;
        async_write(m_socket, buffer(result, result.size()),
                    boost::bind(&CTalk_to_server::handle_write, shared_from_this(),
                                boost::asio::placeholders::error,
                                boost::asio::placeholders::bytes_transferred()));
#elif defined(FIX_LEN)
        string result = m_packer.pack_msg_fix_length(m_message_queue.front(), MAX_MSG);
    async_write(m_socket, buffer(result, MAX_MSG), boost::bind(&CTalk_to_server::handle_write, shared_from_this(),
                                                                    boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred()));
#else
    string result = m_packer.pack_msg_len_body(m_message_queue.front());
        m_socket.async_write_some(buffer(msg), boost::bind(&CTalk_to_server::handle_write, shared_from_this,
                                                                   boost::asio::placeholders::error,
                                                                   boost::asio::placeholders::bytes_transferred()));
#endif
    }

}


int ntest = 0;

void CTalk_to_server::handle_write(const boost::system::error_code &err, size_t bytes) {
    if (err) {
        hbla_log_error("handle_write err %s", err.message().c_str());
        close();
    } else {
        hbla_log_info("client success send ntestcount = %d   ,%d", ntest++, bytes);
        m_message_queue.pop_front();
        if (!m_message_queue.empty()) {
            packer m_packer;
#if defined(LEN_BODY)
            cout << "body ";
            string msg = m_packer.pack_msg_len_body(m_message_queue.front());
            cout << msg.size() << endl;
            async_write(m_socket, buffer(msg, msg.size()),
                        boost::bind(&CTalk_to_server::handle_write, shared_from_this(),
                                    boost::asio::placeholders::error,
                                    boost::asio::placeholders::bytes_transferred()));
#elif defined(FIX_LEN)
            string msg = m_packer.pack_msg_fix_length(m_message_queue.front(), MAX_MSG);
    async_write(m_socket, buffer(msg, MAX_MSG), boost::bind(&CTalk_to_server::handle_write, shared_from_this(),
                                                                    boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred()));
#else
    string msg = m_packer.pack_msg_len_body(m_message_queue.front());
        m_socket.async_write_some(buffer(msg), boost::bind(&CTalk_to_server::handle_write, shared_from_this,
                                                                   boost::asio::placeholders::error,
                                                                   boost::asio::placeholders::bytes_transferred()));
#endif
        }

    }
}

void CTalk_to_server::set_receive_data(void *receivedata) {
    m_receive_data = (ReceiveData) (receivedata);
}

void CTalk_to_server::close() {
    boost::recursive_mutex::scoped_lock lk(g_mutex);
#ifdef ASIO_SSL
    if (m_socket.lowest_layer().is_open()) {
        m_socket.lowest_layer().close();
    }
#else
    if (m_socket.is_open()) {
        m_socket.close();
    }
#endif
    if (m_bStart) {
        hbla_log_info("client is closing");
    }
    m_bStart = false;
}


void CTalk_to_server::send_msg() {

    pthread_t m_tid;
    pthread_create(&m_tid, NULL, write_func, (void *) this);
}

int CTalk_to_server::get_read_count() {
    return m_read_count;
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
