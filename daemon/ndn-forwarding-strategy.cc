/*
 * Copyright (c) 2011-2013 University of California, Los Angeles
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
 *         Ilya Moiseenko <iliamo@cs.ucla.edu>
 *         Davide Pesavento <davidepesa@gmail.com>
 */

#include "ndn-forwarding-strategy.h"

#include "corelib/assert.h"
#include "corelib/ptr.h"
#include "corelib/log.h"
#include "pit/ndn-pit.h"
#include "pit/ndn-pit-entry.h"
#include "network/ndn-interest-header.h"

#include <boost/ref.hpp>
#include <boost/foreach.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>

namespace ll = boost::lambda;

NS_LOG_COMPONENT_DEFINE ("NDNForwardingStrategy");

namespace vndn
{

using namespace __ndn_private;


NDNForwardingStrategy::NDNForwardingStrategy()
{
}

NDNForwardingStrategy::~NDNForwardingStrategy()
{
}

void NDNForwardingStrategy::SetPit(Ptr<NDNPit> pit)
{
    m_pit = pit;
}

bool NDNForwardingStrategy::PropagateInterestViaGreen(const NDNPitEntry  &pitEntry,
                                                      const Ptr<NDNFace> &incomingFace,
                                                      const Ptr<const InterestHeader> &header,
                                                      const Ptr<const Packet> &packet)
{
    NS_LOG_FUNCTION_NOARGS();

    if (!pitEntry.m_fibEntry)
        return false;

    int propagatedCount = 0;

    BOOST_FOREACH(const NDNFibFaceMetric & metricFace, pitEntry.m_fibEntry->m_faces.get<i_metric>()) {
        if (metricFace.GetStatus() == NDNFibFaceMetric::NDN_FIB_RED ||
                metricFace.GetStatus() == NDNFibFaceMetric::NDN_FIB_YELLOW)
            break; //propagate only to green faces

        if (pitEntry.m_incoming.find(metricFace.GetFace()) != pitEntry.m_incoming.end())
            continue; // don't forward to face that we received interest from

        NDNPitEntryOutgoingFaceContainer::type::iterator outgoing =
                pitEntry.m_outgoing.find(metricFace.GetFace());

        if (outgoing != pitEntry.m_outgoing.end() &&
                outgoing->m_retxCount >= pitEntry.m_maxRetxCount) {
            // outgoing face is not associated with pit, but we have
            // reached the maximum refcount
            NS_LOG_DEBUG("retxCount: " << outgoing->m_retxCount << ", maxRetxCount: " << pitEntry.m_maxRetxCount);
            continue;
        }

        bool faceAvailable = metricFace.GetFace()->IsBelowLimit();
        if (!faceAvailable) { // huh...
            // let's try different green face
            continue;
        }

        NS_LOG_INFO("Found green face to forward:" << metricFace.GetFace()->getMonitorFd());

        m_pit->modify(m_pit->iterator_to(pitEntry),
                      ll::bind(&NDNPitEntry::AddOutgoing, ll::_1, metricFace.GetFace()));

        //transmission
        metricFace.GetFace()->Send(packet);

        propagatedCount++;
        break; // propagate only one interest
    }

    return propagatedCount > 0;
}

} //namespace vndn
