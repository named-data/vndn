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
 * Author: Alexander Afanasyev <alexander.afanasyev@ucla.edu>
 *         Ilya Moiseenko <iliamo@cs.ucla.edu>
 *         Davide Pesavento <davidepesa@gmail.com>
 */

#ifndef NDN_L3_PROTOCOL_H
#define NDN_L3_PROTOCOL_H

#include "corelib/ptr.h"
#include "corelib/simple-ref-count.h"

#include <stdint.h>
#include <vector>
#include <boost/date_time/posix_time/posix_time_types.hpp>


namespace vndn
{

class ContentStore;
class ContentObjectHeader;
class InterestHeader;
class NDNFibEntry;
class NDNFib;
class NDNPitEntry;
class NDNPit;
class NDNFace;
class NDNForwardingStrategy;
class Packet;


/**
 * \ingroup ndn
 * \brief Actual implementation of the NDN network layer
 *
 * This class implements the NDN protocol described in the paper
 * "Networking Named Content" by Van Jacobson et al. This class
 * aggregates three objects: a Content Store, a Pending Interest
 * Table (PIT), and a FIB. Besides, it also manages all the faces,
 * including NDNLocalFace that is used to communicate with local
 * applications, NDNAdhocNetDeviceFace that is used to communicate with
 * other peers in vehicular network. Finally this class can be configured
 * with a customized forwarding strategy that decides how interests should
 * be forwarded in the strategy layer of the NDN protocol. By default,
 * it uses the NDNFloodingStrategy.
 */
class NDNL3Protocol : public SimpleRefCount<NDNL3Protocol>
{
public:
    static const uint16_t ETHERNET_FRAME_TYPE; ///< \brief Ethernet Frame Type of NDN
    static const struct timeval REAP_INTERVAL; ///< \brief the time interval between consecutive Reap() calls, where expired interests will be removed

    /**
     * \brief Default constructor. Creates an empty stack without forwarding strategy set
     */
    NDNL3Protocol();
    ~NDNL3Protocol();

    /**
     * \brief the function will call NDNPit::CleanExpired() to remove expired interests in PIT
     */
    static void Reap(int fd, short events, void *args);

    /**
     * \brief Returns the fib
     */
    Ptr<NDNFib> GetFib();

    /**
     * \brief Set the fib to be used by this stack
     */
    void SetFib(Ptr<NDNFib> fib);

    Ptr<NDNForwardingStrategy> GetForwardingStrategy() const;
    void SetForwardingStrategy(Ptr<NDNForwardingStrategy> forwardingStrategy);

    uint32_t AddFace(const Ptr<NDNFace> &face);
    void RemoveFace(Ptr<NDNFace> face);
    Ptr<NDNFace> GetFace(uint32_t face) const;
    const std::vector<Ptr<NDNFace> > &GetAllFaces() const;

    /**
     * \brief Callback function used to NDNFaces to notify the NDNL3Protocol that a packet
     * has arrived at the face.
     * @param face   the NDNFace that has received a packet
     * @param p      the original pakcet
     */
    void Receive(const Ptr<NDNFace> &face, const Ptr<const Packet> &p);

private:
    NDNL3Protocol(const NDNL3Protocol &); ///< copy constructor is disabled
    NDNL3Protocol &operator= (const NDNL3Protocol &); ///< copy operator is disabled

    /**
     * \brief when a face goes down, this function call be called to remove pit entries
     *        that are associated with the face
     * @param face     the face tha goes down or become unreachable
     */
    void TrimPitEntriesAssociatedWithFace(Ptr<NDNFace> face);

    /**
     * \brief Satisfy the pending interest represented by parameter pitEntry by
     *        sending content packet to all the incoming faces of the pitEntry,
     *        and then remove the pitEntry from pit
     * @param pitEntry             the pitEntry to be satisfied
     * @param content_packet       the content packet that can satisfy the pitEntry
     */
    void SatisfyPendingInterests(const NDNPitEntry  &pitEntry,
                                 const Ptr<const Packet> &content_packet);

    /**
     * \brief Decide whether to cache unsolicited data or ignore it based on the variable m_cacheUnsolicitedData
     * @param header  header of the unsolicited data packet
     * @param payload payload of the
     */
    bool HandleUnsolicitedData(const Ptr<const ContentObjectHeader> &header,
                               const Ptr<const Packet> &payload);
    /**
     * \brief Actual processing of incoming NDN interests. Note, interests do not have payload
     *
     * Processing Interest packets
     * @param face    incoming face
     * @param header  deserialized Interest header
     * @param packet  original packet
     */
    void OnInterest(const Ptr<NDNFace> &face,
                    const Ptr<const InterestHeader> &header,
                    const Ptr<const Packet> &p);


    void handleDuplicateInterest(const Ptr<NDNFace> &incomingFace,
                                 const Ptr<const InterestHeader> &header);

    bool updatePITForInterest(const NDNPitEntry &pitEntry,
                              const Ptr<NDNFace> &incomingFace,
                              const Ptr<const InterestHeader> &header);

    bool checkContentStoreForInterest(const Ptr<const InterestHeader> &header,
                                      const NDNPitEntry &pitEntry);

    /**
     * \brief Processing of incoming NDN NACKs. Note, these packets, like interests, do not have payload
     *
     * Processing NACK packets
     * @param face    incoming face
     * @param header  deserialized Interest header
     * @param packet  original packet
     */
    void OnNack(const Ptr<NDNFace> &face,
                const Ptr<const InterestHeader> &header,
                const Ptr<const Packet> &p);

    /**
     * \brief Actual processing of incoming NDN content objects
     *
     * Processing ContentObject packets
     * @param face    incoming face
     * @param header  deserialized ContentObject header
     * @param payload data packet payload
     * @param packet  original packet
     */
    void OnData(const Ptr<NDNFace> &face,
                const Ptr<const ContentObjectHeader> &header,
                const Ptr<const Packet> &packet);

    void GiveUpInterest(const NDNPitEntry &pitEntry,
                        const Ptr<const InterestHeader> &header,
                        const Ptr<const Packet> &packet);


    typedef std::vector<Ptr<NDNFace> > NDNFaceList;
    NDNFaceList m_faces;              ///< \brief list of faces that belongs to ndn stack on this node

    Ptr<NDNForwardingStrategy> m_forwardingStrategy; ///< \brief smart pointer to the selected forwarding strategy

    Ptr<NDNPit> m_pit;                ///< \brief PIT (pending interest table)
    Ptr<NDNFib> m_fib;                ///< \brief FIB
    Ptr<ContentStore> m_contentStore; ///< \brief Content store (for caching purposes only)

    bool m_cacheUnsolicitedData;
    bool m_nacksEnabled;
};

}

#endif /* NDN_L3_PROTOCOL_H */
