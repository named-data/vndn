#ifndef __BUFFER_H
#define __BUFFER_H

#include <stdint.h>

#define BUFLEN 1501

namespace vndn
{
class Packet;

class Buffer
{
    friend class Packet;

public:
    /*
     * Manipulation of an Buffer object should be done via Buffer::Iterator
     */
    class Iterator
    {
        friend class Buffer;
    public:
        bool IsEnd();
        uint8_t ReadU8();
        uint8_t PeekU8();
        uint8_t GetDistance();
    private:
        Iterator(const Buffer &buffer);
        uint32_t m_pos;
        uint32_t m_size;
        char *m_data;
    };

    Buffer();
    ~Buffer();

    Buffer::Iterator Begin() const;
    void WriteU8(uint8_t data);
    void Write(const uint8_t *buffer, uint32_t size);
    uint32_t GetSize() const;
    uint32_t GetCapacity();
    const char *GetBuffer(uint32_t offset) const;

private:
    void SetSize(uint32_t size);
    char *GetBuffer();
    char *m_data;
    uint32_t m_size;
    uint32_t m_capacity;
};

}

#endif
