//
// Created by huanghao on 19-7-1.
//

#include <iostream>
#include "unpacker.h"


string g_temp;
unpacker::unpacker(const string &msg,size_t bytes_transferred) {
    m_message = msg;
    m_bytes_transferred = bytes_transferred;
    m_remain_bytes = bytes_transferred;
    //if()
}

unpacker::~unpacker() {

}

bool unpacker::parse_msg_len_body()
{
    if(m_bytes_transferred < 0)
    {
        return false;
    }
    uint16_t head;
    memcpy(&head, &*std::begin(m_message), sizeof(uint16_t));
    size_t bodylenlen = ntohs(head);
    string msg = m_message;
    string body = msg.substr(sizeof(uint16_t),sizeof(uint16_t)+bodylenlen);
    cout <<
    m_remain_bytes = m_bytes_transferred - sizeof(uint16_t) - bodylenlen;
}