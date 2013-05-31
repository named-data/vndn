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

#include "map.h"
#include "corelib/log.h"

#include <fstream>
#include <iomanip>

NS_LOG_COMPONENT_DEFINE("geo.Map");

namespace vndn {
namespace geo {

Map::Map()
{
}

Map::Map(std::string nodesFile, std::string edgesFile)
{
    bool minLatIsSet = false;
    bool maxLatIsSet = false;
    bool minLonIsSet = false;
    bool maxLonIsSet = false;
    //Coordinate minLatMinLong, minLatMaxLong, maxLatMaxLong, maxLatMinLong; //bounding box
    std::ifstream edgesFileStream(edgesFile.c_str(),std::ios::in);
    if(!edgesFileStream.is_open()){
        NS_LOG_ERROR("Failed to open edges file, returning without a map");
        return;
    }
    std::ifstream nodesFileStream(nodesFile.c_str(),std::ios::in);
    if(!nodesFileStream.is_open()){
        NS_LOG_ERROR("Failed to open nodes file, returning without a map");
        edgesFileStream.close();
        return;
    }

    std::string nodeId;
    std::map<std::string, std::string> tmpJunctionDb; //junction id in the file , junction id in our system
    double lat, lon;
    if(nodesFileStream.is_open()){
        while(!nodesFileStream.eof()){
            //populating nodeTable
            nodesFileStream >> nodeId >> lat >> lon;
            if(nodeId.compare("")!=0&&lat!=0&&lon!=0){
                Coordinate coord(lat,lon);
                Junction newJunction(coord);
                nodeTable.insert(std::pair<std::string, Junction> (newJunction.printId(),newJunction));
                tmpJunctionDb.insert(std::pair<std::string,std::string>(nodeId, newJunction.printId()));
                NS_LOG_DEBUG("Reading node: " << nodeId <<" lat: "<<lat<<" long: "<< lon<< " internal id: "<<newJunction.printId());
                if(minLat>coord.getLatitude()|| !minLatIsSet){
                    minLat=coord.getLatitude();
                    minLatIsSet = true;
                }
                else if(maxLat<coord.getLatitude() || !maxLatIsSet){
                    maxLat=coord.getLatitude();
                    maxLatIsSet = true;
                }
                if(minLon>coord.getLongitude()|| !minLonIsSet){
                    minLon=coord.getLongitude();
                    minLonIsSet = true;
                }else if(maxLon<coord.getLongitude() || !maxLonIsSet){
                    maxLon=coord.getLongitude();
                    maxLonIsSet = true;
                }
            }
        }
    }
    
    std::string linkId, startPoint, endPoint;
    std::map<std::string, Junction>::iterator junIt;
    if(edgesFileStream.is_open()){
        while(!edgesFileStream.eof()){
            //populating linkTable
            edgesFileStream >> linkId >> startPoint >> endPoint;
            // NS_LOG_DEBUG("Reading new link : " << linkId << " from " << startPoint << " to " << endPoint);
            junIt = nodeTable.find(tmpJunctionDb[startPoint]);
            if ( junIt==nodeTable.end()){ //start junction not found
                NS_LOG_WARN("Start junction "<< startPoint << " not found. Discarding link "<< linkId);
                continue;
            }
            Junction startJunction = junIt->second;
            junIt = nodeTable.find(tmpJunctionDb[endPoint]);
            if ( junIt==nodeTable.end()){ //start junction not found
                NS_LOG_WARN("End junction "<< endPoint << " not found. Discarding link "<< linkId);
                continue;
            }
            Junction endJunction = junIt->second;
            Link newLink(startJunction.printId(), endJunction.printId(), "", "", startJunction.getCoordinate(), endJunction.getCoordinate());
            
            if(linkTable.find(newLink.printId())==linkTable.end()){
                linkTable.insert(std::pair<std::string, Link> (newLink.printId(), newLink));
                NS_LOG_DEBUG("created link " << newLink.toString() << " id: "<<newLink.printId());
                //Adding link references to the end-point junctions TODO chose between link and linkId
                nodeTable[startJunction.printId()].addLinkId(newLink.printId());
                nodeTable[startJunction.printId()].addLink(newLink);
                nodeTable[endJunction.printId()].addLinkId(newLink.printId());
                nodeTable[endJunction.printId()].addLink(newLink);
            }
        }
    }

    for(junIt=nodeTable.begin(); junIt!=nodeTable.end(); junIt++){
        lookupNode.push_back(junIt->second);
        NS_LOG_DEBUG("number of links per node "<< junIt->second.printId()<< " : "<< junIt->second.tmpGetLinksSize());
    }
    lookupNode.sort();

    //TODO use list of string instead of list of Link in junction ?? is it a good idea. then who manage the junction has to get the id of link and elaborate them
    NS_LOG_INFO("Number of nodes = " << nodeTable.size() << "; number of edges = " << linkTable.size());
    edgesFileStream.close();
    nodesFileStream.close();
}

Map::~Map()
{
}

const MapElement  *Map::getMapSegmentById(std::string elId) const
{
    //TODO check that is impossible to have a junction and a link with the same id (if it's possible, just add a "l" for link in the id, a "j" in the junction)
    std::map<std::string,Junction>::const_iterator junIt =  nodeTable.find(elId);
    if(junIt!=nodeTable.end()){
        return &(junIt->second);
    }
    std::map<std::string,Link>::const_iterator linkIt = linkTable.find(elId);
    if(linkIt!=linkTable.end()){
        return &(linkIt->second);
    }
    NS_LOG_WARN("MapElement "<< elId << " not found");
    throw MapException("Element not found in the map");
}


const Junction *Map::findClosestJunction(Coordinate c, double maxResearchBoxSize) const
{
    //int min, max = lookupNode.BinarySearch(coord);
    Junction jun = Junction(c);
    NS_LOG_DEBUG("junction created");
    std::list<Junction>::const_iterator lowerBound = std::lower_bound(lookupNode.begin(), lookupNode.end(), jun);
    if(jun.isEqual(*lowerBound)){
        NS_LOG_DEBUG("exact match with a junction in the map:"<< (*lowerBound).toString());
        return &(*lowerBound);
    }
    double distance, minDistance=-1;
    std::list<Junction>::const_iterator closestJunction;
    bool found=false;
    //looking at upper values
    NS_LOG_DEBUG("looking at upper values "<<maxResearchBoxSize);
    int numberOfElement=0;
    std::list<Junction>::const_iterator it;
    for(it=lowerBound; it!=lookupNode.end();it++){
        numberOfElement++;
        // NS_LOG_DEBUG("upper values: "<<std::setprecision(10)<<(*it).getCoordinate().getLatitude()<<","<<(*it).getCoordinate().getLongitude()<<"max:"<<jun.getCoordinate().getLatitude()+maxResearchBoxSize);
        if((*it).getCoordinate().getLatitude()>(jun.getCoordinate().getLatitude()+maxResearchBoxSize)){
            break;
        }
        distance = c.twoPointsDistance((*it).getCoordinate());
        if((minDistance==-1)||(distance<minDistance)){
            minDistance=distance;
            closestJunction=it;
            found=true;
        }
        if(distance<50){
            NS_LOG_DEBUG("distance from "<<(*it).toString() <<" is "<< distance);
        }
    }
    NS_LOG_DEBUG("number of element checked: "<<numberOfElement);
    numberOfElement=0;
    //looking at lower values
    NS_LOG_DEBUG("looking at lower values");
    for(it=lowerBound; it!=lookupNode.begin(); it--){
        numberOfElement++;

        if((*it).getCoordinate().getLatitude()<(jun.getCoordinate().getLatitude()-maxResearchBoxSize)){
            break;
        }
        distance = c.twoPointsDistance((*it).getCoordinate());
        if((minDistance==-1)||(distance<minDistance)){
            minDistance=distance;
            closestJunction=it;
            found =true;
        }
        if(distance<50){
            NS_LOG_DEBUG("distance from "<<(*it).toString() <<" is "<< distance);
        }
    }
    NS_LOG_DEBUG("number of element checked: "<<numberOfElement);
    if(found){
        NS_LOG_INFO("found the junction: "<< (*closestJunction).toString()<<" number of links:"<<(*closestJunction).tmpGetLinksSize());
        return  &(*closestJunction);
    }
    NS_LOG_DEBUG("search for junction failed");
    return NULL;
}

const Link * Map::findClosestLink(Coordinate c, double maxResearchBoxSize) const
{
    Junction jun = Junction(c);
    std::list<Junction>::const_iterator lowerBound = std::lower_bound(lookupNode.begin(), lookupNode.end(), jun);
    
    double distance, minDistance=-1;
    std::list<Link>::const_iterator closestLink, linkIterator;
    std::list<Junction>::const_iterator it;
    bool found =false;
    //looking at upper values
    NS_LOG_DEBUG("looking at upper values");
    int numberOfElement=0;
    for(it=lowerBound; it!=lookupNode.end();it++){
        numberOfElement++;
        //NS_LOG_DEBUG("upper values: "<<std::setprecision(10)<<(*it).getCoordinate().getLatitude()<<","<<(*it).getCoordinate().getLongitude()<<"max:"<<jun.getCoordinate().getLatitude()+maxResearchBoxSize);

        if((*it).getCoordinate().getLatitude()>(jun.getCoordinate().getLatitude()+maxResearchBoxSize)){
            break;
        }
        for(linkIterator=(*it).getLinks().begin(); linkIterator!=(*it).getLinks().end(); linkIterator++){
            std::map<std::string,Junction>::const_iterator jun1It = nodeTable.find((*linkIterator).getKeyNode1());
            std::map<std::string,Junction>::const_iterator jun2It = nodeTable.find((*linkIterator).getKeyNode2());
            distance=c.pointToSegmentDistUTM(jun1It->second.getCoordinate(),jun2It->second.getCoordinate());
            if((minDistance==-1)||(distance<minDistance)){
                minDistance=distance;
                closestLink=linkIterator;
                found =true;
            }
            if(distance<50){
                NS_LOG_DEBUG("distance from "<<(*linkIterator).toString() <<" is "<< distance);
            }
        }
        
    }
    NS_LOG_DEBUG("number of element checked: "<<numberOfElement);
    numberOfElement=0;
    //looking at lower values
    NS_LOG_DEBUG("looking at lower values");
    for(it=lowerBound; it!=lookupNode.begin(); it--){
        numberOfElement++;
        // NS_LOG_DEBUG("lower values: "<<std::setprecision(10)<<(*it).getCoordinate().getLatitude()<<","<<(*it).getCoordinate().getLongitude()<<"min:"<<jun.getCoordinate().getLatitude()-maxResearchBoxSize);

        if((*it).getCoordinate().getLatitude()<(jun.getCoordinate().getLatitude()-maxResearchBoxSize)){
            break;
        }
        for(linkIterator=(*it).getLinks().begin(); linkIterator!=(*it).getLinks().end(); linkIterator++){
            std::map<std::string,Junction>::const_iterator jun1It = nodeTable.find((*linkIterator).getKeyNode1());
            std::map<std::string,Junction>::const_iterator jun2It = nodeTable.find((*linkIterator).getKeyNode2());
            distance=c.pointToSegmentDistUTM(jun1It->second.getCoordinate(),jun2It->second.getCoordinate());

            if((minDistance==-1)||(distance<minDistance)){
                minDistance=distance;
                closestLink=linkIterator;
                found =true;
            }
            if(distance<50){
                NS_LOG_DEBUG("distance from "<<(*linkIterator).toString() <<" is "<< distance);
            }
        }
    }
    NS_LOG_DEBUG("number of element checked: "<<numberOfElement);
    if(found){
        NS_LOG_DEBUG("foudn link:" << (*closestLink).toString());
        return &(*closestLink);
    }
    NS_LOG_DEBUG("search for link failed");
    return NULL;
}

void Map::getJunctionConnectedByLink(Link link, std::list<Junction> &result)
{
    //TODO safety check
    result.push_back(nodeTable[link.getKeyNode1()]);
    result.push_back(nodeTable[link.getKeyNode2()]);
}

}

}
