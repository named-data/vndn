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
 *         Davide Pesavento <davidepesa@gmail.com>
 */

#ifndef NDN_FIB_H
#define NDN_FIB_H

#include "corelib/simple-ref-count.h"
#include "hash-helper.h"
#include "ndn-face.h"
#include "ndn.h"

#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/tag.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/random_access_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/mem_fun.hpp>

#include <iostream>

namespace vndn
{

class InterestHeader;

/**
 * \ingroup ndn
 * \brief Structure holding various parameters associated with a (FibEntry, Face) tuple
 */
class NDNFibFaceMetric
{
public:
    friend class NDNFibEntry;
    friend struct NDNFibFaceMetricContainer;

    enum Status {
        NDN_FIB_GREEN  = 1,
        NDN_FIB_YELLOW = 2,
        NDN_FIB_RED    = 3
    };

    /**
     * \brief Metric constructor
     *
     * \param face Face for which metric
     * \param cost Initial value for routing cost
     */
    NDNFibFaceMetric(Ptr<NDNFace> face, int32_t cost)
        : m_face(face)
        , m_status(NDN_FIB_YELLOW)
        , m_routingCost(cost)
        , m_sRtt(boost::posix_time::seconds(0))
        , m_rttVar(boost::posix_time::seconds(0))
    { }

    /**
     * \brief Comparison operator used by boost::multi_index::identity<>
     */
    bool operator< (const NDNFibFaceMetric &fm) const {
        return *m_face < *fm.m_face;    // return identity of the face
    }

    bool operator< (const Ptr<NDNFace> &face) const {
        return *m_face < *face;
    }

    Ptr<NDNFace> GetFace() const {
        return m_face;
    }
    Status GetStatus() const {
        return m_status;
    }

private:
    friend std::ostream &operator<< (std::ostream &os, const NDNFibFaceMetric &metric);

    Ptr<NDNFace> m_face;    ///< Face

    Status m_status;        ///< \brief Status of the next hop:
    ///<        - NDN_FIB_GREEN
    ///<        - NDN_FIB_YELLOW
    ///<        - NDN_FIB_RED

    int32_t m_routingCost;  ///< \brief routing protocol cost (interpretation of the value depends on the underlying routing protocol)

    boost::posix_time::time_duration m_sRtt;   ///< \brief smoothed round-trip time
    boost::posix_time::time_duration m_rttVar; ///< \brief round-trip time variation
};

/**
 * \ingroup ndn
 * \brief Typedef for indexed face container of NDNFibEntry
 *
 * Currently, there are 2 indexes:
 * - by face (used to find record and update metric)
 * - by metric (face ranking)
 * - random access index (for fast lookup on nth face). Order is
 *   maintained manually to be equal to the 'by metric' order
 */
struct NDNFibFaceMetricContainer {
    typedef boost::multi_index::multi_index_container <
    NDNFibFaceMetric,
    boost::multi_index::indexed_by <
    // For fast access to elements using NDNFace
    boost::multi_index::ordered_unique <
    boost::multi_index::tag<__ndn_private::i_face>,
    boost::multi_index::member<NDNFibFaceMetric, Ptr<NDNFace>, &NDNFibFaceMetric::m_face>
    > ,
    // List of available faces ordered by (status, m_routingCost)
    boost::multi_index::ordered_non_unique <
    boost::multi_index::tag<__ndn_private::i_metric>,
    boost::multi_index::composite_key <
    NDNFibFaceMetric,
    boost::multi_index::member<NDNFibFaceMetric, NDNFibFaceMetric::Status, &NDNFibFaceMetric::m_status>,
    boost::multi_index::member<NDNFibFaceMetric, int32_t, &NDNFibFaceMetric::m_routingCost>
    >
    > ,
    // To optimize nth candidate selection (sacrifice a little bit space to gain speed)
    boost::multi_index::random_access <
    boost::multi_index::tag<__ndn_private::i_nth>
    >
    >
    > type;
};

/**
 * \ingroup ndn
 * \brief Structure for FIB table entry, holding indexed list of
 *        available faces and their respective metrics
 */
class NDNFibEntry : public SimpleRefCount<NDNFibEntry>
{
public:
    class NoFaces {};

    /**
     * \brief Constructor
     * \param prefix Prefix for the FIB entry
     */
    NDNFibEntry(const NameComponents &prefix)
        : m_prefix(Create<NameComponents>(prefix))
        , m_needsProbing(false)
    { }

    /**
     * \brief Update status of FIB next hop
     * \param status Status to set on the FIB entry
     */
    void UpdateStatus(Ptr<NDNFace> face, NDNFibFaceMetric::Status status);

    /**
     * \brief Add or update routing metric of FIB next hop
     *
     * Initial status of the next hop is set to YELLOW
     */
    void AddOrUpdateRoutingMetric(Ptr<NDNFace> face, int32_t metric);

    /**
     * \brief Get prefix for the FIB entry
     */
    const NameComponents &GetPrefix() const {
        return *m_prefix;
    }

    /**
     * \brief Find "best route" candidate, skipping `skip' first candidates (modulo # of faces)
     *
     * throws NDNFibEntry::NoFaces if m_faces.size() == 0
     */
    const NDNFibFaceMetric &FindBestCandidate(uint32_t skip = 0) const;

    /**
     * @brief Remove record associated with `face`
     */
    void RemoveFace(const Ptr<NDNFace> &face) {
        m_faces.erase(face);
    }

private:
    friend std::ostream &operator<< (std::ostream &os, const NDNFibEntry &entry);

    Ptr<NameComponents> m_prefix;               ///< \brief Prefix of the FIB entry
public: // FIXME
    NDNFibFaceMetricContainer::type m_faces;    ///< \brief Indexed list of faces

private:
    bool m_needsProbing;      ///< \brief flag indicating that probing should be performed
};

///////////////////////////////////////////////////////////////////////////////

/**
 * \ingroup ndn
 * \brief Typedef for indexed container for FIB entries
 *
 * Currently, there is only one index
 * - by prefix hash, which is used to perform prefix match
 */
struct NDNFibEntryContainer {
    typedef boost::multi_index::multi_index_container <
    NDNFibEntry,
    boost::multi_index::indexed_by <
    // For fast access to elements using NDNFace
    boost::multi_index::hashed_unique <
    boost::multi_index::tag<__ndn_private::i_prefix>,
    boost::multi_index::const_mem_fun <
    NDNFibEntry,
    const NameComponents &,
    &NDNFibEntry::GetPrefix
    > ,
    NDNPrefixHash
    > ,
    boost::multi_index::random_access <
    boost::multi_index::tag<__ndn_private::i_nth>
    >
    >
    > type;
};

/**
 * \ingroup ndn
 * \brief Class implementing FIB functionality
 */
class NDNFib : public SimpleRefCount<NDNFib>
{
public:
    /**
     * \brief Constructor
     */
    NDNFib();

    void Print() const;

    /**
     * \brief Perform longest prefix match
     *
     * \todo Implement exclude filters
     *
     * \param name Name to match
     * \returns If entry found a valid iterator will be returned, otherwise end()
     */
    NDNFibEntryContainer::type::iterator LongestPrefixMatch(const NameComponents &name) const;

    /**
     * \brief Add or update FIB entry
     *
     * If the entry exists, metric will be updated. Otherwise, new entry will be created.
     *
     * Entries in FIB never deleted. They can be invalidated with metric == NETWORK_UNREACHABLE
     *
     * @param name Prefix
     * @param face Forwarding face
     * @param metric Routing metric
     */
    NDNFibEntryContainer::type::iterator Add(const NameComponents &prefix, Ptr<NDNFace> face, int32_t metric);

    /**
     * @brief Remove reference to a face from the entry. If entry had only this face, the whole
     * entry will be removed
     */
    void Remove(const NDNFibEntry &entry, Ptr<NDNFace> face);

    int RemovePrefix(const NameComponents &prefix, Ptr<NDNFace> face);
    /**
     * @brief Remove all references to a face from FIB.  If for some enty that face was the only element,
     * this FIB entry will be removed.
     */
    void RemoveFromAll(Ptr<NDNFace> face);

    /**
     * \brief Get number of FIB entry (for python bindings)
     */
    uint32_t GetNDNFibEntryCount() const;

    /**
     * \brief Get FIB entry by index (Only for python bindings)
     */
    const NDNFibEntry &GetNDNFibEntry(uint32_t index);

    virtual ~NDNFib();

protected:
    // inherited from Object class
    virtual void DoDispose();

private:
    friend std::ostream &operator<< (std::ostream &os, const NDNFib &fib);
    NDNFib(const NDNFib &) {} ///< \brief copy constructor is disabled

public: // FIXME
    NDNFibEntryContainer::type m_fib;
};

///////////////////////////////////////////////////////////////////////////////

std::ostream &operator<< (std::ostream &os, const NDNFib &fib);
std::ostream &operator<< (std::ostream &os, const NDNFibEntry &entry);
std::ostream &operator<< (std::ostream &os, const NDNFibFaceMetric &metric);

} // namespace vndn

#endif  /* NDN_FIB_H */
