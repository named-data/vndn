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

#ifndef GEOSTORAGE_H_
#define GEOSTORAGE_H_

#include <utility>

namespace vndn
{

/**
 * \brief A GeoStorage store all the the information about its position
 *
 * A GeoStorage contains info about latitude, longitude, time, direction of the node
 * It's usually used to store the geographical information of the source node of a packet
 * Latitude and longitude are always expressed as decimal degree : Example: 34.0294830
 * */
class GeoStorage
{
public:
    /**
     * \brief Creates an empty GeoStorage
     *
     * All the internal variables are set to the default value (0)
     * */
    GeoStorage();


    /**
     * \brief Create a GeoStorage specifying latitude, and longitude
     * \param latP latitude of the node
     * \param longitudeP longitude of the node
     * */
    GeoStorage(double latP, double longitudeP);

    /**
     * \brief Create a GeoStorage specifying latitude, longitude and time
     * \param latP latitude of the node
     * \param longitudeP longitude of the node
     * \param timestampP timestamp. It indicates when the node has got this information. It's a pair of second, microseconds
     * */
    GeoStorage(double latP, double longitudeP, std::pair<unsigned int, unsigned int> timestampP);

    /**
    * \brief Create a GeoStorage specifying latitude, longitude and heading
    * \param latP latitude of the node
    * \param longitudeP longitude of the node
    * \param headingP heading of the node. Expressend in degree
    * */
    GeoStorage(double latP, double longitudeP, double headingP);

    virtual ~GeoStorage();

    /**
     * \brief Get the latitude
     * \return latitude (in decimal degree)
     * */
    double getLat() const;

    /**
     * \brief Set the latitude
     * \param lat latitude of the node
     * */
    void setLat(double lat);

    /**
     * \brief Get the longitude
     * \return the longitude (in decimal degree)
     * */
    double getLongitude() const;

    /**
     * \brief Set the longitude
     * \param longitude longitude (in decima degree)
     * */
    void setLongitude(double longitude);

    /**
     * \brief Get the timestamp of the geoStorage
     * \return a pair of unsigned int that stores seconds (first element) and microseconds(second element)
     * */
    const std::pair<unsigned int, unsigned int> &getTimestamp() const;

    /**
     * \brief Set the timestamp of the information stored in the GeoStorage
     * \param timestamp pair of unsigned int that stores seconds (first element) and microseconds(second element)
     * */
    void setTimestamp(const std::pair<unsigned int, unsigned int> &timestamp);

    /**
     * \brief Get the heading of the node
     * \return the heading (in degree) stored in the GeoStorage
     * */
    double getHeading();

    /**
     * \brief Set the heading
     * \param head heading (in degree)
     * */
    void setHeading(double head);

protected:
    /**Latitude (in decimal degree)*/
    double lat;
    /**Longitude (in decimal degree)*/
    double longitude;
    /**Heading (in degree)*/
    double heading;
    /**timestamp of the information: pair of seconds, microseconds*/
    std::pair<unsigned int, unsigned int> timestamp;

};

} /* namespace vndn */
#endif /* GEOSTORAGE_H_ */
