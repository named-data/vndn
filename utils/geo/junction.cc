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

#include <iomanip>

#include "junction.h"
#include "corelib/log.h"

NS_LOG_COMPONENT_DEFINE("geo.Junction");

namespace vndn {
namespace geo {

Junction::Junction()
{
    valid=false;
}

Junction::Junction(Coordinate coord)
{
    this->coord = coord;
    hash = coord.getHashCode();
    valid = true;

    
    std::stringstream ss;
    stringId="(";
    ss << std::setprecision(15)<< coord.getLatitude();
    stringId.append(ss.str());
    ss.str("");
    stringId.append(" ");
    ss << std::setprecision(15)<< coord.getLongitude();
    stringId.append(ss.str());
    stringId.append(")");
}
    
Junction::~Junction()
{
}
    
bool Junction::isLinkPresent(Link l)
{
    std::list<Link>::iterator it;
    for(it=links.begin(); it!=links.end(); it++){
        if(l.isEqual(*it))
            return true;
    }
    return false;
    
}
    
bool Junction::isLinkPresent(std::string l)
{
    std::list<std::string>::iterator it;
    for(it=linksId.begin(); it!=linksId.end(); it++){
        if(l.compare(*it)==0)
            return true;
    }
    return false;
}
    
void Junction::removeLink(Link link)
{
    std::list<Link>::iterator it;
    for(it=links.begin(); it!=links.end(); it++){
        if(link.isEqual(*it)){
            links.erase(it);
            return;
        }
    }
}
    
void Junction::addLink(Link link)
{
    bool found =false;
    std::list<Link>::iterator it;
    for(it=links.begin(); it!=links.end(); it++){
        if(link.isEqual(*it)){
            found = true;
            break;
        }
    }
    if(!found){
        links.push_back(link);
    }
}

    
void Junction::addLinkId(std::string link){
    bool found =false;
    std::list<std::string>::iterator it;
    for(it=linksId.begin(); it!=linksId.end(); it++){
        if(link.compare(*it)==0){
            found = true;
            break;
        }
    }
    if(!found){
        linksId.push_back(link);
    }
}
    
void Junction::removeLinkId(std::string link)
{
    std::list<std::string>::iterator it;
    for(it=linksId.begin(); it!=linksId.end(); it++){
        if(link.compare(*it)==0){
            linksId.erase(it);
            return;
        }
    }
}
    

void Junction::getConnection(std::list<std::string> &result) const
{
    NS_LOG_DEBUG("getting junction connection of "<< toString() << ",id:"<<hash<< "sizeof link list:"<<linksId.size());
    std::list<std::string>::const_iterator it;
    for(it=linksId.begin(); it!=linksId.end(); it++){
        result.push_back(*it);
        NS_LOG_DEBUG("connection: "<< *it);
    }
}

void Junction::getConnection(std::map<std::string,MapElement>& elementTable, std::list<MapElement> &result) const
{
    NS_LOG_DEBUG("getting junction connection of "<< toString() << ",id:"<<hash<< "sizeof link list:"<<links.size());
    std::list<Link>::const_iterator it;
    for(it=links.begin(); it!=links.end(); it++){
        result.push_back((Link)*it);
        NS_LOG_DEBUG("connection: "<< it->toString()<< ",id:"<<it->getId());
    }
    
}
    
void Junction::getConnection(std::map<std::string,MapElement>& elementTable, std::list<const MapElement*> &result) const
{
    NS_LOG_DEBUG("getting junction connection of "<< toString() << ",id:");
    std::list<Link>::const_iterator it;
    for(it=links.begin(); it!=links.end(); it++){
        const Link * tmpLink = &(*it);
        result.push_back(tmpLink);
        NS_LOG_DEBUG("connection: "<< it->toString()<< ",id:"<<it->getId());
    }    
}

bool Junction::hasOneIntersectionInCommon(MapElement el) const
{
    if (isEqual(el)){
        return false;
    }
    std::list<std::string> elKeys;
    el.getKeysList(elKeys);
    std::list<std::string>::iterator it;
    for(it=elKeys.begin(); it!=elKeys.end(); it++){
        if(printId().compare(*it)!=0){
            return true;
        }
    }
    return false;
}

    
std::string Junction::toString() const
{
    return stringId;/*
    std::stringstream ss;
    std::string s="(";
    ss << std::setprecision(15)<< coord.getLatitude();
    s.append(ss.str());
    ss.str("");
    s.append(" ");
    ss << std::setprecision(15)<< coord.getLongitude();
    s.append(ss.str());
    s.append(")");
    return s;*/
}
    
}
}
