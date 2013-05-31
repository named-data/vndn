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

#ifndef MAP
#define MAP

#include <string>
#include <sstream>
#include <algorithm>
#include <list>
#include <map>
#include "coordinate.h"
#include "junction.h"
#include "link.h"
#include "map-element.h"

namespace vndn {
namespace geo {


class MapException: public std::exception
{
public:
    MapException(std::string strError) : errorDescription(strError) {}

    virtual ~MapException() throw() {}

    virtual const char *what() const throw() { return errorDescription.c_str(); }

protected:
    /**
     * \brief Error message
     * */
    std::string errorDescription;
};


/**
 * \brief It stores the Tiger Map (relative of the actual position of the car)
 */
class Map
{
public:
    
    /**
     * \brief Creates an empy map
     */
    Map();
    
    
    /**
     * \brief Create a map from a list od nodes (nodesFile) and edges(edgefile)
     *
     * Edge format: linkId startNodeId endNodeId
     * nodes format: nodeId latitude longitude
     */
    Map(std::string nodesFile, std::string edgesFile);
    
    virtual ~Map();
    
    //can throw MapException if element not found
    const MapElement *getMapSegmentById(std::string elId) const;


    /**
     * \brief Find the closest Junction to a Coordinate, within a distance
     *
     * \param c Coordinate
     * \param maxResearchBoxSize set the size of the research box ( be careful, a research box too big will speed-down the research)
     * \return The pointer to the closest Junction to c ( NULL if there no such Junction)
     */
    const Junction *findClosestJunction(Coordinate c, double maxResearchBoxSize) const;
    

    /**
     * \brief Find the closest Link to a Coordinate, within a distance
     *
     * \param c Coordinate
     * \param maxResearchBoxSize set the size of the research box ( be careful, a research box too big will speed-down the research)
     * \return The pointer to the closest Link to c ( NULL if there no such Junction)
     */
    const Link *findClosestLink(Coordinate c, double maxResearchBoxSize) const;
    
    /**
     * \brief Get the list of Junction connected to the Link given as parameter
     *
     * \param result it will store the list of Junction connected to the link
     */
    void getJunctionConnectedByLink(Link link, std::list<Junction> &result);
    
    /**
     * \brief Search in the map the Junction with a specific key
     *
     * All the Junction are stored in a map of nodes and for each Junction has been associated a string as a key when the Junction has been created
     * \param key key of the Junction that you're looking for
     */
    Junction &lookupJunction(std::string key) {return nodeTable[key];}
    
    /**
     * \brief Return the map where all the Junction are stored
     */
    const std::map<std::string,Junction> &getNodeTable() const {return nodeTable;}
    
    double getMinLat() {return minLat;}
    double getMaxLat() {return maxLat;}
    double getMinLon() {return minLon;}
    double getMaxLon() {return maxLon;}
    
protected:
    /**Map of Junction that are in the map*/
    std::map<std::string,Junction> nodeTable;
    
    /**Map of Link that are in the map*/
    std::map<std::string,Link>  linkTable;

    /**List (ordered) of Junction that are in the map*/
    std::list<Junction> lookupNode;
    /**List of all the Link that are in the map*/
    std::list<Link> lookupLinks;
    
    std::map<std::string, std::list<std::string> > iNodes;
    
    double maxSegmentLengthAllowed;
    double minLat, maxLat, minLon, maxLon;

};

}
}

/*
 how to read a map:
 get a open street file map (.osm)
 
 netconvert --osm-files MAP_NAME.osm -o OUTPUT_FILE_NAME.net.xml --osm.discard-tls --remove-edges.by-vclass hov,taxi,bus,delivery,transport,lightrail,cityrail,rail_slow,rail_fast,motorcycle,bicycle,pedestrian,rail_slow,rail_fast,bicycle,pedestrian

 get list of junctions:
     grep junction .NET.XML_FILE_GENERATED_BY_NETCONVERT | grep priority | awk '{print $2}' | awk -F "id=" '{print $2}' > file
     grep -f file MAP_NAME.osm  | grep id  | awk -F "\"" '{print $2 " " $4" " $6}'    -> list of nodeId lat/long
 get list of edges:
     grep -F "<edge id"  NET.XML_FILE_GENERATED_BY_NETCONVERT  | grep priority | awk -F "\"" '{print $2 " " $4" " $6}'
     -> edgeId startJunction endJunction
 */

#endif 
