
#include "reader.h"

Reader::Reader()
{
    m_len = 0;
    m_pbuf = 0;
    m_offset = 0;
}

Reader::Reader(unsigned char *buf, size_t len)
{
    m_len = len;
    m_pbuf = buf;
    m_offset = 0;
}
Reader::~Reader()
{
    m_pbuf = 0;
}

int Reader::Reset()
{
    m_len = 0;
    m_pbuf = 0;
    m_offset = 0;
    return 0;
}

int Reader::Reset(unsigned char *buf, size_t len)
{
    m_len = len;
    m_pbuf = buf;
    m_offset = 0;
    return 0;
}
