/*
 * Copyright (c) 2012-2013 Giulio Grassi <giulio.grassi86@gmail.com>
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
 */

#ifndef LALACKMANAGERBYDISTANCE_H_
#define LALACKMANAGERBYDISTANCE_H_

#include "lal-ack-manager.h"
#include "ack-info-by-coordinate.h"

namespace vndn
{

/**
 * \brief Manages the NDN-LAL ack policy
 * */
class LALAckManagerByDistance: public LALAckManager
{
public:
    LALAckManagerByDistance();
    virtual ~LALAckManagerByDistance();

    AckInfo *createAckInfo(const geo::LocationService & locationService, GeoStorage *previousHop, llHeader &hdr);
    
    std::pair<bool, int> receivedRetransmission( llHeader *receivedHdr, const PacketStorage::linkLayerPktElement *storedElement, const geo::LocationService & locationService);
};
    
}

#endif
