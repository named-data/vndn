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

#ifndef GPSINFO_H_
#define GPSINFO_H_

#include "map.h"
#include "map-element.h"
#include "coordinate.h"
#include "network/mac/ll-header.h"    //necessary only to get GPS_STRING_SIZE define

#include <math.h>
#include <algorithm>
#include <stdio.h>
#include <string.h>

namespace vndn {
namespace geo {

/**
 * \brief A GpsInfo stores information about the position and the mobility of a node
 * It contains the actual information, plus the previous (It depends on the gps daemon update frequency, but previous usually mean 1 second before)
 * */
class GpsInfo
{
public:
    /**
     * \brief Creates an empty GpsInfo
     * All the variables are set to the default value
     * */
    GpsInfo();

    /**
     * \brief Creates a GpsInfo, specifying latitude and longitude
     * \param latP latitude (in decimal degree)
     * \param longitudeP longitude (in decimal degree)
     * */
    GpsInfo(double latP, double longitudeP);

    virtual ~GpsInfo();

    /**
     * \brief Calculate the distance between the point indicate by the coordinates stored in the GpsInfo and the coordinates specified by param (stored inside an other GpsInfo
     *
     * It calculates the distance using the haversine formula
     * \param position GpsInfo that stores the position of the second point
     * \return distance between 2 GpsInfo
     * */
    double getDistance(GpsInfo position);

    /**
     * \brief Calculate the distance between the point indicate by the coordinates stored in the GpsInfo and the coordinates specified by param
     *
     * \param gpsLat latitude of the second point (in decimal degree)
     * \param gpsLongitude longitude of the second point (in decimal degree)
     * */
    double getDistance(double gpsLat, double gpsLongitude);


    /**
     * \brief Stores the actual geographical information in the previous section
     *
     * The previous section is overwrite
     * */
    void storeInPrevious();
    
    /**
     * \brief return the mapElement associociated to the actual position (junction, link ...)
     */
    const MapElement * getMapElement() const {return mapPositon;}
    
    /**
     * \brief set the mapElement associated to the actual position
     */
    void setMapElement(const MapElement * el){mapPositon=el;}
    
    /**
     * \brief get the actual coordinate
     */
	Coordinate getCoordinate() const;
    
    /**
     * \brief return a pointer to the char version of the latitude
     */
    const char * getLatChar() const {return latChar;}
    
    /**
     * \brief return a pointer to the char version of the longitude
     */
    const char * getLonChar() const {return longitudeChar;}
    
    /**
     * \brief Return the actual latitude
     */
    double getLat() const {return lat;}
    
    /**
     * \brief Return the actual longitude
     */
    double getLon() const {return longitude;}
    
    /**
     \brief return the actual altitude
     */
    double getAltitude() const {return alt;}
    
    /**
     * \brief Return the actual speed
     */
    double getSpeed() const {return speed;}
    
    /**
     * \brief Return the actual heading
     */
    double getHeading() const {return heading;}
    
    /**
     \brief Set the actual position
     */
    void setPosition(double latit, double lon,  double alt, double speed, double heading);

protected:

    /**
     * \brief(Auxiliare function to calculate distance with greatCircleFormula)
     * It's necessary to avoid some computational errors
     * */
    double asinSafe(double x);

    /**
     * \brief{Calculates the distance  between 2 points expressed in decimal degree using the great crcle formula}
     * Be aware that this function is WORKING PROGRESS
     * aviation formula (http://williams.best.vwh.net/avform.htm#Implement):
     * d=2*asin(sqrt((sin((lat1-lat2)/2))^2 + cos(lat1)*cos(lat2)*(sin((lon1-lon2)/2))^2))
     * \param gpsLat latitude of second point (in decimal degree)
     * \param gpsLongitude longitude of second point (in decimal degree)
     * \return distance
     * */
    double greatCircleFormula(double gpsLat, double gpsLongitude);

    /**
     * \brief{Use haversine formula to calculate distance between 2 point}
     * Calculates the distance between 2 points (giving GPS coordinates) using haversine formula
     * One point is this, the second is given by parameters
     * \param gpsLat latitude of external point
     * \param gpsLongitute longitude of external point
     * \return haversine distance between 2 point (in meters)
     * */
    double haversineFormula(double gpsLat, double gpsLongitude);
    
    const MapElement * mapPositon;
    
    /**Latitude (in decimal degree)*/
    double lat;
    /**Longitude (in decimal degree)*/
    double longitude;
    /**Altitude (not used yet)*/
    double alt;
    /**Speed of the node (not used yet)*/
    double speed;
    /**heading (in degree)*/
    double heading;

    /**Latitude, expressed in char*/
    char latChar[GPS_STRING_SIZE];
    /**Longitude (expressed in char)*/
    char longitudeChar[GPS_STRING_SIZE];

    /**Latitude (relative of previous update) */
    double previousLat;
    /**Longitude (relative of previous update) */
    double previousLongitude;
    /**Altitude (relative of previous update) */
    double previousAlt;
    /**Speed (relative of previous update) */
    double previousSpeed;
    /**Heading  (relative of previous update) */
    double previousHeading;
    /**Latitude chars (relative of previous update) */
    char previousLatChar[GPS_STRING_SIZE];
    /**Longitude chars (relative of previous update) */
    char previousLongitudeChar[GPS_STRING_SIZE];

};

} /* namespace geo */
} /* namespace vndn */

#endif /* GPSINFO_H_ */
