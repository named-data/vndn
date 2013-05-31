/*
 * Copyright (c) 2011 University of California, Los Angeles
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
 * Author: Alexander Afanasyev <alexander.afanasyev@ucla.edu>
 */

#include "ndn-pit-entry-outgoing-face.h"

using boost::posix_time::microsec_clock;

namespace vndn
{

NDNPitEntryOutgoingFace::NDNPitEntryOutgoingFace(Ptr<NDNFace> face)
    : m_face(face)
    , m_sendTime(microsec_clock::local_time())
    , m_retxCount(0)
    , m_waitingInVain(false)
{
}

void NDNPitEntryOutgoingFace::UpdateOnRetransmit()
{
    m_sendTime = microsec_clock::local_time();
    m_retxCount++;
    m_waitingInVain = false;
}

} // namespace vndn
