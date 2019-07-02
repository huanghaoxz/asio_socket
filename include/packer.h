//
// Created by huanghao on 19-6-28.
//

#ifndef HBAUDITFLOW_PACKER_H
#define HBAUDITFLOW_PACKER_H

#include <string>
using namespace std;

class packer {
public:
    packer(const string &message);
    ~packer();
    pack_msg_fix_length();
    pack_msg_len_body();
private:
    string m_message;
};


#endif //HBAUDITFLOW_PARSE_MSG_H
