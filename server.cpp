//
// Created by huanghao on 19-5-15.
//
#include <boost/thread/recursive_mutex.hpp>
#include "server.h"
#include "hb_log4def.h"

array_clients m_clients;
boost::recursive_mutex m_cs;

//extern boost::asio::io_service service;
CServer::CServer(std::string ip, short port, const asio::ssl::context::method &m) : m_acceptor(service,
                                                                                               ip::tcp::endpoint(
                                                                                                       ip::address::from_string(
                                                                                                               ip),
                                                                                                       port)),
                                                                                    m_content(m) {
    m_clients.clear();

}

/*
CServer::CServer(std::string ip, short port,const asio::ssl::context::method& m) : m_acceptor(service,
                                                                                              ip::tcp::endpoint(ip::address::from_string(ip), port)),
                                                                                   {
}*/


CServer::~CServer() {

}

void CServer::start() {
    //这里加上ssl
    client_ptr client = CTalk_to_client::new_client(service, m_content);
    m_acceptor.async_accept(client->get_socket(), boost::bind(&CServer::handle_accept, this, client, _1));
    for (int i = 0; i < THREAD_NUM; ++i) {
        boost::thread(boost::bind(&CServer::handle_talk_to_client_thread, this));
    }
}

void CServer::handle_accept(client_ptr client, const boost::system::error_code &err) {
    boost::recursive_mutex::scoped_lock lk(m_cs);
    m_clients.push_back(client);
    client->set_receive_data((void *) m_receive_data);
    client->set_client_changed();
    client->start();
    client_ptr newclient = CTalk_to_client::new_client(service, m_content);
    m_acceptor.async_accept(newclient->get_socket(), boost::bind(&CServer::handle_accept, this, newclient, _1));
}

void CServer::handle_talk_to_client_thread() {
    boost::asio::io_service::work work(service);
    service.run();
}

void CServer::update_clients_changed() {
    /*
    array_clients copy;
    {
        boost::recursive_mutex::scoped_lock lk(m_clients_cs);
        copy = m_clients;
        for (array_clients::iterator b = copy.begin(),e=copy.end();b!=e;++b) {
            (*b)->set_clients_changed();
        }
    }
     */
}

void CServer::stop() {
    boost::recursive_mutex::scoped_lock lk(m_cs);
    cout << "stop" << endl;
    if(m_clients.size()>0)
    {
        array_clients::iterator it = m_clients.begin();
        for (it; it != m_clients.end(); it++) {
            if ((*it)->started()) {
                (*it)->stop();
            }
        }
    }
    m_clients.clear();
}

void CServer::stop_client(client_ptr client) {
    boost::recursive_mutex::scoped_lock lk(m_cs);
    array_clients::iterator it = std::find(m_clients.begin(), m_clients.end(), client);
    if (it != m_clients.end()) {
        client->stop();
        m_clients.erase(it);
    }
}

void CServer::stop_client(int fd) {
    boost::recursive_mutex::scoped_lock lk(m_cs);
    array_clients::iterator it = m_clients.begin();
    for (; it != m_clients.end(); ++it) {
#ifdef ASIO_SSL
        if ((*it)->get_socket().lowest_layer().native() == fd) {
#else
        if ((*it)->get_socket().native() == fd) {
#endif
            (*it)->stop();
            m_clients.erase(it);
        }
    }
}


void CServer::set_receive_data(void *receivedata) {
    m_receive_data = (ReceiveData) receivedata;
}

void CServer::send_msg(int fd, std::string &msg) {
    boost::recursive_mutex::scoped_lock lk(m_cs);
    array_clients::iterator it = m_clients.begin();
    for (; it != m_clients.end(); ++it) {
        if ((*it)->get_socket().lowest_layer().native() == fd) {
            (*it)->do_write(msg);
            return;
        }
    }
    hbla_log_error("can not find the socket %d", fd);
}

asio::ssl::context &CServer::context() {
    return m_content;
}
