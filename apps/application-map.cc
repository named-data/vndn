/*
 * Copyright (c) 2012-2013 Giulio Grassi <giulio.grassi86@gmail.com>
 *                         Davide Pesavento <davidepesa@gmail.com>
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

#include "application-map.h"
#include "corelib/log.h"

#include <cmath>

NS_LOG_COMPONENT_DEFINE("ApplicationMap");


namespace vndn {

std::string ApplicationMap::getRandomCoordinate()
{
    int latSize = floor((locationService.getMaxLat() - locationService.getMinLat())*100000);// 1 meter precision
    int longSize = floor((locationService.getMaxLon() - locationService.getMinLon()) * 100000);
    double lat = ((rand()%latSize) + locationService.getMinLat()*100000)/100000;
    double longitude = ((rand()%longSize) + locationService.getMinLon()*100000)/100000;
    geo::Coordinate coord(lat, longitude);

    NS_LOG_DEBUG("Coordinate randomly picked: " << coord.toString());
    return coord.toString();
}

double ApplicationMap::getDistance(geo::Coordinate c, std::string namePosition)
{
    double distance = c.getDistance(namePosition);
    return distance;
}

}
