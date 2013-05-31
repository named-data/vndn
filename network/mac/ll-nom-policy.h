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

#ifndef LLNOMPOLICY_H_
#define LLNOMPOLICY_H_

//#define TEST_GPS_MOVING
#define LAL_STATISTICS

#include "ll-policy.h"
#include "ll-metadata-80211-adhoc.h"
#include "ll-header.h"
#include "ndnsock/ndn-raw-socket.h"
#include "ndnsock/ndn-socket-exception.h"

#include "lal-ack-manager-by-distance.h"
#include "lal-stats.h"
#include "geo-storage.h"

#include "helper/ccnb-parser/ccnb-parser-common.h"
#include "helper/ndn-header-helper.h"
#include "network/ndn-interest-header.h"
#include "network/ndn-content-object-header.h"
#include "network/packet.h"

#include <boost/foreach.hpp>
#include <algorithm>
#include <iostream>
#include <sys/time.h>
#include <stddef.h>
#include <stdio.h>
#include <utility>

namespace vndn
{

using geo::LocationService;

/**
 * \brief Defines the link layer policy for 802.11 ad-hoc (specialized for VANET). Check the Nom paper (Rapid Traffic Information Dissemination Using Named Data)
 *
 * Briefly, each outgoing packet is transmitted several time if not acked by an implicit ack ( an other node forward the same packet).
 * It implements timers to avoid collision (Tca) and to prefer farther node from closer node in the forwarding process (Tgap), to decrease the number of packets sent out
 * This policy needs geographical information about the nodes (GPS)
 */
class LLNomPolicy : public LLPolicy
{
public:

#define interestKey "INTEREST"
#define dataKey "DATAOBJECT"

    //policy configuration (as specified in nom paper: Rapid Trafﬁc Information Dissemination Using Named Data - L. Wang, A. Afanasyev, R. Kuntz, R. Vuyyuru, R. Wakikawa, L. Zhang)

    /**max number of retransmission of a packet*/
    static const int maxRetransmissionNumber = 8;
    /**Delay for second retransmission (seconds)*/
    static const unsigned int secFirstRetransmission = 0;
    /**Delay for second retransmission (micro seconds)*/
    static const unsigned int usecFirstRetransmission = 500000;//50000; //50 ms


    /**
     * Tca has an uniform distribution between 0 and maxTca
     */
    static const unsigned int maxTca = 2000;

    /**
     * Minimum delay for a node that is next to the previous hop (microseconds)
     * */
    static const unsigned int Tdist = 5000;

    /**
     * maximum range for wireless transmission (meters)
     * */
    static const unsigned int Dmax = 150;

    /**
     * \brief Creates a LLPolicy that implements the Nom paper policy
     * */
    LLNomPolicy();

    virtual ~LLNomPolicy();

    /*
     * \brief The upper layer asked to send a packet out. LLNomPolicy is going to apply its rules and decide what to do with the packet
     *
     * LLNomPolicy is going to check if the packet is already pending. If this is true (also if nonces are differet), the packet is discarded (this policy could change in the future).
     * If the packet is not present, it will be sent out using broadcast communication.
     * The packet will be encapsulated with a NDN-LAL adaptation header (LLHeader). Tca and Tgap will be applied. The packet will be stored for further retransmission if no implict ack will be received
     * \param pkt packet that has to be sent out (NDN packet)
     * \param len address of the length of the pkt. At the begin is stores the NDN packet, at the end it will store the size of NDN-LAL adatpation header + NDN portion
     * \param metadata address of metadata
     * \return  what LLNomPolicy has decided about the packet (send the packet or  discard it)
     * */
    int addOutgoingPkt(uint8_t pkt[], int *len, LLMetadata80211AdHoc *metadata, const LocationService & locationService);


    /**
     * \brief A pkt that has been received from the network. LLNomPolicy has to apply the NDN-LAL policies
     *
     * LLNomPolicy will check if the pkt can be considered as implicit ack (in this case, the relatice entry in the pending table will be deleted).
     * LLNomPolicy will decided if the pkt needs to be transmitted at the upper layer (NDN)
     * \param pkt pointer to the packet that has been received (still with NDN-LAL adaptation header). The function will store the NDN packet without NDN-LAL adaptation header in it.
     * \param len it stores the size of the received packet. After, it will store the size of the NDN packet without NDN-LAL adaptation header
     * \return  what LLPolicy has decided about the packet (send it to the upper layer or discard it).
     * */
    int pktFromNetwork(uint8_t pkt[], int *len, void *&dataWithoutLLHeader, const LocationService & locationService);


    /**
     * \brief  Tells when the next packet has to be retransmitted
     *
     * The values that tell when a packet has to be retransmitted are in the same form of a timeval (second and microseconds)
     * \param sec the function will store when the next packet has to be retransmitted ( seconds)
     * \param usec the function will store when the next packet has to be retransmitted (micro seconds)
     * \return 1 if there is at least a packet that has to be retransmitted. NOTIMER otherwise
     * */
    int getNextDeadline(long int *sec, long int *usec);


    /**
     * \brief Get the next packet that has to be retransmitted
     *
     * It gets the next packet scheduled for a retransmission
     * If the packet is at the last retransmission, the entry in the pending table will be deleted
     * \param ptrData the function will store the packet scheduled for the retransmission
     * \return size of the packet (it can't be greater than MAXNETWORKPKTSIZE)
     * */
    int getPktForRetransmission(uint8_t ptrData[], const LocationService &locationService);

    /**
     * \brief Give to LLNomPolicy the reference to the LLUpperLayerCommunicationService that can be used to communicate with NDN layer (through NDNNetDeviceFace)
     *
     * \param upperLayerComServ pointer to the LLUpperLayerCommunicationService
     * */
    void setLLupperLayerCommunication(LLUpperLayerCommunicationService *upperLayerComServ);

    void printStatistic();

protected:
    /**
     * \brief It calculates the deadline for the next retransmission of the pkt (Tretx)
     *
     * This version of LLNomPolicy used a fixed value as Trext, no matter how many retransmission has been already completed.
     * After simulation phase, this would be probably changed
     * \param numberOfRetrasmission indicates how many time the pck has been retransmitted
     * \param maxNumberOfRetrasmission max number of retransmission allowed
     * \param sec it will store the next deadline (seconds)
     * \param usec it will store the next deadline (micro seconds)
     * */
    void calculateNextTimer(int numberOfRetrasmission, int maxNumberOfRetrasmission, unsigned int *sec, unsigned int *usec);


    /**
     * \brief Calculates how much the pkt has to be delayed before the first transmission
     *
     * It calculate the Tca (random timer for collision avoidance) and Tgap (in inverse proportion to previous hop distance)
     * Check Rapid Traffic Information Dissemination Using Named Data paper (NOM) for further information about the timer
     *
     *\param geoStorage it stores the position of the previous hop (if the node is forwarding the node). It's null if the node itself generated the pkt
     *\param sec it will store the next deadline (seconds)
     *\param usec it will store the next deadline (micro seconds)
     * */
    void calculateFirstTransmission(GeoStorage *geoStorage, const LocationService & locationService, unsigned int *sec, unsigned int *usec);

    /**
     * \brief Extract useful information from NDN packet
     *
     * \param p pointer to NDN packet
     * \param header it will store the NDN header extracted form p
     * \return the packet type (interest or content)
     * */
    int getNDNHeader(const Ptr<const Packet> &p, Ptr<Header> header);


    /**
     * \brief Set the NDN-LAL header
     *
     * It stores the NDN-Link Adaptation Layer header in the packet
     * \param llhdr NDN-LAL header that will be stored in the packet
     * \param buffer address where NDN-LAL header and NDN packet will be stored
     * \param data NDN packet address
     * \param len size of NDN packet
     * \return size of the new packet stored in buffer (NDN size + NDN-LAL header size)
     * */
    int setLLHeader(llHeader *llhdr, uint8_t *buffer, const uint8_t *data, int len, const LocationService &locationService );

    /**
     * \brief Update coordinates on NDN-LAL header with the current local node position
     *
     * At this moment, when a packet is stored in the pending table, LLNomPolicy stores everything, NDN data and NDN-LAL header (in the future should be only NDN data)
     * Anyway, right now when a packet has to be retransmitted, the information about coordinates node carried in the header could be old, so they need to be uptaded
     *
     *\param llhdr pointer to the NDN-LAL header that has to be updated
     * */
    void updateLLHeaderCoordinates(llHeader *llhdr, const LocationService &locationService);

#ifdef LAL_STATISTICS
    void printStatistics();
#endif
    
#ifdef TEST_GPS_MOVING
    bool isReachable(GeoStorage &position, const LocationService &locationService);
#endif

    /** It stores the pending packet table*/
    PacketStorage storage;

#ifdef LAL_STATISTICS
    LALStatistic statistics;
#endif
};

} /* namespace vndn */

#endif /* LLNOMPOLICY_H_ */
