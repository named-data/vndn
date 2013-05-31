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

#include "link.h"
#include "corelib/log.h"
#include "junction.h"

NS_LOG_COMPONENT_DEFINE("geo.Link");

namespace vndn {
namespace geo {

Link::Link()
{
    valid=false;
    stringId="";
    c1=Coordinate();
    c2=Coordinate();
}

Link::Link(std::string id)
{
    this->stringId = id;
    this->streetType = "unknown";
    valid=false;
}

Link::Link(std::string keyNode1, std::string keyNode2, const Coordinate c1, const Coordinate c2)
{
    this->keyNode1 = keyNode1;
    this->keyNode2 = keyNode2;
    this->streetType = "unknown";
    this->c1=c1;
    this->c2=c2;
    boost::hash<std::string> stringHash1, stringHash2;
    hash = stringHash1(keyNode1)^stringHash2(keyNode2);
    std::stringstream ss;
    ss << getHashCode();
    stringId = ss.str();
    valid=true;

}
    
Link::Link(std::string keyNode1, std::string keyNode2, std::string streetName, const Coordinate c1, const Coordinate c2)
{
    this->keyNode1 = keyNode1;
    this->keyNode2 = keyNode2;
    this->streetName = streetName;
    this->streetType = "unknown";
    this->c1=c1;
    this->c2=c2;
    boost::hash<std::string> stringHash1, stringHash2;
    hash = stringHash1(keyNode1)^stringHash2(keyNode2);
    std::stringstream ss;
    ss << getHashCode();
    stringId = ss.str();
    valid=true;


}

Link::Link(std::string keyNode1, std::string keyNode2, std::string streetName, std::string streetType, const Coordinate c1, const Coordinate c2)
{
    this->keyNode1 = keyNode1;
    this->keyNode2 = keyNode2;
    this->streetName = streetName;
    this->streetType = streetType;
    this->c1=c1;
    this->c2=c2;
    boost::hash<std::string> stringHash1, stringHash2;
    hash = stringHash1(keyNode1)^stringHash2(keyNode2);
    std::stringstream ss;
    ss << getHashCode();
    stringId = ss.str();
    valid=true;
}
    
Link::~Link()
{
}
    
double Link::getDistance(Coordinate c) const
{
    return c.pointToSegmentDistUTM(c1,c2);
}
    
void Link::getConnection(std::list<std::string> &result) const
{
    result.push_back(getKeyNode1());
    result.push_back(getKeyNode2());
}
    
void Link::getConnection(std::map<std::string,MapElement>& elementTable, std::list<MapElement> &result) const
{
    const Junction * jun1 = (Junction*)&(elementTable[keyNode1]);
    const Junction * jun2 = (Junction*)&(elementTable[keyNode2]);
    NS_LOG_DEBUG("getting link connection: "<< keyNode1<< ":" <<jun1->toString()<<";"<<
                 keyNode2<<":"<<jun2->toString()<<" hashcode:" << jun1->getHashCode()<<" "<<jun2->getHashCode());
    result.push_back((Junction)*jun1);
    result.push_back((Junction)*jun2);
  //  result.insert(map.lookupJunction(keyNode2));

}

void Link::getConnection(std::map<std::string,MapElement>& elementTable, std::list<const MapElement*> &result) const
{
    const Junction * jun1 = (Junction*)&(elementTable[keyNode1]);
    const Junction * jun2 = (Junction*)&(elementTable[keyNode2]);
    NS_LOG_DEBUG("getting link connection: "<< keyNode1<< ":" <<jun1->toString()<<";"<<
                    keyNode2<<":"<<jun2->toString()<<" hashcode:" << jun1->getHashCode()<<" "<<jun2->getHashCode());
    result.push_back(jun1);
    result.push_back(jun2);
}
    
bool Link::hasOneIntersectionInCommon(MapElement el) const
{
    if (isEqual(el)){
        return false;
    }
    std::list<std::string> elKeys;
    el.getKeysList(elKeys);
    std::list<std::string>::iterator it;
    for(it=elKeys.begin(); it!=elKeys.end(); it++){
        if(keyNode1.compare(*it)!=0){
            return true;
        }
        if(keyNode2.compare(*it)!=0){
            return true;
        }
        
    }
    return false;    
}
    
std::string Link::toString() const
{
    std::stringstream ss;
    std::string s;
    if( stringId.compare("")!=0){
        s.append("HASH:");
        ss << hash;
        s.append(ss.str());
        s.append(",");
    }
    s.append(" NAME:");
    s.append(streetName);
    s.append(", ");
    s.append(keyNode1);
    s.append(" <-> ");
    s.append(keyNode2);
    s.append("]");
    return s;
}

void Link::getKeysList(std::list<std::string> &result)
{
    result.push_back(keyNode1);
    result.push_back(keyNode2);
}
    
}
}
