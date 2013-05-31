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

#ifndef NDN_FLOODING_STRATEGY_H
#define NDN_FLOODING_STRATEGY_H

#include "ndn-forwarding-strategy.h"

namespace vndn
{

class NDNFace;
class InterestHeader;

/**
 * \ingroup ndn
 * \brief Flooding strategy
 *
 * \todo Describe
 */
class NDNFloodingStrategy : public NDNForwardingStrategy
{
public:

    /**
     * @brief Default constructor
     */
    NDNFloodingStrategy ();

    // inherited from  NDNForwardingStrategy
    virtual bool
    PropagateInterest (const NDNPitEntry  &pitEntry,
                       const Ptr<NDNFace> &incomingFace,
                       const Ptr<const InterestHeader> &header,
                       const Ptr<const Packet> &packet);
};

} //namespace vndn

#endif /* NDN_FLOODING_STRATEGY_H */
