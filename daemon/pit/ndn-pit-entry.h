/*
 * Copyright (c) 2011 University of California, Los Angeles
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
 */

#ifndef _NDN_PIT_ENTRY_H_
#define _NDN_PIT_ENTRY_H_

#include "corelib/ptr.h"
#include "ndn-pit-entry-incoming-face.h"
#include "ndn-pit-entry-outgoing-face.h"
#include "daemon/ndn-fib.h"
#include "network/request-source-info.h"

#include <iostream>
#include <set>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/tag.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/mem_fun.hpp>

namespace vndn
{

class NDNFace;
class NameComponents;

namespace __ndn_private
{
class i_retx {};
}

/**
 * \ingroup ndn
 * \brief Typedef for indexed face container of NDNPitEntryIncomingFace
 *
 * Indexes:
 * - by face (may be it will be possible to replace with just the std::map)
 */
struct NDNPitEntryIncomingFaceContainer {
    typedef boost::multi_index::multi_index_container <
    NDNPitEntryIncomingFace,
    boost::multi_index::indexed_by <
    // For fast access to elements using NDNFace
    boost::multi_index::ordered_unique <
    boost::multi_index::tag<__ndn_private::i_face>,
    boost::multi_index::member<NDNPitEntryIncomingFace, Ptr<NDNFace>, &NDNPitEntryIncomingFace::m_face>
    >
    >
    > type;
};

/**
 * \ingroup ndn
 * \brief Typedef for indexed face container of NDNPitEntryOutgoingFace
 *
 * Indexes:
 * - by face (may be it will be possible to replace with just the std::map)
 */
struct NDNPitEntryOutgoingFaceContainer {
    typedef boost::multi_index::multi_index_container <
    NDNPitEntryOutgoingFace,
    boost::multi_index::indexed_by <
    // For fast access to elements using NDNFace
    boost::multi_index::ordered_unique <
    boost::multi_index::tag<__ndn_private::i_face>,
    boost::multi_index::member<NDNPitEntryOutgoingFace, Ptr<NDNFace>, &NDNPitEntryOutgoingFace::m_face>
    > ,
    boost::multi_index::ordered_non_unique <
    boost::multi_index::tag<__ndn_private::i_retx>,
    boost::multi_index::member<NDNPitEntryOutgoingFace, uint32_t, &NDNPitEntryOutgoingFace::m_retxCount>
    >
    >
    > type;
};


/**
 * \ingroup ndn
 * \brief structure for PIT entry
 */
class NDNPitEntry
{
public:
    /**
     * \brief PIT entry constructor
     * \param prefix Prefix of the PIT entry
     * \param offsettime_duration Relative time to the current moment, representing PIT entry lifetime
     * \param fibEntry A FIB entry associated with the PIT entry
     */
    NDNPitEntry(Ptr<const NameComponents> prefix,
                const boost::posix_time::time_duration &offsettime_duration,
                const NDNFibEntry *fibEntry = 0);

    /**
     * \brief Destroy the pitentry and free memory used to store metadata
     * */
    virtual ~NDNPitEntry();

    /**
     * @brief Update lifetime of PIT entry
     *
     * This function will update PIT entry lifetime to the maximum of the current lifetime and
     * the lifetime Simulator::Now () + offsettime_duration
     *
     * @param offsettime_duration Relative time to the current moment, representing PIT entry lifetime
     */
    void UpdateLifetime(const boost::posix_time::time_duration &offsettime_duration);

    const NameComponents &GetPrefix() const {
        return *m_prefix;
    }

    /**
     * @brief Get current expiration time of the record
     *
     * @returns current expiration time of the record
     */
    const boost::posix_time::ptime &GetExpireTime() const {
        return m_expireTime;
    }

    /**
     * @brief Set expiration time on record as `expiretime_duration` (absolute time)
     *
     * @param expiretime_duration absolute simulation time of when the record should expire
     */
    void SetExpireTime(const boost::posix_time::ptime &expireTime) {
        m_expireTime = expireTime;
    }

    /**
     * @brief Check if nonce `nonce` for the same prefix has already been seen
     *
     * @param nonce Nonce to check
     */
    bool IsNonceSeen(uint32_t nonce) const {
        return m_seenNonces.find(nonce) != m_seenNonces.end();
    }

    /**
     * @brief Add `nonce` to the list of seen nonces
     *
     * @param nonce nonce to add to the list of seen nonces
     *
     * All nonces are stored for the lifetime of the PIT entry
     */
    void AddSeenNonce(uint32_t nonce) {
        m_seenNonces.insert(nonce);
    }

    /**
     * @brief Add `face` to the list of incoming faces
     *
     * @param face Face to add to the list of incoming faces
     * @returns iterator to the added entry
     */
    NDNPitEntryIncomingFaceContainer::type::iterator AddIncoming(Ptr<NDNFace> face);

    /**
     * @brief Remove incoming entry for face `face`
     */
    void RemoveIncoming(Ptr<NDNFace> face);

    /**
     * @brief Clear all incoming faces either after all of them were satisfied or NACKed
     */
    void ClearIncoming() {
        m_incoming.clear();
    }

    /**
     * @brief Add `face` to the list of outgoing faces
     *
     * @param face Face to add to the list of outgoing faces
     * @returns iterator to the added entry
     */
    NDNPitEntryOutgoingFaceContainer::type::iterator AddOutgoing(Ptr<NDNFace> face);

    /**
     * @brief Clear all incoming faces either after all of them were satisfied or NACKed
     */
    void ClearOutgoing() {
        m_outgoing.clear();
    }

    /**
     * @brief Remove all references to face.
     *
     * This method should be called before face is completely removed from the stack.
     * Face is removed from the lists of incoming and outgoing faces
     */
    void RemoveAllReferencesToFace(Ptr<NDNFace> face);

    /**
     * @brief Flag outgoing face as hopeless
     */
    void SetWaitingInVain(NDNPitEntryOutgoingFaceContainer::type::iterator face);

    /**
     * @brief Check if all outgoing faces are NACKed
     */
    bool AreAllOutgoingInVain() const;

    /*
     * @brief Similar to AreAllOutgoingInVain, but ignores `face`
     * \see AreAllOutgoingInVain
     **/
    bool AreTherePromisingOutgoingFacesExcept(Ptr<NDNFace> face) const;

    /**
     * @brief Increase maximum limit of allowed retransmission per outgoing face
     */
    void IncreaseAllowedRetxCount();

    /**
     * \brief return the info about the node that sent or generated the interest
     * */
    const RequestSourceInfo *getSourceMetadata() const {
        return sourceMetadata;
    }

    /**
     * \brief set a new RequestSourceInfo
     * \param rsInfo pointer to the new sourceMetadata
     * */
    void setSourceMetadata(RequestSourceInfo *rsInfo);


private:
    friend std::ostream &operator<< (std::ostream &os, const NDNPitEntry &entry);

    /**
     * \brief Default constructor
     */
    NDNPitEntry() : m_fibEntry(0) {}


public:
    Ptr<const NameComponents> m_prefix; ///< \brief Prefix of the PIT entry
    const NDNFibEntry *m_fibEntry;      ///< \brief FIB entry related to this prefix
    std::set<uint32_t> m_seenNonces;    ///< \brief map of nonces that were seen for this prefix

    NDNPitEntryIncomingFaceContainer::type m_incoming; ///< \brief container for incoming interests
    NDNPitEntryOutgoingFaceContainer::type m_outgoing; ///< \brief container for outgoing interests

    boost::posix_time::ptime m_expireTime; ///< \brief time_duration when PIT entry will be removed

    uint32_t m_maxRetxCount; ///< @brief Maximum allowed number of retransmissions via outgoing faces

    /**
     * \brief info about the node that sent or generated the interest
     */
    RequestSourceInfo *sourceMetadata;
};

} // namespace vndn

#endif // _NDN_PIT_ENTRY_H_
