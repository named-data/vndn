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

#ifndef LALACKMANAGER_H_
#define LALACKMANAGER_H_

#include "ack-info.h"
#include "ll-header.h"
#include "packet-storage.h"
#include "geo-storage.h"
#include "utils/geo/gps-info.h"
#include "utils/geo/location-service.h"

namespace vndn
{

/**
 * \brief Manages the NDN-LAL ack policy
 * */
class LALAckManager
{
public:

    LALAckManager();
    virtual ~LALAckManager();

    /**
     * \brief it creates and set properly the ack struct used to manage the acknowledgment process of a outgoing packet
     *
     * */
    virtual AckInfo *createAckInfo(const geo::LocationService & locationService, GeoStorage *previousHop, llHeader &hdr) = 0;

    /**
     * \brief a retransmission of a pending packet has been heard. Check for push progress, if needed add the ack to the list
     *
     * \return pair of < is the received packet an ack? , number of ack still required (0 if the pending packet is completly acked>
     * */
    virtual std::pair<bool, int> receivedRetransmission(llHeader *receivedHdr, const  PacketStorage::linkLayerPktElement *storedElement, const geo::LocationService & locationService) = 0;

protected:
    bool isAPushProgress(geo::GpsInfo *localNode, geo::GpsInfo &receivedPosition, const GeoStorage &previousHop );  //has in LLNomPolicy   it can be implemented in this class!


    geo::GpsInfo *localNodeLocation;

};

} /* namespace vndn */
#endif /* LALACKMANAGER_H_ */
