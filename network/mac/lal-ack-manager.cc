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

#include "lal-ack-manager.h"
#include "corelib/log.h"

NS_LOG_COMPONENT_DEFINE("LALAckManager");

namespace vndn
{

LALAckManager::LALAckManager()
{
}

LALAckManager::~LALAckManager()
{
}

bool LALAckManager::isAPushProgress(geo::GpsInfo *localNode, geo::GpsInfo &receivedPosition, const GeoStorage &previousHop)
{
    if (receivedPosition.getDistance(previousHop.getLat(), previousHop.getLongitude()) < localNode->getDistance(previousHop.getLat(), previousHop.getLongitude())) {
        NS_LOG_DEBUG("LALAckManager received pkt: no progress");
        return false;
    } else {
        NS_LOG_DEBUG("LALAckManager received pkt: there is a progress");
        return true;
    }
    return false;
}

/*tof(lhdr->lat),atof(lhdr->longitude));
    NS_LOG_DEBUG("GSP received lat: "<<atof(lhdr->lat)<< " long: "<< atof(lhdr->longitude));
    NS_LOG_DEBUG("GSP previous hop: lat: "<<geoStorage.getLat()<< " long: "<<geoStorage.getLongitude());
    NS_LOG_DEBUG("GSP my position: lat: "<<gpsInfo->lat<< " long: "<<gpsInfo->longitude);


*/

} /* namespace vndn */
