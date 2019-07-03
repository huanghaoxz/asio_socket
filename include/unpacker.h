//
// Created by huanghao on 19-7-1.
//

#ifndef HBAUDITFLOW_UNPACKER_H
#define HBAUDITFLOW_UNPACKER_H

#include <string>
#include <queue>

using namespace std;
class unpacker
{
public:
    unpacker();
    ~unpacker();
    bool parse_msg_len_body();
    bool parse_msg_fix_len();
private:
};

#endif //HBAUDITFLOW_UNPACKER_H
