/*
 * Copyright (c) 2011-2013 University of California, Los Angeles
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
 *
 * Author: Ilya Moiseenko <iliamo@cs.ucla.edu>
 *         Alexander Afanasyev <alexander.afanasyev@ucla.edu>
 *         Giulio Grassi <giulio.grassi86@gmail.com> 
 *         Davide Pesavento <davidepesa@gmail.com>
 */

#ifndef NDN_CONSUMER_H
#define NDN_CONSUMER_H

#include <arpa/inet.h>
#include <set>
#include <sys/types.h>
#include <list>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/tag.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "corelib/ptr.h"
#include "helper/monitorable.h"
#include "network/ndn-name-components.h"
using namespace boost::posix_time;

namespace vndn
{
class ContentObjectHeader;
class InterestHeader;
class Packet;

/**
 * @ingroup ndn
 * \brief NDN application for sending out Interest packets
 */
class NDNConsumer : public Monitorable
{
public:
    /* inherited from Monitorable */
    virtual void readHandler(EventMonitor &em);
    virtual int getMonitorFd() const;

    /**
     * \brief Default constructor
     * Sets up randomizer function and set default interest lifetime to be 4 seconds
     */
    NDNConsumer(int sockfd);

    /**
     * \brief Event Handler upon receiving a Nack packet
     *
     * Print a message that shows a nack packet is received
     */
    virtual void OnNack(const Ptr<const InterestHeader> &interest, Ptr<Packet> packet);

    /**
     * \brief Event Handler upon receiving a content packet
     *
     * Print a message that shows the received content packet
     */
    virtual void OnContentObject(const Ptr<const Packet> &contentPacket);

   /**
     * \brief send an interest with "name" as name through the file descriptor passed in the constructor
     * \param name name of the interest
     * \return size of the packet sent, -1 if an error occurred
     */
    int SendPacket(NameComponents &name);

    
    /**
     * \brief send an interest with "name" as name through the file descriptor passed in the constructor
     * \param name name of the interest
     * \param minSuffixComponents maxSuffixComponent. see ccn doc(not implemented)
     * \param maxSuffixComponents maxSuffixComponents. see ccn doc(not implemented)
     * \param childSelector childSelector. see ccn doc (not implemented)
     * \param tos type of service (not implemented)
     * \return size of the packet sent, -1 if an error occurred
     */
    int SendPacket(NameComponents &name, int32_t minSuffixComponents, int32_t maxSuffixComponents, bool childSelector, int tos);

    /**
     * \brief read a content and store it in buffer
     * \param buffer buffer where the content will be stored
     * \name it will store the name of the received content
     * \return size of the content, -1 if an error occurred
     * */    
    int read(char *buffer, NameComponents & name);


protected:
    int m_sock_fd; /* file descriptor used to communicate with the daemon*/
    NameComponents  m_interestName;        ///< \brief NDNName of the Interest (use NameComponents)
    time_duration   m_interestLifeTime;    ///< \brief LifeTime for interest packet
    int32_t         m_minSuffixComponents; ///< \brief MinSuffixComponents. See InterestHeader for more information
    int32_t         m_maxSuffixComponents; ///< \brief MaxSuffixComponents. See InterestHeader for more information
    bool            m_childSelector;       ///< \brief ChildSelector. See InterestHeader for more information
    NameComponents  m_exclude;             ///< \brief Exclude. See InterestHeader for more information
};

} // namespace vndn

#endif
