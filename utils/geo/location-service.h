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

#ifndef LOCATION_SERVICE
#define LOCATION_SERVICE

#include <list>

#include "gps-info.h"
#include "gpsd-parser.h"
#include "map.h"
#include "coordinate.h"

namespace vndn {
namespace geo {

#define SIZEOFINTERSECTION 40

/**
 * \brief This obkject takes care of the localization of a node. 
 * 
 * A LocationService manages the exctraction of information from gpsd, the translation of a map (Tiger map) into a more suitable structure for location service operation
 */
class LocationService
{

public:
    /**
     *\brief It creates the LocationService
     *
     * The LocationService will loaded the tigerMap (the path is written in the code, but this is just a temporary solution)
     * */
    LocationService();

    virtual ~LocationService();

    /**
     * \brief Update car position using information given by gpsd
     * 
     * \param buffer pointer to the gpsd output (the otiginally gpsd output, not parsed yet)
     * \param len size of gpsd output
     * */
    void updatePosition(uint8_t * buffer, int len);
    
    /**
     * \brief Get actual latitude
     */
    double getLatitude() const {return position.getCoordinate().getLatitude();}
    
    /**
     * \brief get actual longitude
     */
    double getLongitude() const {return position.getCoordinate().getLongitude();}
    
    /**
     * Calculates the distance between the actual position and the poind given by parameter (double, double)
     */
    double getDistance (double lat, double lon) const;
    
    /**
     * \brief Return the pointer to the cahr buffer where the string version of the actual latitude is stored
     */
    const char * getLatChar() const {return position.getLatChar();}
    
    /**
     * \brief Return the pointer to the cahr buffer where the string version of the actual longitude is stored
     */
    const char * getLonChar() const {return position.getLonChar();}
    
    /**
     * \brief Return the actual coordinate
     */
    Coordinate getCoordinate(){return position.getCoordinate();}
    
    /**
     * \brief Return the actual mapElement
     */
    const MapElement *getPositionInTheMap() const {return position.getMapElement();}
    const std::string getPositionIdInTheMap() const {return position.getMapElement()->printId();}

    
    /**
     * \brief Check if the 2 coordiantes given as parameters are in the same MapElement
     */
    bool areInTheSameSegment(Coordinate c1, Coordinate c2);
    
    /**
     * \brief Check if the coordinate given as parameter as the actual position are in the same MapElement
     */
    bool areWeInTheSameSegment(Coordinate c);
    
    /**
     * \brief Checks if the actual mapElement is equal to the one given as parameter
     */
    bool areWeInTheSameSegment(MapElement el) const;
    bool areWeInTheSameSegment(std::string elId) const {return elId.compare(position.getMapElement()->printId())==0;}
    
    
    /**
     * \brief get the MapElement that is the closest to c and farthest to the local node (and connected to el)
     *
     * If el is a segment, it can return one of the 2 MapElement-Intersection in el
     * If el is a MapElement-Intersection, it return the list of MapElement-Segment that are connected to el, closer to c and farthest from the local node
     * result will store the list
     * */
    //void getClosestMapToParamAndFarthestToMe(const MapElement &el, Coordinate c, std::list<const MapElement*> & result) const;
    void getClosestMapToParamAndFarthestToMe(std::string elId, Coordinate c, std::list<std::string> & result) const;

    
    /**
     * \brief apply reverse geocoding technique to find out which street is indicated by the Coordinate
     * */
	  const MapElement * findLocationInMap(Coordinate c) const;
    const std::string findLocationIdInMap(Coordinate c) const; //return "" if not found

	
    /**
     * \brief Gets the list of the MapElement connected to the MapElement given as parameter
     *
     * \param result it will store the list of MapElement connected to the MapElement given as parameter
     */
	  //void getNeighbourMapElement(const MapElement & el, std::list<MapElement> & result) const;
    void getNeighbourMapElement(const MapElement * el, std::list<std::string> & result) const;
    
    /**
     * \brief same as getNeighbourMapElement(this.getMapElement)
     */
    void getNeighbourMapElement(std::list<std::string> & result) const;

    /**
     * \brief Set default (invalid) position as actual position (in case gpsd is not responding) 
     */
    void noValidData();
    
    double getMinLat() {return map.getMinLat();}
    double getMaxLat() {return map.getMaxLat();}
    double getMinLon() {return map.getMinLon();}
    double getMaxLon() {return map.getMaxLon();}
	
protected:

    /**
     * \brief search the correct segment in the map and update the node position
     * */
    void updateMapInfo();
    
    /**
     * \brief Updates the actual position with the MapElement given as parameter
     */
    void updateMapInfo(const MapElement * mapEl);
    
    
    /**Actual position*/
    GpsInfo position;
  
    /**It takes care of the gpsd output parsing*/
    GpsdParser gpsParser;  
    
    Map map;
};

}
}

#endif // LOCATION_SERVICE
