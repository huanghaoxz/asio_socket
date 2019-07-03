#include <string>
#include <vector>
#include "client.h"
#include "hbla_log4.h"


using namespace std;


HbLog *hb_log = new HbLog();



static int ncount = 0;
boost::recursive_mutex m_mutex;
void receive_data(std::string &message,int bytes,int fd,int &nstatis)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    cout<<"result:" << message <<"  " << bytes << "fd " << fd <<endl;
    cout << "total ncount"<< ncount++<<endl;
    return;
}

vector<CClient*> m_vClient;

void func()
{

    string ip = "10.0.1.106";
    short port = 8888;
    for(int i = 0;i<200;i++)
    {
        CClient* client = new CClient(ip,port,asio::ssl::context::sslv23_client);//应该先设置这些属性，然后在进行连接,所以设计有误
        client->context().set_options(asio::ssl::context::default_workarounds | asio::ssl::context::no_sslv2);
        client->context().set_verify_mode(asio::ssl::context::verify_peer | asio::ssl::context::verify_fail_if_no_peer_cert);
        client->context().load_verify_file("certs/server.crt");
        client->context().use_certificate_chain_file("client_certs/client.crt");
        client->context().use_private_key_file("client_certs/priv.key", asio::ssl::context::pem);
        client->set_receive_data((void*)receive_data);
        client->start();
        for(int j = 0;j<500;j++)
        {
            string *msg = new string("abcd");
            client->send_msg(*msg);
        }
    }
}

int main()
{
    for (int i = 0; i < 5; ++i) {
        boost::thread thread(&func);
        thread.join();
    }
    while (1) {
        sleep(1);
    }
    return 0;
}