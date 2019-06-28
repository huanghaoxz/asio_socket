//
// Created by huanghao on 19-5-22.
//
#include "client.h"
#include <boost/lexical_cast.hpp>
#include "hb_log4def.h"

boost::asio::io_service m_ios;//这个必须是全局变量，不然定时器和程序会出问题,具体原因没有分析出来

CClient::CClient(std::string &ip, short &port, const asio::ssl::context::method &m) : m_ep(
        boost::asio::ip::address::from_string(ip), port), m_content(m) {
    boost::asio::ip::tcp::resolver resolver(m_ios);
    boost::asio::ip::tcp::resolver::query query(ip, lexical_cast<string>(port));
    m_iterator = resolver.resolve(query);
}

/*
CClient::CClient(std::string ip, short port):m_ep(boost::asio::ip::address::from_string(ip),port) {
    m_talk_to_server = CTalk_to_server::create_client(m_ep,m_ios);
}
*/

CClient::~CClient() {

}

void CClient::start() {
    hbla_log_info("client start");
    m_talk_to_server = CTalk_to_server::create_client(m_ep, m_ios, m_content, m_iterator);
    m_talk_to_server->set_receive_data((void *) m_receivedata);
    start_listen();
}

void CClient::stop() {
    if (m_talk_to_server->started()) {
        m_talk_to_server->stop();
    } else
    {
        hbla_log_error("CClient::stop() client is not started");
    }
}

void CClient::send_msg(std::string &msg) {//此处优化,将消息放入一个缓存中

    m_talk_to_server->do_write(msg);
}

void CClient::set_receive_data(void *receivedata) {
    m_receivedata = (void *) receivedata;
}

bool CClient::get_client_status() {
    return m_talk_to_server->started();
}

void CClient::start_listen() {
    //hbla_log_info("start listen");
    for (int i = 0; i < THREAD_NUM; ++i) {
        boost::thread(boost::bind(&CClient::handle_talk_to_server_thread, this));
    }
}

void CClient::handle_talk_to_server_thread() {
    boost::asio::io_service::work work(m_ios);
    m_ios.run();
}

asio::ssl::context &CClient::context() {
    return m_content;
}