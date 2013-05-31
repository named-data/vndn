/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/*
 * Copyright (c) 2011 University of California, Los Angeles
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
 * Author: Ilya Moiseenko <iliamo@cs.ucla.edu>
 *         Alexander Afanasyev <alexander.afanasyev@ucla.edu>
 */

#include "ndn-content-object-header.h"

#include "corelib/log.h"
#include "helper/ndn-encoding-helper.h"
#include "helper/ndn-decoding-helper.h"

NS_LOG_COMPONENT_DEFINE ("ContentObjectHeader");

namespace vndn
{

ContentObjectHeader::ContentObjectHeader ()
{
}

void ContentObjectHeader::SetName (const Ptr<NameComponents> &name)
{
    m_name = name;
}

Ptr<const NameComponents> ContentObjectHeader::GetName () const
{
    if (m_name == 0) throw ContentObjectHeaderException();
    return m_name;
}

uint32_t ContentObjectHeader::GetSize (void) const
{
    // unfortunately, we don't know exact header size in advance
    return NDNEncodingHelper::GetSize (*this);
}

void ContentObjectHeader::Serialize (Buffer &buffer) const
{
    NDNEncodingHelper::Serialize (buffer, *this);
}

uint32_t ContentObjectHeader::Deserialize (const Buffer &buf)
{
    return NDNDecodingHelper::Deserialize (buf, *this); // \todo Debugging is necessary
}

/*
void ContentObjectHeader::Print (std::ostream &os) const
{
    os << "<ContentObject>\n  <Name>" << *GetName () << "</Name>";
    if (m_position.x != 0 && m_position.y != 0 && m_position.z != 0)
        os << "  <Position>" << GetPosition () << "</Position>";

    os << "  <Content>";
}
*/

}
