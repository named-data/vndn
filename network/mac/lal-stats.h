/*
 * Copyright (c) 2012-2013 Giulio Grassi <giulio.grassi86@gmail.com>
 *                         Davide Pesavento <davidepesa@gmail.com>
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

#ifndef LAL_STATISTIC_H_
#define LAL_STATISTIC_H_

#define LAL_STATISTICS

/**

#define STATISTIC-MONITORING

 -how often do we have to update these information??
*
* Do we need a per packet log? (number of retransmission, ndn name, ndn type)
*   is this the right place??

* It's possible to get the delay statistic using the log
*/


namespace vndn
{

class LALStatistic
{
public:
    LALStatistic();
    virtual ~LALStatistic() {}

    /**
     * \brief Sends all the statistic information to the log
     * */
    void sendToLog();

    void increaseInterestSent() {interestSent++; pktSent++;}
    void increaseContentSent() {contentSent++; pktSent++;}
    void increaseSendingOutContentRequest() {sendingOutContentRequest++; sendingOutRequest++;}
    void increaseSendingOutInterestRequest() {sendingOutInterestRequest++; sendingOutRequest++;}
    void increaseReceivedContent() {receivedContent++; receivedPacket++;}
    void increaseReceivedInterest() {receivedInterest++; receivedPacket++;}
    void increaseReceivedContentGoingUp() {receivedContentGoingUp++; receivedPacketGoingUp++;}
    void increaseReceivedInterestGoingUp() {receivedInterestGoingUp++; receivedPacketGoingUp++;}
    void increaseAckedPacket() {ackedPacket++;}
    void increaseNumberOfRetransmission() {numberOfRetransmission++;}
    void increaseInterestAckedByContent() {interestAckedByContent++;}

protected:
    /**
     * \brief Set all variables to their initial values.
     */
    void resetStats();

    /**
     * \brief Number of packets sent out (effecively sent out)
     *     (in addOutoingPkt if is not present in packetStorage)
     */
    unsigned int pktSent;   //derived data: interestSent+contentSent;

    /**
     * \brief Number of interests sent out (effecively sent out)
     *     (in addOutoingPkt if is not present in packetStorage)
     */
    unsigned int interestSent;

    /**
     * \brief Number of contents sent out (effecively sent out)
     *     (in addOutoingPkt if is not present in packetStorage)
     */
    unsigned int contentSent;

    /**
     * \brief Number of packet NDN requests to send out
     * It counts also the packet that NDN-LAL doesn't send out because already pending
     * (in addOutoingPk, whenever is present or not in packetStorage)
     */
    unsigned int sendingOutRequest;  // sendingOutContentRequest + sendingOutInterestRequest
    unsigned int sendingOutContentRequest;
    unsigned int sendingOutInterestRequest;

    /**
     * \brief Number of received packet
     * It counts all the received packet, no matter if it's a retransmission, if the pkt is discarded or if it's transmitted up to NDN
     */
    unsigned int receivedPacket;  //receivedContent + receivedInterest
    unsigned int receivedContent;
    unsigned int receivedInterest;

    /**
     * \brief number or received packets that LAL sends up to NDN
     */
    unsigned int receivedPacketGoingUp; //receivedContentGoingUp + receivedInterestGoingUp
    unsigned int receivedContentGoingUp;
    unsigned int receivedInterestGoingUp;

    /**
     * \brief Number of acked packet
     */
    unsigned int ackedPacket;

    /**
     * \brief Total number of retransmission
     * It counts the total number of retransmission of the packets send out and acked successfully
     * The counter will be reset every time the statistic will be transmitted
     */
    unsigned int numberOfRetransmission;

    /**
     * \brief Number of interests acked by a content
     */
    unsigned int interestAckedByContent;
};

}

#endif //NDN_LAL_STATISTIC_H_


/**
 * LOGGING
 *
 * GPS
 *  new position: gps coordinate
 *
 * Location service :
 *  new position -> map segment (we can collapse this info with gps coordinate)
 *
 * LAL LEVEL
 *  sent a packet out (info packet)
 *  received a packet from the network (info packet + source mac + ?position?)
 *      is it an ack
 *          how many ack are needed
 *      is it a push progress
 *      is a content that ack an interest?
 *      Could the position be written by locationService??

 *  expiration of a packet (info packet + received ack / needed ack)
 *  retransmission of a packet (info packet + number of transmission + received ack / needed ack)
 *
 * packet acked by content
 *  number of receivec ack, number of retransmission
 * packet acked
 *  number of receivec ack, number of retransmission
 *
 *  INFO FOR EACH PACKET:
 *      type: interest / content
 *      name
 *      size
 *      nonce (if present)
 *
 * communication to NDN (in / out) it can be done at NDN face level (if we have enough info about pkt)
 *
 * NDN FACE
 *  send out a packet (info packet)
 *  received a packet (info packet)
 *
 */
