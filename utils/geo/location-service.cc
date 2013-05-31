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

#include "location-service.h"
#include "corelib/log.h"

#include <iomanip>

NS_LOG_COMPONENT_DEFINE("geo.LocationService");

namespace vndn {
namespace geo {

LocationService::LocationService()
{
    noValidData();

    map = Map("utils/geo/map/nodes", "utils/geo/map/edges");

    NS_LOG_INFO("Map: minLat = " << map.getMinLat() << ", maxLat = " << map.getMaxLat() << ", minLong = " << map.getMinLon() << ", maxLong = " << map.getMaxLon());
}

LocationService::~LocationService()
{
}

void LocationService::updatePosition(uint8_t * buffer, int len)
{
    if (gpsParser.parseData((char *) buffer, len, &position) == -1) {
        NS_LOG_WARN("parsing data from gps failed");
        return;
    }

    position.getCoordinate().updateUTM();
    updateMapInfo();
    NS_LOG_JSON(vndn::log::CurrentGpsPosition,
                "lat" << position.getLat() <<
                "lng" << position.getLon());
    
    NS_LOG_INFO("new position, map ID: " << position.getMapElement()->getId() <<
                ", latitude: " << position.getLatChar() << ", longitude: " << position.getLonChar());
}

bool LocationService::areInTheSameSegment(Coordinate c1, Coordinate c2)
{
    const MapElement * mapEl1=findLocationInMap(c1);
    const MapElement * mapEl2=findLocationInMap(c2);
    if(mapEl1==NULL || mapEl2==NULL){
        return false;
    }
    return mapEl1->isEqual(*mapEl2);
}

bool LocationService::areWeInTheSameSegment(Coordinate c)
{
    const MapElement * mapEl=findLocationInMap(c);
    if(mapEl==NULL){
        return false;
    }
    return mapEl->isEqual(*(position.getMapElement()));
}

bool LocationService::areWeInTheSameSegment(MapElement el) const
{
    return el.isEqual(*position.getMapElement());
}

const std::string LocationService::findLocationIdInMap(Coordinate c) const
{
    const MapElement * mapEl = findLocationInMap(c);
    if(mapEl!=NULL){
        return mapEl->printId();
    }
    return "";
}

const MapElement * LocationService::findLocationInMap(Coordinate c) const
{
    const Junction *jun = map.findClosestJunction(c,0.001); //	111 m (at the equator)
    if(jun!=NULL){
        if(jun->isValid()){
            if(c.twoPointsDistance(jun->getCoordinate())<SIZEOFINTERSECTION){
                //we are at an intersection
                NS_LOG_DEBUG("we are at an intersection: lat/long:" <<jun->getCoordinate().getLatitude()<<","<<jun->getCoordinate().getLongitude() << " id :"<< jun->printId());
                return jun;
            }
        }
    }
    NS_LOG_DEBUG("findClosestJunction found nothing");
    const Link *link = map.findClosestLink(c,0.001); //	111 m
    if(link==NULL){
        NS_LOG_ERROR("element not found in the map");
        return NULL;
    }
    if(!link->isValid()){
        //ERROR, noting found
        NS_LOG_ERROR("element not found in the map");
        return NULL;
    }
    NS_LOG_DEBUG("we are at a link: " <<std::setprecision(15)<<link->getId() << " id: "<< link->printId());
    return link;
}

void LocationService::getClosestMapToParamAndFarthestToMe(std::string elId, Coordinate c, std::list<std::string> & result) const
{
    std::list<std::string> connections;
    const MapElement *el;
    try{
        el = map.getMapSegmentById(elId);
    } catch (MapException e){
        NS_LOG_WARN( e.what()<<" for id "<< elId);
        return;
    }
    el->getConnection(connections);
    
    std::list<std::string>::iterator it;
    NS_LOG_DEBUG("getClosestMapToParamAndFarthestToMe " << el->printId());
    for (it=connections.begin(); it!=connections.end(); it++) {
        const MapElement * element;
        try{
            element = map.getMapSegmentById(*it);
        } catch (MapException e){
            NS_LOG_WARN( e.what()<<" for id "<< *it);
            continue;
        }

        NS_LOG_DEBUG("distance: from me " <<element->getDistance(position.getCoordinate())<<"; from source:" <<element->getDistance(c));
        if(element->getDistance(c)<=element->getDistance(position.getCoordinate())){
            result.push_back(*it);
        }
    }
}

void LocationService::getNeighbourMapElement(const MapElement * el, std::list<std::string> & result) const
{
    
    el->getConnection(result);  //Link: list of Junction (2); Junction: list of link

    NS_LOG_INFO("getNeighbourMapElement");
    std::list<std::string>::iterator it;
    for(it=result.begin(); it!=result.end(); it++){
        // NS_LOG_DEBUG("getNeighbourMapElement "<<el->toString()<<" "<<el->getId()<<" neighbour: "<< it->toString()<<"; "<<it->getHashCode());
        NS_LOG_DEBUG("getNeighbourMapElement "<<el->toString()<<" "<<el->getId()<<" neighbour: "<< *it);
    }
}

void LocationService::getNeighbourMapElement(std::list<std::string> & result) const
{
    getNeighbourMapElement(position.getMapElement(), result);
}


void LocationService::updateMapInfo()
{
    NS_LOG_DEBUG("updateMapInfo");
    const MapElement * mapEl=findLocationInMap(position.getCoordinate());
    NS_LOG_DEBUG("location in the map found:" << mapEl->toString());
    updateMapInfo(mapEl);
    /*only for debug*/
    NS_LOG_DEBUG("position: "<< position.getMapElement()->toString());
    std::list<std::string> result;
    getNeighbourMapElement(mapEl,result);
    
    std::list<std::string>::iterator it;
    for(it=result.begin(); it!=result.end(); it++){
        // NS_LOG_DEBUG("getNeighbourMapElement "<<mapEl->toString()<<" "<<mapEl->printId()<<"neighbour: "< *it);
    }
}

void LocationService::updateMapInfo(const MapElement * mapEl)
{
    if (mapEl==NULL) {
        return;
    }
    position.setMapElement(mapEl);
}

double LocationService::getDistance (double lat, double lon) const
{
    return position.getCoordinate().twoPointsDistance(lat, lon);
}

void LocationService::noValidData()
{
    //there are some problem with gpsd. We do not have valid gps data
    position.setPosition(DEFAULT_COORDINATE_DOUBLE,DEFAULT_COORDINATE_DOUBLE,DEFAULT_COORDINATE_DOUBLE,DEFAULT_COORDINATE_DOUBLE,DEFAULT_COORDINATE_DOUBLE);
}

}

}
