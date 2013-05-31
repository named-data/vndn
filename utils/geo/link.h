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

#ifndef LINK_H
#define LINK_H

#include <boost/functional/hash.hpp>

#include "map-element.h"
#include "coordinate.h"

namespace vndn {
namespace geo {

/**
 * \brief It's a MapElement corresponding to a street (a segment) of a map
 *
 * A Link consists of a couple of coordinates. These coordinates are the coordinates of the Junction connected to the Link
 * A link can have additional information, as type and name of the street
 */
class Link : public MapElement
{
public:
    /**
     * \brief Creates an invalid Link (it doesn not have any coordinates)
     */
    Link();
    
    /**
     * \brief Creates an invalid Link (it doesn not have any coordinates)
     */
    Link(std::string id);
    
    /**
     * \brief Creates an valid Link
     * \param keyNode1 key of one of the Junction connected to the Link
     * \param keyNode1 key of one of the Junction connected to the Link
     * \param Coordinate c1 one of the Coordinate of the Link
     * \param Coordinate c1 one of the Coordinate of the Link
     */
    Link(std::string keyNode1, std::string keyNode2, const Coordinate c1, const Coordinate c2);
    
    /**
     * \brief Creates an valid Link
     * \param keyNode1 key of one of the Junction connected to the Link
     * \param keyNode1 key of one of the Junction connected to the Link
     * \param streetName name of the street
     * \param Coordinate c1 one of the Coordinate of the Link
     * \param Coordinate c1 one of the Coordinate of the Link
     */
    Link(std::string keyNode1, std::string keyNode2, std::string streetName, const Coordinate c1, const Coordinate c2);
    
    /**
     * \brief Creates an valid Link
     * \param keyNode1 key of one of the Junction connected to the Link
     * \param keyNode1 key of one of the Junction connected to the Link
     * \param streetName name of the street
     * \param streetType Type of the street
     * \param Coordinate c1 one of the Coordinate of the Link
     * \param Coordinate c1 one of the Coordinate of the Link
     */
    Link(std::string keyNode1, std::string keyNode2, std::string streetName, std::string streetType, const Coordinate c1, const Coordinate c2);
    
    virtual ~Link();

    /**
     * \brief Check if the mapElement given as parameter and "this" are the same Link
     */
    bool isEqual(MapElement el) const {return hash==el.getHashCode();}
    
    /**
     * \brief Get the Link hash code
     */
    int getHashCode() const {return hash;}
    
    /**
     * \brief Return the lenght of the Link
     */
    double getSegmentSize() {return segmentSize;}
    
    /**
     * \brief Set the lenght of the Link
     */
    void setSegmentSize(double value) {segmentSize = value;}
    
    /**
     * \brief Return the string "ID:, NAME: key of Junction 1 <-> keyOfJunction2 ]"
     */
    std::string toString()const;
    
    /**
     * \brief Return the key of the Junction 1
     */
    std::string getKeyNode1() const {return keyNode1;}
    
    /**
     * \brief Return the key of the Junction2
     */
    std::string getKeyNode2() const {return keyNode2;}
    
    /**
     * \brief Calculates the distance between the Link and the Coordinate given as parameter
     */
    double getDistance(Coordinate c) const;
    
    /**
     * \brief Get the list of Junction connected to the Link
     *
     * \param elementTable map used to get the Junction indicated by keynode1 and keynode2
     * \param result it will store the list of Junction connected to the Link
     */
    void getConnection(std::map<std::string,MapElement>& elementTable, std::list<MapElement> &result) const;
    void getConnection(std::map<std::string,MapElement>& elementTable, std::list<const MapElement*> &result) const;
    void getConnection(std::list<std::string> &result) const;

    /**
     * \brief Check if the Link is valid
     */
    bool isValid() const {return valid;}
    
    
    /**
     * \brief Check if the MapElement given as parameter has 1 (and only 1) intersection in common
     */
    bool hasOneIntersectionInCommon(MapElement el) const;
    
    /**
     * \brief Return the id of the Link
     */
    std::string printId() const {return stringId;}
    
    /**
     * \brief Stores in the param the keys of the Junction connected to the Link
     */
    void getKeysList(std::list<std::string> &result);

    /**
     * \brief Set the Link Id
     */
    void setId(std::string val) { stringId=val;}
    
    /**
     * \brief Return the Link id"
     */
    //std::string getId() {return id;}
    int getId() const {return hash;}
    
protected:
    /**key of one of the Junction connected to the Link*/
    std::string keyNode1;
    /**key of one of the Junction connected to the Link*/
    std::string keyNode2;
    /**Name of the street*/
    std::string streetName; //not used
    std::string streetType; //not used
    Coordinate c1,c2;
    
    double segmentSize;
    bool valid;
    
};

}
}

#endif
