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

#include "geo-storage.h"

namespace vndn
{

GeoStorage::GeoStorage()
{
    lat = 0;
    longitude = 0;
    heading = 0;
    timestamp.first = 0;
    timestamp.second = 0;
}

GeoStorage::GeoStorage(double latP, double longitudeP, double headingP)
{
    lat = latP;
    longitude = longitudeP;
    heading = headingP;
    timestamp.first = 0;
    timestamp.second = 0;
}

GeoStorage::GeoStorage(double latP, double longitudeP, std::pair<unsigned int, unsigned int> timestampP)
{
    lat = latP;
    longitude = longitudeP;
    timestamp = timestampP;
    heading = 0;
}

GeoStorage::GeoStorage(double latP, double longitudeP)
{
    lat = latP;
    longitude = longitudeP;
    timestamp.first = 0;
    timestamp.second = 0;
    heading = 0;
}

GeoStorage::~GeoStorage()
{
}

double GeoStorage::getLat() const
{
    return lat;
}

void GeoStorage::setLat(double lat)
{
    this->lat = lat;
}

double GeoStorage::getLongitude() const
{
    return longitude;
}

void GeoStorage::setLongitude(double longitude)
{
    this->longitude = longitude;
}

const std::pair<unsigned int, unsigned int> &GeoStorage::getTimestamp() const
{
    return timestamp;
}

void GeoStorage::setTimestamp(
    const std::pair<unsigned int, unsigned int> &timestamp)
{
    this->timestamp = timestamp;
}

double GeoStorage::getHeading()
{
    return heading;
}
void GeoStorage::setHeading(double head)
{
    heading = head;
}

} /* namespace vndn */
