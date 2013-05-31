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

#include "gps-info.h"
#include "corelib/log.h"

NS_LOG_COMPONENT_DEFINE("geo.GpsInfo");

#define DEG2RAD (M_PI / 180.0)

namespace vndn {
namespace geo {

GpsInfo::GpsInfo()
{
    alt = 0;
    lat = 0;
    longitude = 0;
    speed = 0;
    heading = 0;
    memset(latChar, 0 , GPS_STRING_SIZE);
    memset(longitudeChar, 0 , GPS_STRING_SIZE);
    sprintf(latChar, "0.0");
    sprintf(longitudeChar, "0.0");
    previousAlt = 0;
    previousLat = 0;
    previousLongitude = 0;
    previousSpeed = 0;
    previousHeading = 0;
}

GpsInfo::GpsInfo(double latP, double longitudeP)
{
    lat = latP;
    longitude = longitudeP;
    alt = 0;
    speed = 0;
    heading = 0;
    memset(latChar, 0 , GPS_STRING_SIZE);
    memset(longitudeChar, 0 , GPS_STRING_SIZE);
    sprintf(latChar, "%f", lat);
    sprintf(longitudeChar, "%f", longitude);
    previousAlt = 0;
    previousLat = 0;
    previousLongitude = 0;
    previousSpeed = 0;
    previousHeading = 0;
}

GpsInfo::~GpsInfo()
{
}

void GpsInfo::storeInPrevious()
{
    previousAlt = alt;
    previousLat = lat;
    previousLongitude = longitude;
    previousSpeed = speed;
    previousHeading = heading;
}


double GpsInfo::asinSafe(double x)
{
    return asin(std::max(-1.0, std::min(x, 1.0)));
}

//great circle formulae
double GpsInfo::greatCircleFormula(double gpsLat, double gpsLongitude)
{
    return 2 * asinSafe(sqrt(pow((sin((gpsLat - lat) / 2)), 2) +
                             cos(gpsLat) * cos(lat) * pow((sin((gpsLongitude - longitude) / 2)), 2)));
}


double GpsInfo::haversineFormula(double gpsLat, double gpsLongitude)
{

    double dlong = (gpsLongitude - longitude) * DEG2RAD;
    double dlat = (gpsLat - lat) * DEG2RAD;
    double a = pow(sin(dlat / 2.0), 2) + cos(lat * DEG2RAD) * cos(gpsLat * DEG2RAD) * pow(sin(dlong / 2.0), 2);
    double c = 2 * atan2(sqrt(a), sqrt(1 - a));
    return 1000 * 6367 * c;
}

double GpsInfo::getDistance(double gpsLat, double gpsLongitude)
{
    return haversineFormula(gpsLat, gpsLongitude);
}

double GpsInfo::getDistance(GpsInfo gps)
{
    return getDistance(gps.lat, gps.longitude);
}

Coordinate GpsInfo::getCoordinate() const
{
    try{
        return Coordinate(lat,longitude);
    } catch (CoordinateException e) {
        NS_LOG_WARN("some problem with coordinates "<< e.what()<<" returning coordinate(0,0)");
        return Coordinate(0,0);
    }
    
}

void GpsInfo::setPosition(double latit, double lon,  double alt, double speed, double heading)
{
    this->lat = latit;
    this->longitude = lon;
    this->alt = alt;
    this->speed = speed;
    this->heading = heading;
    memset(latChar, 0 , GPS_STRING_SIZE);
    memset(longitudeChar, 0 , GPS_STRING_SIZE);
    if (lat==DEFAULT_COORDINATE_DOUBLE) {
        memcpy(latChar, DEFAULT_COORDINATE_CHAR, strlen(DEFAULT_COORDINATE_CHAR));    
    } else {
        sprintf(this->latChar, "%#1.6f", this->lat);
    }
    if (lon==DEFAULT_COORDINATE_DOUBLE) {
        memcpy(longitudeChar, DEFAULT_COORDINATE_CHAR, strlen(DEFAULT_COORDINATE_CHAR));    
    } else {
        sprintf(this->longitudeChar, "%#1.6f", this->longitude);
    }
}

} /* namespace geo */
} /* namespace vndn */
