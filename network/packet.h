/*
 * Copyright (c) 2012 University of California, Los Angeles
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Author: Lucas Wang <lucas@cs.ucla.edu>
 */

#ifndef PACKET_H_
#define PACKET_H_

#include "buffer.h"
#include "header.h"
#include "corelib/ptr.h"
#include "corelib/simple-ref-count.h"
#include "helper/ndn-header-helper.h"
#include "network/mac/ll-metadata.h"

#include <exception>


namespace vndn
{

class Packet : public SimpleRefCount<Packet>
{
    template <typename T> friend Ptr<T> GetHeader(const Packet &p);
    friend class NDNHeaderHelper;

public:
    static Ptr<Packet> InitFromFD(int fd);
    static Ptr<Packet> InitFromBuffer(const uint8_t *buffer, int len);

    void AddHeader(const Ptr<const Header> &header);
    void AddPayload(const uint8_t *payload, uint32_t payload_size);

    NDNHeaderHelper::Type GetHeaderType() const;

    /*
     * the offset is usually the size of header
     */
    const char *GetPayload(uint32_t offset) const;

    /*
     * the offset is usually the size of the header
     */
    uint32_t GetPayloadSize(uint32_t offset) const;
    uint32_t GetSize (void) const;
    const char *GetRawBuffer() const;

    /*
     * Pointer to the metadata object used by link layer to store information about the position of the source node
     * The double pointer is necessary to avoid any changes at the method signature of some class that take packet as const
     * In the future it will be done in a better way (TODO)
     */
    LLMetadata **llmetadata;
    LLMetadata *llmetadataptr;

    Buffer &getBuffer() {
        return m_buffer;
    }

private:
    uint32_t CopyData(uint8_t *buf, uint32_t len) const;
    Buffer m_buffer;
};

class PacketException : public std::exception
{
    virtual const char *what() const throw() {
        return "PacketException";
    }
};

template <typename T>
Ptr<T> GetHeader(const Packet &p)
{
    Ptr<T> header = Create<T>();
    header->Deserialize(p.m_buffer);
    return header;
}

}

#endif // PACKET_H_
