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

#include "packet.h"

#include <cstdio>
#include <cstring>
#include <unistd.h>

namespace vndn
{

Ptr<Packet> Packet::InitFromFD(int fd)
{
    Ptr<Packet> newPacket = Create<Packet>();

    int bytes_read;
    if ((bytes_read = read(fd, newPacket->m_buffer.GetBuffer(), newPacket->m_buffer.GetCapacity())) < 0) {
        throw PacketException();
    }
    newPacket->m_buffer.SetSize(bytes_read);
    newPacket->llmetadataptr = NULL;
    newPacket->llmetadata = &(newPacket->llmetadataptr);

    return newPacket;
}

Ptr<Packet> Packet::InitFromBuffer(const uint8_t *buffer, int len)
{
    Ptr<Packet> newPacket = Create<Packet>();

    memcpy(newPacket->m_buffer.GetBuffer(), buffer, len);
    newPacket->m_buffer.SetSize(len);
    newPacket->llmetadataptr = NULL;
    newPacket->llmetadata = &(newPacket->llmetadataptr);

    return newPacket;
}

void Packet::AddHeader(const Ptr<const Header> &header)
{
    header->Serialize(m_buffer);
}

void Packet::AddPayload(const uint8_t *payload, uint32_t payload_size)
{
    m_buffer.Write(payload, payload_size);
}

uint32_t Packet::GetSize() const
{
    return m_buffer.GetSize();
}

const char *Packet::GetRawBuffer() const
{
    return m_buffer.GetBuffer(0);
}

NDNHeaderHelper::Type Packet::GetHeaderType() const
{
    return NDNHeaderHelper::GetNDNHeaderType(this);
}

const char *Packet::GetPayload(uint32_t offset) const
{
    return m_buffer.GetBuffer(offset);
}

uint32_t Packet::CopyData(uint8_t *buf, uint32_t len) const
{
    memcpy(buf, m_buffer.GetBuffer(0), len);
    return len;
}

}
