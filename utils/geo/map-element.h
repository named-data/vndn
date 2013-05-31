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

#ifndef MAP_ELEMENT
#define MAP_ELEMENT

#include "coordinate.h"

#include <string>
#include <sstream>
#include <algorithm>
#include <list>
#include <map>

namespace vndn {
namespace geo {

/**
 * \brief It's the most elementary element of the Map
 */
class MapElement
{
public:
    /**
     * \brief Creates an empty MapElement
     */
    MapElement();
    virtual ~MapElement();
    
    virtual bool isAnIntersection() const {return false;}
    virtual bool isASegment() const {return false;}
        
    /**
     * \brief check if it's equal to the MapElement given as parameter
     */
    bool isEqual (MapElement el) const {return hash==el.getHashCode();}
    
    /**
     * \brief Get the list to MapElement connected to me
     *
     * \param result it will store the list of MapElement connected to me
     */
    virtual void getConnection(std::map<std::string,MapElement> &elementTable, std::list<MapElement> &result) const {};
    virtual void getConnection(std::map<std::string,MapElement>& elementTable, std::list<const MapElement*> &result) const{};
    virtual void getConnection(std::list<std::string> &result) const {};


    /**
     * \brief Return the MapElement hashCode
     */
    virtual int getHashCode() const {return hash;}
    
    /**
    * \brief return true only if this and el have only one intersection in common
    *
    * A-B / A-C -> true
    * A-B / A -> true
    * A-B / C-D -> false
    * A / A -> false               isEqual
    * A-B / A-B -> false           isEqual
    * */
    virtual bool hasOneIntersectionInCommon(MapElement el) const;
    
    /**
     * \brief Calculates the distance from a coordinate gives as parameter
     */
    virtual double getDistance(Coordinate c) const {return 0;}
    
    virtual std::string printId() const;
    
    virtual std::string toString() const {return "This is a mapElement";}
    
    virtual void getKeysList(std::list<std::string> &result) {}
    
    virtual int getId() const {return hash;}

protected:
    int hash;
    std::string stringId;
};

}
}

#endif
