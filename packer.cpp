//
// Created by huanghao on 19-6-28.
//
#include <arpa/inet.h>
#include "packer.h"

packer::packer()
{
}

packer::~packer()
{

}

string packer::pack_msg_len_body(const string &message)
{
    string msg;
    string body = message;
    auto total_len = body.size();
    auto head_len = htons(body.size());
    msg.reserve(total_len);
    msg.append((const char *) &head_len, sizeof(uint16_t));
    msg.append(body);
    return msg;
}

string packer::pack_msg_fix_length(const string &message,int size)
{
    string msg;
    msg.resize(size);
    msg.assign(message);
    msg.append(size-message.size(),0);
    return msg;
}