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
#define LINUX_DOMAIN "/tmp/byhj.domain"
HbLog *hb_log = new HbLog();
//boost::asio::io_service service;
//CServer server("172.16.0.183", 8888, asio::ssl::context::sslv23_server);//这里有bug,不能让ip作为参数
CServer server("172.16.0.183", 8888);
//CServer server(LINUX_DOMAIN);
int ncount = 0;
boost::recursive_mutex m_mutex;

void receive_data(std::string &message, int bytes, int fd) {

    boost::recursive_mutex::scoped_lock lk(m_mutex);
    cout << "message:" << message << " bytes: " << bytes << "  fd " << fd << endl;
    cout << "ncount" << ncount++ << endl;
    string msg = "123456";
    server.send_msg(fd, msg);
    return;
}


int main() {
    cout << "server" << endl;
    server.set_receive_data((void *) receive_data);
    server.start();
    server.start_listen();
    while (1) {
        cout << "while" << endl;
        sleep(1);
    }
    return 0;
}
