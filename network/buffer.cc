#include "buffer.h"

#include <assert.h>
#include <string.h>

namespace vndn
{

Buffer::Buffer()
{
    m_size = 0;
    m_capacity = BUFLEN;
    m_data = new char[m_capacity];
    memset(m_data, 0, m_capacity);
}

uint32_t Buffer::GetCapacity()
{
    return m_capacity;
}

uint32_t Buffer::GetSize() const
{
    return m_size;
}

void Buffer::SetSize(uint32_t size)
{
    assert(size <= m_capacity);
    m_size = size;
}

const char *Buffer::GetBuffer(uint32_t offset) const
{
    return m_data + offset;
}

char *Buffer::GetBuffer()
{
    return m_data;
}

Buffer::~Buffer()
{
    delete[] m_data;
}

void Buffer::Write(const uint8_t *buffer, uint32_t size)
{
    assert(m_size + size <= m_capacity);
    memcpy(m_data + m_size, buffer, size);
    m_size += size;
}

void Buffer::WriteU8(uint8_t data)
{
    Write(&data, sizeof(char));
}

Buffer::Iterator Buffer::Begin() const
{
    return Iterator(*this);
}

/////////Buffer::Iterator implemenation/////////////
Buffer::Iterator::Iterator(const Buffer &buffer)
{
    m_size = buffer.GetSize();
    m_pos = 0;
    m_data = buffer.m_data;
}

bool Buffer::Iterator::IsEnd()
{
    return m_pos == m_size;
}

uint8_t Buffer::Iterator::ReadU8()
{
    return m_data[m_pos++];
}

uint8_t Buffer::Iterator::PeekU8()
{
    return m_data[m_pos];
}

uint8_t Buffer::Iterator::GetDistance()
{
    return m_pos;
}

}
