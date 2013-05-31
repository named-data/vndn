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

#ifndef PACKETSTORAGE_H_
#define PACKETSTORAGE_H_

/**
 * interest: name + list of header + list of everything + timer (list of element ordered by time)!!
 * interest and content key: nonce. how can we do for content?
 * but hashed_unique on the namecomponent doesn't work anymore
 * 
 * content:
 * 	unique name
 * 	no header
 * 	no nonce
 * 
 * interest
 * 	not unique name
 * 	header
 * 	nonce
 * 
 * */


#include <stddef.h>
#include <utility>
#include <list>
#include <algorithm>
#include <iostream>
#include <iterator>
#include <string>
#include <string.h>
#include <bits/basic_string.h>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/tag.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/member.hpp>

#include "geo-storage.h"
#include "ack-info.h"
#include "network/ndn-name-components.h"
#include "daemon/ndn.h"
#include "daemon/hash-helper.h"
#include "corelib/ptr.h"

#include "utils/geo/gps-info.h"



namespace vndn
{

/**
 * \brief It stores all the packet sent on the network, until they are considered pending
 *
 * It stores the following information:
 * -packet
 * -number of retransmission
 * -when should be retransmitted next time
 * -partial ack list
 * See linkLayerPktElement for further information about the information stored
 * */
class PacketStorage
{

public:
    /**
     *\brief Creates a apcketStorage ready to use
     *
     * */
    PacketStorage();
    virtual ~PacketStorage();

    struct nameT {};
    struct nonceT {};
    struct timerT {};


    /**
     * \brief elements stored for each packet sent out
     *
     * */
    struct linkLayerPktElement {
        /**NDN name + type (Example CONTENT/local/ ... ) */
        std::string name;
        
        /**Components of the name*/
        Ptr<const NameComponents> components;
        

        /**Nonce: -1 is not used*/
        uint32_t nonce;

        /**Data + NDN-LAL header*/
        uint8_t *data;
        /**Size of the data stored*/
        unsigned int size;
        /**Number of time the pkt has been retransmitted*/
        unsigned int retransmission;
        /**Maximum number of retransmission*/
        unsigned int retransmissionLimit;
        /**Next retransmission (seconds, microseconds)*/
        std::pair<unsigned int, unsigned int> timer;


        /**
         *  \brief Stores the list of ack
         *
         *  It should store (not implemented yet) a list of implicit ack for the packet, with their source position
         *  The object will be deleted by PacketStorage when the entire linkLayerPktElement would have to be deleted
         * */
        AckInfo *ackInfo;

        /**Position information of the node (if the packet is originated locally) or of the previous hop (if the packet has been forwarded) */
        GeoStorage geoInfo;

        /**
         * If true, means that the pkt is originated locally
         * */
        bool localSource;
        //unsigned char * srcMacAddress;//pointer of array? unsigned char srcMacAddress[ETH_ALEN];
        //can I just use a bool: local source? y/n ?? the check would be faster, but without a good ack policy, we can't distinguish a retransmission of the source from an implicit ack of our transmission

        linkLayerPktElement(std::string nameP, Ptr<const NameComponents> componentsP, uint32_t nonceP, unsigned int sizeP, unsigned int retransmissionP, unsigned int retransmissionLimitP, std::pair<unsigned int, unsigned int> timerP, AckInfo *ackInfo/*, Ptr<InterestHeader> header*/):
            name(nameP), components(componentsP), nonce(nonceP), size(sizeP), retransmission(retransmissionP), retransmissionLimit(retransmissionLimitP), timer(timerP), ackInfo(ackInfo)/*, header(header)*/ {}
        //linkLayerPktElement(std::string keyP,std::string nameP, unsigned int sizeP, unsigned int retransmissionP,unsigned int retransmissionLimitP, std::pair<unsigned int, unsigned int> timerP):
        //        key(keyP),name(nameP),size(sizeP),retransmission(retransmissionP),retransmissionLimit(retransmissionLimitP), timer(timerP){}

        /**
         * \brief Defines an order for the timer field of a linkLayerPktElement
         *
         * Boost:multi_index will keep the table ordered by the timer index using this function
         * */
        bool operator<(const linkLayerPktElement &e)const {
            return (timer.first < e.timer.first) ||
                   ((timer.first == e.timer.first) && (timer.second < e.timer.second));
        }
        
        const NameComponents &GetPrefix() const {
			return *components;
		}

    };
    //int insertPkt(void * pkt, int len, int maxNumberOfRetransmission,std::string name,std::pair<unsigned int, unsigned int> timerP, std::list<GeoStorage> * geoIncoming);

    /**
     * \brief inserts a packet in the hash table
     *
     * \param pkt pointer to the packet
     * \param len size of the packet
     * \param maxNumberOfRetransmission max number of retransmission
     * \param name NDN name of the packet (used as key of the table). To avoid conflicts between interest and content with the same name, the type of the packet has to be attached to the name
     * \param components ptr to the name components of the packet
     * \param nonde NDN nonce (-1 for content)
     * \param timerP indicates when the next retransmission should happen
     * \param gpsInfo information about the location of the node (packet generated locally) of of the previous hop(pkt forwarding)
     * \param ackInfo it contains all useful information for the acknowledgment process
     * \return 1 if the packet has been inserted, -1 in case of error
     *
     * */
    int insertPkt(void *pkt, int len, int maxNumberOfRetransmission, std::string name,Ptr<const NameComponents> components, uint32_t nonce, std::pair<unsigned int, unsigned int> timerP, GeoStorage gpsInfo, AckInfo *ackInfo);


    /**
     * \brief Get the element of the storage with the closest retransmission deadline
     * \param el the function will store the element stored
     * \return 1 if an element has been found, -1 otherwise
     * */
    int getFirstDeadline(const linkLayerPktElement  *&el);

    /**
     * \brief Increase the number of retransmission of a packet and update the retransmission deadline
     * \param key name of the packet that has to be updated
     * \param newTime new retransmission deadline
     * \return 1 if everything is ok, -1 in case of error
     * */
    int increaseRetransmissionCounterAndSetNewTimer(std::string key, std::pair <unsigned int, unsigned int> newTime);

    /**
     * \brief Search a packet by key (NDN name + type)
     * \param key NDN name + type (Example: INTEREST/local/traffic)
     * \param el the function will store the element with the required key
     * \return 1 if the element is found, -1 otherwise
     * */
    int getPktByKey(std::string key, const linkLayerPktElement  *&el);

    /**
     * \brief It deletes all entry with a specified name
     * Useful when a content is received and delete all the relative interest is necessary
     * This function is disabled. It will be used again when we will use nonce as key
     * \param name name of entries that have to be deleted
     * \return number of deleted entries
     * */
    int deletePktByName(std::string name);

    /**
     * \brief It deletes a linkLayerPktElement by key (name)
     * \param key NDN name(+type) of packet that has to be deleted
     * \return 1 if everything is ok, -1 if there is no packet with this key
     * */
    int deletePktByKey(std::string key);

    /**
     * \brief deletes the packet with the closest retransmission deadline
     * \param key name (+type) of the packet that has to be deleted
     * \return 1 if everything is ok, -1 if there is no packet
     * */
    int deleteFirstPkt(std::string key);//the key is just a double check. The element that is gonna be deletes is the pkt in the timer order

    /**
     * \brief Update the retransmission deadline of a packet
     * \param key name+type of packet
     * \param newTime new retransmission deadline
     * */
    int setNewTimer(std::string key, std::pair <unsigned int, unsigned int> newTime);
    
    
    /**
     * \brief Search for elements in the storage using the longest prefix match
     * \param components NameComponents which we're looking for
     * \param el address where the linkLayerPktElement that match with components (if founded) will be stored
     * \return 1 if an element is found, 0 otherwise
     * */
    void searchByPrefixMatch(Ptr<const NameComponents> components, std::list<const PacketStorage::linkLayerPktElement *> &matchingElements);

    /**
     * \brief Search for elements in the storage using the longest prefix match and delete it
     * \param components NameComponents which we're looking for
     * \return 1 if an element is found (and deleted), 0 otherwise
     * */    
    int searchAndDeleteByLongestPrefixMatch(Ptr<const NameComponents> components);



    void debugDumpAllStorage();


protected:
    /**
     * \brief Define the structure of the table stored (linkLayerPktElementSet)
     * It's a multi index hash table of linkLayerPktElement
     * The name(+type) of the packet is used as key
     * The second index of the table is the retransmission deadline. It's sorted in chronological order
     * */
    typedef boost::multi_index::multi_index_container <
    linkLayerPktElement,        // The type of the elements stored
    boost::multi_index::indexed_by <   // The indices that our container will support
    boost::multi_index::hashed_unique <
    boost::multi_index::tag<nameT>,
    boost::multi_index::member<linkLayerPktElement, std::string , &linkLayerPktElement::name>
    > ,
    boost::multi_index::ordered_non_unique <
    boost::multi_index::tag<timerT>,
    boost::multi_index::member<linkLayerPktElement, std::pair<unsigned int, unsigned int>, &linkLayerPktElement::timer>
    > ,// map-like index (sorted by name)
    boost::multi_index::hashed_unique <
    boost::multi_index::tag<__ndn_private::i_prefix>,
    boost::multi_index::const_mem_fun <
    linkLayerPktElement,
    const NameComponents &,
    &linkLayerPktElement::GetPrefix
    > ,
    NDNPrefixHash
    >
    >
    > linkLayerPktElementSet;

    /* Restore this if we introduce nonce also inside the content pkt. If this happend, we will use nonce as key
     *    typedef boost::multi_index::multi_index_container<
                  linkLayerPktElement,        // The type of the elements stored
                  boost::multi_index::indexed_by<    // The indices that our container will support
                          boost::multi_index::hashed_unique<
                              boost::multi_index::tag<nonceT>,
                              boost::multi_index::member<linkLayerPktElement, uint32_t ,&linkLayerPktElement::nonce>
                          >,
                  boost::multi_index::ordered_non_unique<
                      boost::multi_index::tag<timerT>,
                      boost::multi_index::member<linkLayerPktElement,std::pair<unsigned int, unsigned int>,
                                       &linkLayerPktElement::timer>
                      >, // map-like index (sorted by name)
                  boost::multi_index::hashed_non_unique<
                      boost::multi_index::tag<nameT>,
                      boost::multi_index::member<linkLayerPktElement, std::string ,&linkLayerPktElement::name>
                  >
               >
             > linkLayerPktElementSet;
    */

    /**
     * \brief Table that stored the pending packet
     * */
    linkLayerPktElementSet storage;

};

} /* namespace vndn */

#endif /* PACKETSTORAGE_H_ */
