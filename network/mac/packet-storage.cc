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

#include "packet-storage.h"
#include "corelib/log.h"

#include <algorithm>
#include <iostream>
#include <iterator>
#include <string>
#include <string.h>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/member.hpp>

using boost::multi_index_container;
using namespace boost::multi_index;

NS_LOG_COMPONENT_DEFINE ("LLPacketStorage");

namespace vndn
{

PacketStorage::PacketStorage()
{
}

PacketStorage::~PacketStorage()
{
    const linkLayerPktElementSet::index<nameT>::type &timer_index = storage.get<nameT>();
    const linkLayerPktElement *el;
    linkLayerPktElementSet::index<nameT>::type::iterator it;
    /**Deleting all the elements present in the storage*/
    for (it = timer_index.begin(); it != timer_index.end(); it++) {
        el = &(*it);
        delete[] el->data;
        delete el->ackInfo;
        storage.erase(it);
    }
}


/*int PacketStorage::insertPkt(void * pkt, int len,int maxNumberOfRetransmission, std::string name,uint32_t nonce, std::pair<unsigned int, unsigned int> timerP)
{
    linkLayerPktElement el(name,nonce,len,1,maxNumberOfRetransmission, timerP);
    el.data=new char[len];
    memcpy(el.data,pkt,len);
    //const linkLayerPktElementSet::nth_index<0>::type& key_index=storage.get<0>();
    std::pair<linkLayerPktElementSet::nth_index<0>::type::iterator,bool> ret = storage.insert(el); 
    if(ret.second)
    {
        return 1;
    }
    return -1;   // remove the function
}*/


//versione with nonce as a key (use it if we introduce a nonce also in content object)
/*int PacketStorage::insertPkt(void * pkt, int len, int maxNumberOfRetransmission,std::string key,std::string name, std::pair<unsigned int, unsigned int> timerP, GeoStorage gpsInfo){
    linkLayerPktElement el(key,name,len,1,maxNumberOfRetransmission, timerP);
    el.geoInfo=gpsInfo;
    el.data=new char[len];
    memcpy(el.data,pkt,len);
    //const linkLayerPktElementSet::nth_index<0>::type& key_index=storage.get<0>();
    std::pair<linkLayerPktElementSet::nth_index<0>::type::iterator,bool> ret = storage.insert(el); //? check return value
    if(ret.second)
    {
        return 1;
    }
    return -1;  // caller has to check
}*/


int PacketStorage::insertPkt(void *pkt, int len, int maxNumberOfRetransmission, std::string name,  Ptr<const NameComponents> components,  uint32_t nonce, std::pair<unsigned int, unsigned int> timerP, GeoStorage gpsInfo, AckInfo *ackInfo)
{
    linkLayerPktElement el(name, components,nonce, len, 1, maxNumberOfRetransmission, timerP, ackInfo);
    el.geoInfo = gpsInfo;
    el.data = new uint8_t[len];
    memcpy(el.data, pkt, len);
    //const linkLayerPktElementSet::nth_index<0>::type& key_index=storage.get<0>();
    std::pair<linkLayerPktElementSet::index<nameT>::type::iterator, bool> ret = storage.insert(el);
    if (ret.second) {
        return 1;
    }
    return -1;
}

int PacketStorage::getFirstDeadline(const linkLayerPktElement  *&el)
{
    NS_LOG_DEBUG("PacketStorage::getFirstDeadline");
    const linkLayerPktElementSet::index<timerT>::type &timer_index = storage.get<timerT>();
    linkLayerPktElementSet::index<timerT>::type::iterator it = timer_index.begin();
    if (it == timer_index.end()) {
        //element not found
        return -1;
    }
    el = &(*it);
    return 1;
}


int PacketStorage::getPktByKey(std::string key, const linkLayerPktElement  *&el)
{
    const linkLayerPktElementSet::index<nameT>::type &key_index = storage.get<nameT>();
    linkLayerPktElementSet::iterator it = key_index.find(key);
    if (it == key_index.end()) {
        //element not found
        return -1;
    }
    el = &(*it);
    return 1;
}

int PacketStorage::deletePktByKey(std::string key)
{
    const linkLayerPktElementSet::index<nameT>::type &key_index = storage.get<nameT>();
    linkLayerPktElementSet::iterator it = key_index.find(key);
    if (it == key_index.end()) {
        //element not found
        return -1;
    }
    const linkLayerPktElement *el = &(*it);
    delete[] el->data;
    delete el->ackInfo;
    storage.erase(it);
    return 1;
}

int PacketStorage::deleteFirstPkt(std::string key)
{
    const linkLayerPktElementSet::index<timerT>::type &time_index = storage.get<timerT>();
    linkLayerPktElementSet::index<timerT>::type::iterator match = time_index.begin();
    if (match == time_index.end()) {
        NS_LOG_DEBUG("DEBUG PacketStorage, deleteFirstPkt, element not found");
        //element not found
        return -1;
    }
    const linkLayerPktElement *el = &(*match);
    if (el->name.compare(key)!=0) {
		NS_LOG_WARN("First element in the queue doesn't have the specified key");
		return -1;
	}
    delete[] el->data;
    delete el->ackInfo;
    storage.get<timerT>().erase(match);
    return 1;
}


//useful if we use nonce in content object and we use it also as key
/*int PacketStorage::deletePktByName(std::string name){
    int numEntries=0;
    const linkLayerPktElementSet::nth_index<nameT>::type& indexByName=storage.get<nameT>();
    linkLayerPktElementSet::nth_index<nameT>::type::iterator it;
    const linkLayerPktElement * el;
    for(it=indexByName.find(name); it!=indexByName.end(); it++){
        el= &(*it); //it's stupid, but otherwise it won't compile
        NS_LOG_DEBUG("deletePktByName, deleted "<< el->name <<", nonce: "<<el->key);
        delete[] el->data;
        storage.erase(it);
        numEntries++;
    }
    return numEntries;
}*/

int PacketStorage::setNewTimer(std::string key, std::pair <unsigned int, unsigned int> newTime)
{
    //const linkLayerPktElementSet::nth_index<nameT>::type &key_index = storage.get<0>();
    const linkLayerPktElementSet::index<nameT>::type &key_index = storage.get<nameT>();
    linkLayerPktElementSet::iterator it = key_index.find(key);
    if (it == key_index.end()) {
        NS_LOG_DEBUG("DEBUG PacketStorage, setNewTimer, element not found");
        //element not found
        return -1;
    }
    linkLayerPktElement updatedEl = *it;
    updatedEl.timer = newTime;
    if (storage.replace(it, updatedEl)) {
        return 1;
    }
    return -1;
}

int PacketStorage::increaseRetransmissionCounterAndSetNewTimer(std::string key, std::pair <unsigned int, unsigned int> newTime)
{
    const linkLayerPktElementSet::index<nameT>::type &key_index = storage.get<nameT>();
    linkLayerPktElementSet::iterator it = key_index.find(key);
    if (it == key_index.end()) {
        NS_LOG_DEBUG("DEBUG PacketStorage, increaseRetransmissionCounter, element not found");
        //element not found
        return -1;
    }
    linkLayerPktElement updatedEl = *it;
    updatedEl.retransmission = updatedEl.retransmission + 1;
    updatedEl.timer = newTime;
    if (storage.replace(it, updatedEl)) {
        return 1;
    }
    return -1;
}

void PacketStorage::debugDumpAllStorage()
{
    const linkLayerPktElementSet::index<timerT>::type &timer_index = storage.get<timerT>();
    const linkLayerPktElement *el;
    linkLayerPktElementSet::index<timerT>::type::iterator it;// = timer_index.begin();
    for (it = timer_index.begin(); it != timer_index.end(); it++) {
        el = &(*it);
        NS_LOG_INFO("Dump Storage: " << el->name << ", timer " << el->timer.first << "." << el->timer.second <<
                  " size: " << el->size << " retransmission remaining: " << el->retransmission);
    }


}

//int PacketStorage::searchByPrefixMatch(Ptr<const NameComponents> components, const linkLayerPktElement  *&el)s
void PacketStorage::searchByPrefixMatch(Ptr<const NameComponents> components, std::list<const PacketStorage::linkLayerPktElement *> &matchingElements )
{
    const NameComponents &name = *components;
    NS_LOG_DEBUG("Trying to match name:" << name);
    for (size_t componentsCount = name.GetComponents().size() + 1; componentsCount > 0; componentsCount--) {
        NameComponents subPrefix(name.GetSubComponents(componentsCount - 1));
        NS_LOG_DEBUG("subPrefix:" << subPrefix);        
        const linkLayerPktElementSet::index<__ndn_private::i_prefix>::type &components_index = storage.get<__ndn_private::i_prefix>();
        linkLayerPktElementSet::index<__ndn_private::i_prefix>::type::iterator match = components_index.find(subPrefix);
        if (match != components_index.end()) {
            NS_LOG_DEBUG("Found entry (longest prefix match) in packet storage.");
            matchingElements.push_back(&(*match));
            //el = &(*match);
            //return 1;
        }
    }
    return;
}


int PacketStorage::searchAndDeleteByLongestPrefixMatch(Ptr<const NameComponents> components)
{
	const NameComponents &name = *components;
    NS_LOG_DEBUG("Trying to match name:" << name);
    for (size_t componentsCount = name.GetComponents().size() + 1; componentsCount > 0; componentsCount--) {
        NameComponents subPrefix(name.GetSubComponents(componentsCount - 1));
        NS_LOG_DEBUG("subPrefix:" << subPrefix);        
        const linkLayerPktElementSet::index<__ndn_private::i_prefix>::type &components_index = storage.get<__ndn_private::i_prefix>();
        linkLayerPktElementSet::index<__ndn_private::i_prefix>::type::iterator match = components_index.find(subPrefix);
        if (match != components_index.end()) {
            NS_LOG_DEBUG("Found entry (longest prefix match) in packet storage.");
            const linkLayerPktElement *el = NULL;
            el = &(*match);
            delete[] el->data;
            delete el->ackInfo;
            storage.get<__ndn_private::i_prefix>().erase(match);
            return 1;
        }
    }
    return 0;
}

} /* namespace vndn */
