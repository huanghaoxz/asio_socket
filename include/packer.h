//
// Created by huanghao on 19-6-28.
//

#ifndef HBAUDITFLOW_PACKER_H
#define HBAUDITFLOW_PACKER_H

#include <string>
using namespace std;

class packer {
public:
    packer();
    ~packer();
    string pack_msg_fix_length(const string &message,int size);
    string pack_msg_len_body(const string &message);
};


#endif //HBAUDITFLOW_PARSE_MSG_H
