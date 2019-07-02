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
    unpacker(const string & msg);
    ~unpacker();
    bool parse_msg_len_body();
    bool parse_msg_fix_len();

private:
    string m_message;
    size_t m_bytes_transferred;
    size_t m_remain_bytes;
    queue<string> m_read_msg_queue;
};

#endif //HBAUDITFLOW_UNPACKER_H
