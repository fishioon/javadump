#ifndef READER_H_
#define READER_H_
#include "stdio.h"

class Reader
{
public:
    Reader();
    Reader(unsigned char *buf, size_t len);
    ~Reader();

    inline size_t GetCurrentOffset()
    {
        return m_offset;
    }

    inline size_t SetOffset(size_t offset)
    {
        return m_offset = offset;
    }
    inline size_t GetBufLength()
    {
        return m_len;
    }

    inline bool IsOK()
    {
        return m_offset <= m_len;
    }

    inline size_t AddOffset(int offset)
    {
        if (offset+m_offset > m_len)
        {
            m_offset = m_len;
        }
        else if (offset+m_offset < 0)
        {
            m_offset = 0;
        }
        else
        {
            m_offset += offset;
        }
        return m_offset;
    }

    inline unsigned char GetOffsetU1(size_t offset)
    {
        return offset < m_len ? m_pbuf[offset] : 0;
    }

    inline unsigned int GetOffsetU4(size_t offset)
    {
        if (offset +3 < m_len)
        {
            unsigned int ch1 = m_pbuf[offset];
            unsigned int ch2 = m_pbuf[offset+1];
            unsigned int ch3 = m_pbuf[offset+2];
            unsigned int ch4 = m_pbuf[offset+3];
            return (ch1 << 24) + (ch2 << 16) + (ch3 << 8) + ch4;
        }
        return 0;
    }

    //reset 函数仅当对象已经成功创建时方可调用
    int Reset();
    int Reset(unsigned char *buf, size_t len);

    inline unsigned char ReadU1()
    {
        return m_offset < m_len ? m_pbuf[m_offset++] : 0;
    }
    inline unsigned short ReadU2()
    {
        if (m_offset + 1 < m_len)
        {
            unsigned short ch1 = m_pbuf[m_offset++];
            unsigned short ch2 = m_pbuf[m_offset++];
            return (ch1 << 8) + ch2;
        }
        return 0;
    }
    inline unsigned int ReadU4()
    {
        if (m_offset + 3 < m_len)
        {
            unsigned int ch1 = m_pbuf[m_offset++];
            unsigned int ch2 = m_pbuf[m_offset++];
            unsigned int ch3 = m_pbuf[m_offset++];
            unsigned int ch4 = m_pbuf[m_offset++];
            return (ch1 << 24) + (ch2 << 16) + (ch3 << 8) + ch4;
        }
        return 0;
    }

    inline int ReadArray(unsigned char *arr, int len)
    {
        if (m_offset + len < m_len)
        {
            int i = 0;
            for (i = 0; i < len; i++)
            {
                arr[i] = m_pbuf[m_offset];
                ++m_offset;
            }
            return i;
        }
        return 0;
    }

private:
    size_t m_offset;
    size_t m_len;
    unsigned char *m_pbuf;
};

#endif


