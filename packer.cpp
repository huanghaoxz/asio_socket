//
// Created by huanghao on 19-6-28.
//

#include "packer.h"

packer::packer(const string &message)
{
    m_message = message;
}

packer::~packer()
{

}

string packer::pack_msg_len_body()
{
    string msg;
    string body = m_message;
    auto total_len = body.size();
    auto head_len = htons(body.size());
    msg.reserve(total_len);
    msg.append((const char *) &head_len, sizeof(uint16_t));
    msg.append(body);
    return msg;
}

string packer::pack_msg_fix_length()
{

}