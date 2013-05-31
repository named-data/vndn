/*
 * Copyright (c) 2011,2012 University of California, Los Angeles
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Alexander Afanasyev <alexander.afanasyev@ucla.edu>
 *         Ilya Moiseenko <iliamo@cs.ucla.edu>
 */

#include "ndn-content-store.h"
#include "corelib/log.h"
#include "network/packet.h"
#include "network/ndn-name-components.h"
#include "network/ndn-interest-header.h"
#include "network/ndn-content-object-header.h"

namespace vndn
{

ContentStore::~ContentStore ()
{
}

//////////////////////////////////////////////////////////////////////

Entry::Entry (Ptr<const ContentObjectHeader> header, Ptr<const Packet> packet)
    : m_header (header)
    , m_packet (packet)
{
}

// Ptr<Packet>
// Entry::GetFullyFormedNdnPacket () const
// {
//   static ContentObjectTail tail; ///< \internal for optimization purposes

//   Ptr<Packet> packet = m_packet->Copy ();
//   packet->AddHeader (*m_header);
//   packet->AddTrailer (tail);
//   return packet;
// }

const NameComponents &
Entry::GetName () const
{
    return *(m_header->GetName ());
}

Ptr<const ContentObjectHeader>
Entry::GetHeader () const
{
    return m_header;
}

Ptr<const Packet>
Entry::GetPacket () const
{
    return m_packet;
}

} // namespace vndn
