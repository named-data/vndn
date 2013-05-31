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

#ifndef LLPOLICY_H_
#define LLPOLICY_H_

#include "packet-storage.h"
#include "corelib/ptr.h"
#include "network/packet.h"
#include "ll-upper-layer-comm-service.h"
#include "ll-metadata-80211-adhoc.h"
#include "lal-ack-manager.h"

#include "utils/geo/location-service.h"
#include "utils/geo/gps-info.h"


namespace vndn
{

/**
 * \brief Defines all the policies at ndn-link adaptation layer
 *
 * The policies defined are:
 * -Retransmission protocol (if necessary)
 * -Acknowledgment (if necessary)
 * -Neighboring protocol (if necessary)
 *
 * LLPolicy provides an API that allows the NDNDeviceAdapter to send a packet out on the network, to receive a packet from the network, to know when and which packet has to be retransmitted ..
 * All the policies above don't involve directly the NDN protocol. Retransmission, ack ... defined in LLPolicy involve only the ndn-link adaptation layer (sort of 2.5 ISO/OSI layer)
 * */
class LLPolicy
{
public:
    //command between policy and LLDaemon
    
    //TODO: review command implementation: part of them are not used anymore ( the communicationService does the job of the NDNDevceAdaptpet without using the commands
    static const int ERROR = -1;
    static const int GOUPLAYER = 1;
    static const int GOTONETWORK = 2;
    static const int DISCARD = 3;


    static const int NOTIMER = -1;

    /**
     * \brief Create an empty  LLPolicy
     * */
    LLPolicy();

    virtual ~LLPolicy();

    /*
     * \brief The upper layer asked to send a packet out. LLPolicy has to be applied
     *
     * The upper layer (NDN through NDNDeviceAdapter) has to send a packet out.
     * This packet has to be processed by LLPolicy, that will apply the ndn-link adaptation layer policies
     * It's possible that LLPolicy stops the sending process according to the policy
     *
     * \param pkt NDN packet that has to be sent out on the network. At this address the funtion will store the packet plus the link adaptation header
     * \param len the caller has to store the size of the NDN packet. THe function will store the size of the packet plus the link adaptation header (the size CAN'T be greather then MAXNETWORKPKTSIZE)
     * \return what LLPolicy has decided about the packet (send the packet or  discard it)
     * */
    virtual int addOutgoingPkt(uint8_t pkt[], int *len, LLMetadata80211AdHoc *metadata, const geo::LocationService & locationService) = 0;


    /*
     * \brief LLPolicy has to process a pkt that has been received from the network
     *
     * \param pkt pointer to the packet that has been received (still with NDN-LL adaptation header). The function will store the NDN packet without NDN-LL adaptation header in it.
     * \param len it stores the size of the received packet. After, it will store the size of the NDN packet without NDN-LL adaptation header
     * \return  what LLPolicy has decided about the packet (send it to the upper layer or discard it).
     *   */
    virtual int pktFromNetwork(uint8_t pkt[], int *len, void *&dataWithoutLLHeader, const geo::LocationService & locationService) = 0;


    /**
     * \brief Tells when the next packet has to be retransmitted
     *
     * The values that tell when a packet has to be retransmitted are in the same form of a timeval (second and microseconds)
     * \param sec the function will store when the next packet has to be retransmitted ( seconds)
     * \param usec the function will store when the next packet has to be retransmitted (micro seconds)
     * \return 1 if there is at least a packet that has to be retransmitted. NOTIMER otherwise
     * */
    virtual int getNextDeadline(long int *sec, long int *usec) = 0;


    /**
     * \brief Get the next packet that has to be retransmitted
     *
     * It gets the next packet scheduled for a retransmission
     * \param ptrData the function will store the packet scheduled for the retransmission
     * \return size of the packet (it can't be greater than MAXNETWORKPKTSIZE)
     * */
    virtual int getPktForRetransmission(uint8_t ptrData[], const geo::LocationService & locationService) = 0;

    /**
     * \brief Give to LLPolicy the reference to the LLUpperLayerCommunicationService that can be used to communicate with NDN layer (through NDNNetDeviceFace)
     *
     * \param upperLayerComServ pointer to the LLUpperLayerCommunicationService
     * */
    virtual void setLLupperLayerCommunication(LLUpperLayerCommunicationService *upperLayerComServ) = 0;

    virtual void printStatistic()= 0;


protected:
    /**
     * \brief reference to the LLUpperLayerCommunicationService used to communicate with a NDNNetDeviceFace
     * */
    LLUpperLayerCommunicationService *communicationService;

    /**
     * \brief Manages the acknowledgment process of outoing packets
     * */
    LALAckManager *ackManager;
};

} /* namespace vndn */
#endif /* LLPOLICY_H_ */
