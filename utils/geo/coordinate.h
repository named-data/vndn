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

#ifndef COORDINATE_H
#define COORDINATE_H

#include <exception>
#include <string>

namespace vndn {
namespace geo {

/**
 * \brief It's used to store information about a point in the world. It expresses a coordinate (UTM and lat/long)
 */
class Coordinate
{
public:
    Coordinate();

    /**
     * \brief Creates a coordinate
     *
     * Creates Given a lat/long it creates a Coordinate (both UTM and lat/long information) 
     * \param lat Latitude
     * \param lon Longitude
     */
    Coordinate(double lat, double lon);
    
    /**
     * \brief Creates a coordinate
     *
     * Creates Given a lat/long and UTM information it creates a Coordinate (both UTM and lat/long information)
     * \param latitudeP Latitude
     * \param longitudeP Longitude
     * \param UTMNorthingP UTM nothing coordinate
     * \param UTMEastingP UTM easting coordinate
     * \param UTMzoneP UTM zone
     */
    Coordinate(double latitudeP, double longitudeP, double UTMNorthingP, double UTMEastingP, std::string UTMzoneP);
    
    virtual ~Coordinate();
    
    /**
     * \brief Get latitude
     */
    double getLatitude() const {return latitude;}
    
    /**
     * \brief Get longitude
     */
    double getLongitude() const {return longitude;}

    /**
     * \brief Set latitude
     */
    void setLatitude(double lat){latitude=lat;}    
    
    /**
     * \brief Set longitude
     */
    void setLongitude(double longit){longitude=longit;}
    
    /**
     * \brief Get UTM Northing coordinate
     */
    double getUTMNorthing(){return UTMNorthing;}
    
    /**
     * \brief Get UTM easting coordinate
     */
    double getUTMEasting(){return UTMEasting;}     

    /**
    * \brief Calculates the distance using the haversine formula
    * \param position it stores the position of the second point
    * \return distance between 2 coordinate point
    * */
    double getDistance(Coordinate position) const;

    /**
     * \brief Calculates the distance using the haversine formula
     *
     * \param gpsLat latitude of the second point (in decimal degree)
     * \param gpsLongitude longitude of the second point (in decimal degree)
     * \return distance between 2 coordinate point
     * */
    double getDistance(double gpsLat, double gpsLongitude) const;

    /**
     * \brief calculate distance using haversine formula. It take the coordinate.toString() as input
     */
    double getDistance(std::string coordinateFormat);

    /**
     * \brief Calculates the distance using getDistance (the haversine formula)
     *
     * \param coord1 Coordinate of the second point
     * \return distance between 2 coordinate point
     * */
    double twoPointsDistance(const Coordinate coord1) const;
    
    /**
     * \brief As getDistance (double, double)
     * */
    double twoPointsDistance(double coord1_X, double coord1_Y) const;
    
    /**
     * \brief Calculates the distance between two UTM coordinate
     *
     * \param northing UTM northing of the second point
     * \param easting UTM easting of the second point
     * \return Distance between 2 UTM coordinate
     */
    double twoPointsDistanceUTM(double northing, double easting) const;
    
    /**
     * \brief Calculates the distance between 1 segment and a coordinate
     *
     * \param start first coordinate of the segment
     * \param end second coordinate of the segment
     * \result distance (UTM or ENU) between the segment and this
     */
    double pointToSegmentDist(Coordinate start, Coordinate end) const;
    
    /**
     * \brief Calculates the distance (using math for UTM coordintes) between 1 segment and a coordinate
     *
     * \param start first coordinate of the segment
     * \param end second coordinate of the segment
     * \result distance between the segment and this
     */
    double pointToSegmentDistUTM(Coordinate lineStart, Coordinate lineEnd) const;
    
    /**
     * \brief Calculates the distance between 1 segment and a coordinate
     *
     * Calculates distance between this and a segment from 2 different UTM zone.
     * Not implemented yet!!
     *
     * \param start first coordinate of the segment
     * \param end second coordinate of the segment
     * \result distance between the segment and this
     */
    double pointToSegmentDistENU(Coordinate start, Coordinate end) const;


    /**
     * \brief compare 2 coordinates (this and the one given as parameter)
     */
    bool isEqual(const Coordinate coord) {return coord.getLatitude()==latitude&&coord.getLongitude()==longitude;}

    /**
     * \brief return the string (lat long) 
     */
    std::string toString();
    
    /**
     * \brief Update UMT coordinate based on longitude and latitude
     * */
    void updateUTM();
    
    /**
     * \brief return the coordinate hash code
     */
    int getHashCode() {return hash;}
protected:
    
    /**
     * \brief Calculates the haversine distance between this and the coordinate (double, double) given as parameter
     */
    double haversineFormula(double gpsLat, double gpsLongitude) const;
    
    /**
     * \brief Calculates the great circle formula between 2 coordinated: this and the one given as parameter (as double,double)
     */
    double greatCircleFormula(double gpsLat, double gpsLongitude);
    
    /**
     * \brief Compute the dot product AB . BC
     */
    double dot(double xA, double yA, double xB, double yB, double xC, double yC) const;
    
    /**
     * \brief Compute the distance from AB (segment)  to C (single point)
     */
    double linePointDist(double xA, double yA, double xB, double yB, double xC, double yC) const;
    
    double distance(double xA, double yA, double xB, double yB) const;
    
    double cross(double xA, double yA, double xB, double yB, double xC, double yC) const;
    
    double asinSafe(double x);
    
    
    double latitude;
    double longitude;
    
    /**coordinate hash code: (int)((latitude + 90) * 100000) ^ (int)((longitude + 180) * 100000)*/
    int hash;
    double UTMNorthing;
    double UTMEasting;
    std::string UTMzone;
    
};   

class CoordinateException: public std::exception
{
public:
    CoordinateException(std::string strError) {errorDescription = strError;}

    virtual ~CoordinateException() throw() {}
    
    virtual const char *what() const throw() {return errorDescription.c_str();}

protected:
    /**
     * \brief Error message
     * */
    std::string errorDescription;
};
    
}
}

#endif // COORDINATE_H
