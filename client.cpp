//
// Created by huanghao on 19-5-22.
//
#include "client.h"
#include <boost/lexical_cast.hpp>
#include "hb_log4def.h"

boost::asio::io_service m_ios;//这个必须是全局变量，不然定时器和程序会出问题,具体原因没有分析出来

#if defined(ASIO_SSL)
CClient::CClient(std::string &ip, short &port, const asio::ssl::context::method &m) : m_ep(
        boost::asio::ip::address::from_string(ip), port), m_content(m) {
    boost::asio::ip::tcp::resolver resolver(m_ios);
    boost::asio::ip::tcp::resolver::query query(ip, lexical_cast<string>(port));
    m_iterator = resolver.resolve(query);
}
#elif defined(ASIO_TCP)

CClient::CClient(std::string &ip, short &port) : m_ep(
        boost::asio::ip::address::from_string(ip), port) {
    boost::asio::ip::tcp::resolver resolver(m_ios);
    boost::asio::ip::tcp::resolver::query query(ip, lexical_cast<string>(port));
    m_iterator = resolver.resolve(query);
}

#elif defined(ASIO_LOCAL)

CClient::CClient(const std::string &file) {
    m_filename = file;
}

#else
#endif


CClient::~CClient() {

}

void CClient::start() {
    hbla_log_info("client start");
#if defined(ASIO_SSL)
    m_talk_to_server = CTalk_to_server::create_client(m_ep, m_ios, m_content, m_iterator);
#elif defined(ASIO_TCP)
    m_talk_to_server = CTalk_to_server::create_client(m_ep, m_ios, m_iterator);
#elif defined(ASIO_LOCAL)
    m_talk_to_server = CTalk_to_server::create_client(m_ios,m_filename);
#else
#endif
    m_talk_to_server->set_receive_data((void *) m_receivedata);
}

void CClient::stop() {
    if (m_talk_to_server->started()) {
        m_talk_to_server->stop();
    } else {
        hbla_log_error("CClient::stop() client is not started");
    }
}

void CClient::send_msg(std::string &msg) {//此处优化,将消息放入一个缓存中

    m_talk_to_server->do_send(msg);
}

void CClient::set_receive_data(void *receivedata) {
    m_receivedata = (void *) receivedata;
}

bool CClient::get_client_status() {
    return m_talk_to_server->started();
}

void CClient::start_listen() {

    boost::asio::io_service::work work(m_ios);//保证service.run() 一直运行下去,直到你调用service.stop()或dummy_work.reset(0)
    m_ios.run();

    /*
    for (int i = 0; i < THREAD_NUM; ++i) {
        boost::thread(boost::bind(&CClient::handle_talk_to_server_thread, this));
    }*/
}

void CClient::handle_talk_to_server_thread() {
    boost::asio::io_service::work work(m_ios);//保证service.run() 一直运行下去,直到你调用service.stop()或dummy_work.reset(0)
    m_ios.run();
}

#ifdef ASIO_SSL
asio::ssl::context &CClient::context() {
    return m_content;
}
#endif