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
    cout<<"result:" << message <<"  bytes  " << bytes << " fd " << fd <<endl;
    cout << "total ncount"<< ncount++<<endl;
    return;
}

vector<CClient*> m_vClient;


void func()
{

    string ip = "172.16.0.183";
    short port = 8888;
    for(int i = 0;i<1;i++)
    {
        //CClient* client = new CClient(ip,port,asio::ssl::context::sslv23_client);//应该先设置这些属性，然后在进行连接,所以设计有误
        CClient* client = new CClient(ip,port);//应该先设置这些属性，然后在进行连接,所以设计有误

        client->set_receive_data((void*)receive_data);
        client->start();

        for(int j = 0;j<1000;j++)
        {
            string *msg = new string("abcd");
            client->send_msg(*msg);
        }
        client->start_listen();
    }
}


int main()
{

    for (int i = 0; i < 1; ++i) {
        boost::thread thread(&func);
        thread.join();
    }
    while (1) {
        cout << "while" << endl;
        sleep(1);
    }
    return 0;
}
