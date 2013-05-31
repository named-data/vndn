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

#ifndef NDN_FORWARDING_STRATEGY_H
#define NDN_FORWARDING_STRATEGY_H

#include "network/packet.h"
#include "corelib/simple-ref-count.h"

namespace vndn
{

class NDNFace;
class InterestHeader;
class NDNPit;
class NDNPitEntry;
class NDNFibFaceMetric;

/**
 * \ingroup ndn
 * \brief Abstract base class for NDN forwarding strategies
 */
class NDNForwardingStrategy : public SimpleRefCount<NDNForwardingStrategy>
{
public:

    /**
     * @brief Default constructor
     */
    NDNForwardingStrategy ();
    virtual ~NDNForwardingStrategy ();

    /**
     * @brief Base method to propagate the interest according to the forwarding strategy
     *
     * @param pitEntry      Reference to PIT entry (reference to corresponding FIB entry inside)
     * @param incomingFace  Incoming face
     * @param header        InterestHeader
     * @param packet        Original Interest packet
     * @param sendCallback  Send callback
     *
     * @return true if interest was successfully propagated, false if all options have failed
     */
    virtual bool
    PropagateInterest (const NDNPitEntry  &pitEntry,
                       const Ptr<NDNFace> &incomingFace,
                       const Ptr<const InterestHeader> &header,
                       const Ptr<const Packet> &packet) = 0;

    /**
     * @brief Set link to PIT for the forwarding strategy
     *
     * @param pit pointer to PIT
     */
    void
    SetPit (Ptr<NDNPit> pit);

protected:
    /**
     * @brief Propagate interest via a green interface. Fail, if no green interfaces available
     *
     * @param pitEntry      Reference to PIT entry (reference to corresponding FIB entry inside)
     * @param incomingFace  Incoming face
     * @param header        InterestHeader
     * @param packet        Original Interest packet
     * @param sendCallback  Send callback
     * @return true if interest was successfully propagated, false if all options have failed
     *
     * \see PropagateInterest
     */
    bool
    PropagateInterestViaGreen (const NDNPitEntry  &pitEntry,
                               const Ptr<NDNFace> &incomingFace,
                               const Ptr<const InterestHeader> &header,
                               const Ptr<const Packet> &packet);


protected:
    Ptr<NDNPit> m_pit;
};

} //namespace vndn

#endif /* NDN_FORWARDING_STRATEGY_H */
