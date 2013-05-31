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

#ifndef APPLICATION_MAP_H_
#define APPLICATION_MAP_H_

#include "utils/geo/coordinate.h"
#include "utils/geo/location-service.h"

#include <string>

namespace vndn
{

class ApplicationMap
{
public:
    std::string getRandomCoordinate();
    double getDistance(geo::Coordinate c, std::string namePosition);

private:
    geo::LocationService locationService;
};

}

#endif // APPLICATION_MAP_H_
