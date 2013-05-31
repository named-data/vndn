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

#include "lal-ack-manager-by-distance.h"
#include "corelib/log.h"

NS_LOG_COMPONENT_DEFINE("LALAckManagerByDistance");

namespace vndn
{

using namespace geo;

LALAckManagerByDistance::LALAckManagerByDistance() {}

LALAckManagerByDistance::~LALAckManagerByDistance() {}

AckInfo *LALAckManagerByDistance::createAckInfo(const LocationService & locationService, GeoStorage *previousHop, llHeader &hdr)
{
    if (previousHop != NULL) {
        try{
            Coordinate c (previousHop->getLat(), previousHop->getLongitude());
            AckInfoByCoordinate * ackInfo = new AckInfoByCoordinate(c);
            return ackInfo;
        } catch (CoordinateException e){
            NS_LOG_WARN("Impossible to create the required AckInfo. Exception while creating the coordinate: "<< e.what());
            return NULL;
        }
    } else {
        try{
            Coordinate c (locationService.getLatitude(), locationService.getLongitude());
            AckInfoByCoordinate * ackInfo = new AckInfoByCoordinate(c);
            return ackInfo;
        } catch(CoordinateException e){
            NS_LOG_WARN("Impossible to create the required AckInfo. Exception while getting local coordinate: "<< e.what());
            return NULL;
        }
    }
}

std::pair<bool, int> LALAckManagerByDistance::receivedRetransmission(llHeader *receivedHdr, const PacketStorage::linkLayerPktElement *storedElement, const LocationService & locationService)
{
    std::pair<bool, int> result;
    AckInfoByCoordinate *ack =(AckInfoByCoordinate*) storedElement->ackInfo;
    
    Coordinate packetCoordinate;
    try {
        packetCoordinate = Coordinate(atof(receivedHdr->lat), atof(receivedHdr->longitude));
    } catch (CoordinateException e) {
        NS_LOG_WARN("some problem with coordinates: "<<e.what()<<", considering the pkt as a non-ack");
        result.first = false;
        result.second = 1;
        return result;
    }

    double distanceBetweenMeAndPreviousHop = locationService.getDistance(ack->getCoordinate().getLatitude(), ack->getCoordinate().getLongitude());
    double distanceBetweenPreviousHopAndPacket = packetCoordinate.getDistance(ack->getCoordinate());
    //ack if distance(previous hop, packetCoordinate) > distance (previous hop, me)
    if(distanceBetweenPreviousHopAndPacket > distanceBetweenMeAndPreviousHop){
        //packet is acked
        NS_LOG_DEBUG("Packet acked. distanceBetweenPreviousHopAndPacket = "<< distanceBetweenPreviousHopAndPacket << ", distanceBetweenMeAndPreviousHop: "<< distanceBetweenMeAndPreviousHop);
        result.first = true;
        result.second = 0;
    } else {
        NS_LOG_DEBUG("Packet NOT acked. distanceBetweenPreviousHopAndPacket = "<< distanceBetweenPreviousHopAndPacket << ", distanceBetweenMeAndPreviousHop: "<< distanceBetweenMeAndPreviousHop);
        result.first = false;
        result.second = 1;
    }
    return result;
}

}
