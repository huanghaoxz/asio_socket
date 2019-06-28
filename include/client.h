//
// Created by huanghao on 19-5-22.
//

#ifndef HBAUDITFLOW_CLIENT_H
#define HBAUDITFLOW_CLIENT_H

#include <string>
#include <boost/asio.hpp>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/asio/ssl.hpp>
#include <vector>
#include "talk_to_server.h"
#include "common.h"

using namespace std;
using namespace boost;
using namespace boost::asio;

class CClient
{
public:
    CClient(std::string &ip, short &port,const asio::ssl::context::method & m);
    //CClient(std::string ip, short port);
    ~CClient();
    void start();
    void stop();
    void send_msg(std::string & msg);
    void set_receive_data(void* receivedata);
    bool get_client_status();
    void start_listen();
    void handle_talk_to_server_thread();
    asio::ssl::context& context();
private:
    talk_to_server_ptr m_talk_to_server;
    boost::asio::ip::tcp::endpoint m_ep;// m_ep和m_iterator实际上用一个就可以
    boost::asio::ssl::context m_content;
    boost::asio::ip::tcp::resolver::iterator m_iterator;
    void* m_receivedata;
    vector<string> m_vSendMsg;
};

#endif //HBAUDITFLOW_CLIENT_H
