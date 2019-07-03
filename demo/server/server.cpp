//
// Created by huanghao on 19-5-22.
//
#include <iostream>
#include <string>
//#include <boost/>
#include "server.h"
//#include "talk_to_client.h"
//#include "common.h"
#include "hbla_log4.h"

using namespace std;

HbLog *hb_log = new HbLog();
//boost::asio::io_service service;
CServer server("10.0.1.106", 8888, asio::ssl::context::sslv23_server);

int ncount = 0;
boost::recursive_mutex m_mutex;
void receive_data(std::string &message, int bytes, int fd,int &nstatics) {

    boost::recursive_mutex::scoped_lock lk(m_mutex);
    cout <<"message:" << message <<" bytes: " << bytes << "  fd " << fd <<endl;
    cout << "ncount"<< ncount++<<endl;
    string msg  = "123456";
    server.send_msg(fd,msg);
    return;
}

int main() {
    cout << "server" << endl;
    //string ip = "10.0.1.106";
    //short port = 8888;
    //CServer server(ip,port);
    server.context().set_options(
            asio::ssl::context::default_workarounds | asio::ssl::context::no_sslv2 | asio::ssl::context::single_dh_use);
    server.context().set_verify_mode(asio::ssl::context::verify_peer | asio::ssl::context::verify_fail_if_no_peer_cert);
    server.context().load_verify_file("client_certs/client.crt");
    server.context().use_certificate_chain_file("certs/server.crt");
    server.context().use_private_key_file("certs/priv.key", asio::ssl::context::pem);
    server.set_receive_data((void *) receive_data);
    server.start();
    while (1) {
        sleep(1);
    }
    return 0;
}
