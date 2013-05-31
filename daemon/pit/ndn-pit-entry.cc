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

#include "ndn-pit-entry.h"
#include "network/ndn-name-components.h"
#include "corelib/log.h"

#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>

namespace ll = boost::lambda;
using namespace boost::posix_time;

NS_LOG_COMPONENT_DEFINE ("NDNPitEntry");

namespace vndn
{

NDNPitEntry::NDNPitEntry(Ptr<const NameComponents> prefix,
                         const time_duration &expiretime_duration,
                         const NDNFibEntry *fibEntry)
    : m_prefix(prefix)
    , m_fibEntry(fibEntry)
    , m_expireTime(microsec_clock::local_time() + expiretime_duration)
    , m_maxRetxCount(0)
{
    sourceMetadata = NULL;
}

NDNPitEntry::~NDNPitEntry()
{
    NS_LOG_FUNCTION_NOARGS();

    if (sourceMetadata != NULL) {
        free(sourceMetadata);
    }
}

void NDNPitEntry::UpdateLifetime(const time_duration &offsettime_duration)
{
    ptime newExpireTime = microsec_clock::local_time() + offsettime_duration;
    if (newExpireTime > m_expireTime)
        m_expireTime = newExpireTime;
}

NDNPitEntryIncomingFaceContainer::type::iterator NDNPitEntry::AddIncoming(Ptr<NDNFace> face)
{
    NS_LOG_FUNCTION(*face);

    std::pair<NDNPitEntryIncomingFaceContainer::type::iterator, bool> ret =
            m_incoming.insert(NDNPitEntryIncomingFace(face));

    if (!ret.second)
        NS_LOG_WARN("Something is wrong.");

    return ret.first;
}

void NDNPitEntry::RemoveIncoming(Ptr<NDNFace> face)
{
    NS_LOG_FUNCTION(*face);
    m_incoming.erase(face);
}

NDNPitEntryOutgoingFaceContainer::type::iterator NDNPitEntry::AddOutgoing(Ptr<NDNFace> face)
{
    NS_LOG_FUNCTION(*face);

    std::pair<NDNPitEntryOutgoingFaceContainer::type::iterator, bool> ret =
            m_outgoing.insert(NDNPitEntryOutgoingFace(face));

    if (!ret.second) {
        // outgoing face already exists
        m_outgoing.modify (ret.first, ll::bind(&NDNPitEntryOutgoingFace::UpdateOnRetransmit, ll::_1));
    }

    return ret.first;
}

void NDNPitEntry::RemoveAllReferencesToFace(Ptr<NDNFace> face)
{
    NS_LOG_FUNCTION(*face);

    NDNPitEntryIncomingFaceContainer::type::iterator incoming = m_incoming.find(face);
    if (incoming != m_incoming.end())
        m_incoming.erase(incoming);

    NDNPitEntryOutgoingFaceContainer::type::iterator outgoing = m_outgoing.find(face);
    if (outgoing != m_outgoing.end())
        m_outgoing.erase(outgoing);
}

void NDNPitEntry::SetWaitingInVain(NDNPitEntryOutgoingFaceContainer::type::iterator face)
{
    NS_LOG_DEBUG(boost::cref(*face->m_face));

    m_outgoing.modify(face, (&ll::_1)->* &NDNPitEntryOutgoingFace::m_waitingInVain = true);
}

bool NDNPitEntry::AreAllOutgoingInVain () const
{
    bool inVain = true;
    std::for_each (m_outgoing.begin (), m_outgoing.end (),
                   ll::var(inVain) &= (&ll::_1)->* &NDNPitEntryOutgoingFace::m_waitingInVain);

    NS_LOG_DEBUG("inVain = " << inVain);
    return inVain;
}

bool NDNPitEntry::AreTherePromisingOutgoingFacesExcept (Ptr<NDNFace> face) const
{
    bool inVain = true;
    std::for_each(m_outgoing.begin(), m_outgoing.end(),
                  ll::var(inVain) &=
            ((&ll::_1)->* &NDNPitEntryOutgoingFace::m_face == face ||
             (&ll::_1)->* &NDNPitEntryOutgoingFace::m_waitingInVain));

    return !inVain;
}

void NDNPitEntry::IncreaseAllowedRetxCount ()
{
    m_maxRetxCount++;
}

void NDNPitEntry::setSourceMetadata(RequestSourceInfo *rsInfo)
{
    if (sourceMetadata != NULL) {
        free(sourceMetadata);
    }
    sourceMetadata = rsInfo;
}

}
