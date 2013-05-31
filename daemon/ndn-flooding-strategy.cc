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
 * Author: Ilya Moiseenko <iliamo@cs.ucla.edu>
 *         Davide Pesavento <davidepesa@gmail.com>
 */

#include "ndn-flooding-strategy.h"
#include "network/ndn-interest-header.h"
#include "ndn-l3-protocol.h"
#include "pit/ndn-pit.h"
#include "pit/ndn-pit-entry.h"
#include "corelib/assert.h"
#include "corelib/log.h"
#include "corelib/singleton.h"

#include <boost/ref.hpp>
#include <boost/foreach.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>

namespace ll = boost::lambda;

NS_LOG_COMPONENT_DEFINE("NDNFloodingStrategy");

namespace vndn
{

using namespace __ndn_private;

NDNFloodingStrategy::NDNFloodingStrategy()
{
}

bool NDNFloodingStrategy::PropagateInterest(const NDNPitEntry  &pitEntry,
                                            const Ptr<NDNFace> &incomingFace,
                                            const Ptr<const InterestHeader> &header,
                                            const Ptr<const Packet> &packet)
{
    NS_LOG_FUNCTION_NOARGS();

    // Try to work out with just green faces
    bool greenOk = PropagateInterestViaGreen(pitEntry, incomingFace, header, packet);
    if (greenOk)
        return true;

    int propagatedCount = 0;

    // Populate list of eligible faces
    std::list<Ptr<NDNFace> > faces;
    if (pitEntry.m_fibEntry) {
        BOOST_FOREACH(const NDNFibFaceMetric & metricFace, pitEntry.m_fibEntry->m_faces.get<i_metric>()) {
            if (metricFace.GetStatus() != NDNFibFaceMetric::NDN_FIB_RED)
                faces.push_back(metricFace.GetFace());
        }
    } else {
        NS_LOG_DEBUG("No FibEntry available, forwarding on all faces.");
        const std::vector<Ptr<NDNFace> > &allFaces = Singleton<NDNL3Protocol>::Get()->GetAllFaces();
        for (std::vector<Ptr<NDNFace> >::const_iterator it = allFaces.begin(); it != allFaces.end(); ++it)
            faces.push_back(*it);
    }

    for (std::list<Ptr<NDNFace> >::iterator face = faces.begin(); face != faces.end(); ++face) {
        NS_LOG_DEBUG("Trying " << **face);

        //NDNPitEntryOutgoingFaceContainer::type::iterator outgoing = pitEntry.m_outgoing.find(*face);
        //if (outgoing != pitEntry.m_outgoing.end() && outgoing->m_retxCount >= pitEntry.m_maxRetxCount) {
        //    NS_LOG_DEBUG ("continue (same as previous outgoing)");
        //    continue; // already forwarded before during this retransmission cycle
        //}
        //NS_LOG_DEBUG ("max retx count: " << pitEntry.m_maxRetxCount);

        bool faceAvailable = (*face)->IsBelowLimit();
        if (!faceAvailable) // huh...
            continue;

        m_pit->modify(m_pit->iterator_to(pitEntry), ll::bind(&NDNPitEntry::AddOutgoing, ll::_1, *face));

        // transmission
        (*face)->Send(packet);

        propagatedCount++;
    }

    NS_LOG_INFO("Propagated to " << propagatedCount << " faces.");
    return propagatedCount > 0;
}

} //namespace vndn
