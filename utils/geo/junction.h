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

#ifndef JUNCTION
#define JUNCTION

#include "coordinate.h"
#include "link.h"
#include "map-element.h"

#include <list>

namespace vndn {
namespace geo {

/**
 * \brief It's a MapElement corresponding to a junction in the map
 *
 * A Junction has its own coordinate and has a list of link (streets) connected to the junction itself
 * A Link is connected to a Junction if the segment of th road starts (or ends) at the Junction (they have one common coordinate)
 */
class Junction : public MapElement
{

public:
    
    /**
     * \brief Creates an empty (and not valid) Junction
     */
    Junction();
    
    /**
     * \brief Creates a Junction at the coordinates given by parameter. The Junction is valid
     */
    Junction(Coordinate coord);
  
    virtual ~Junction();
    
    /**
     * \brief Returns the coordinate of the Junction
     */
    const Coordinate getCoordinate() const {return coord;}
    
    /**
     * \brief Adds a Link to the list of Link connected to the Junction
     */
    void addLink(Link link);
    void addLinkId(std::string link);
    void removeLinkId(std::string link);
    void removeLink(Link link);

    
    /**
     * \brief Returns the list of Link connected to the Junction
     */
    const std::list<Link> &getLinks() const {return links;}
    const std::list<std::string> &getLinksId() const {return linksId;}

    
    /**
     * \brief Get the list of Link (as MapElement) connected to the Junction
     *
     * \param elemenTable map that stores the association mapElement/id (not used)
     * \param result it will store all the Link connected to the Junction
     */
    void getConnection(std::map<std::string,MapElement> &elementTable, std::list<MapElement> &result) const;
    void getConnection(std::map<std::string,MapElement>& elementTable, std::list<const MapElement*> &result) const;
    void getConnection(std::list<std::string> &result) const;

    /**
     * \brief Check if the Link given as parameter is connected to the Junction
     */
    bool isLinkPresent(Link l);
    bool isLinkPresent(std::string l);

    /**
     * \brief Returns the Junction hash code
     */
    int getHashCode() const {return hash;}
    
    /**
     * \brief Checks if the MapElement given as parameter is equal to this
     */
    bool isEqual(MapElement el) const {return el.getHashCode()==hash;}
    
    /**
     * \brief Estabilish an order between the Junction
     *
     * A Junction j1 is < j2 if j1.latitude < j2.latitude 
     * This order does not have a particular meaning. It's necessary only to speed-up some operation on the map
     */
    bool operator<(const Junction &j) const{return coord.getLatitude() < j.getCoordinate().getLatitude();}
    
    /**
     * \brief Returns the string "(latitude longitude)"
     */
    std::string toString() const;
    
    /**
     * \brief Check if the Junction is valid (it's invalid only if it has been created without specifying a coordinate)
     */
    bool isValid() const {return valid;}
    
    /**
     * \brief Caculates the distance between the Junction and the coordinates given as parameter
     *
     * The distance is calculated using the "getDistance" method among coordinates
     */
    double getDistance(Coordinate c) const{return c.getDistance(coord);}
    
    /**
     * \brief Checks if the Junction and the MapElement gives as parameters has one (and only one) intersection in common
     *
     * If the junction are the same, the function returns false
     * If the parameter is a Link connected to the Junction, the function returns true
     * Otherwise, it returns false
     */
    bool hasOneIntersectionInCommon(MapElement el) const;
    
    /**
     * \brief Returns the string "(latitude longitude)"
     */
    std::string printId() const {return toString();}
    
    /**
     * \brief Stores the string given by printId as an element of the list of string given as parameter
     */
    void getKeysList(std::list<std::string> &result) {result.push_back(printId());}
    
    int getId() const {return hash;}
        
    int tmpGetLinksSize() const {return linksId.size();}
protected:
    Coordinate coord;
    /**List of Link connected to the Junction*/
    std::list<Link> links;
    
    std::list<std::string> linksId;

    
    /**Junction hashcode, got using coord.getHashCode()*/
   // int hash;
    
    /**It's true if a coordinate is associated to the Junction, false otherwise*/
    bool valid;
};

}
}

#endif
