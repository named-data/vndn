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

#ifndef _NDN_PIT_H_
#define _NDN_PIT_H_

#include "ndn-pit-entry.h"
#include "daemon/hash-helper.h"

#include <iostream>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/tag.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <boost/tuple/tuple.hpp>

namespace vndn
{

class NDN;
class NDNFace;
class ContentObjectHeader;
class InterestHeader;

/**
 * \ingroup ndn
 * \private
 * \brief Private namespace for NDN PIT implementation
 */
namespace __ndn_private
{
class i_timestamp {}; ///< tag for timestamp-ordered records (for cleanup optimization)
}

/**
 * \ingroup ndn
 * \brief Typedef for PIT container implemented as a Boost.MultiIndex container
 *
 * - First index (tag<i_prefix>) is a unique hash index based on
 *   prefixes
 * - Second index (tag<i_timestamp>) is a sequenced index based on
 *   arrival order (for clean-up optimizations)
 *
 * \see http://www.boost.org/doc/core/1_46_1/core/multi_index/doc/ for more information on Boost.MultiIndex library
 */
struct NDNPitEntryContainer {
    typedef boost::multi_index::multi_index_container <
    NDNPitEntry,
    boost::multi_index::indexed_by <
    // indexed by hash
    boost::multi_index::hashed_unique <
    boost::multi_index::tag<__ndn_private::i_prefix>,
    boost::multi_index::const_mem_fun<NDNPitEntry, const NameComponents &, &NDNPitEntry::GetPrefix>,
    NDNPrefixHash
    > ,
    // sequenced to implement MRU
    boost::multi_index::ordered_non_unique <
    boost::multi_index::tag<__ndn_private::i_timestamp>,
    boost::multi_index::member<NDNPitEntry, boost::posix_time::ptime, &NDNPitEntry::m_expireTime>
    >
    >
    > type;
};

////////////////////////////////////////////////////////////////////////

/**
 * \ingroup ndn
 * \brief Class implementing Pending Interests Table
 */
class NDNPit : public NDNPitEntryContainer::type, public SimpleRefCount<NDNPit>
{
public:
    NDNPit();
    virtual ~NDNPit();

    void Print();

    /**
     * \brief remove a pit entry according to its prefix
     */
    void Remove(const NDNPitEntry &pitEntry);

    /**
     * \brief Find corresponding PIT entry for the given content name
     * \param prefix Prefix for which to lookup the entry
     * \returns const reference to Pit entry. If record not found,
     *          NDNPitEntryNotFound exception will be thrown
     */
    const NDNPitEntry &Lookup(const ContentObjectHeader &header) const;

    /**
     * \brief Find corresponding PIT entry for the given content name
     * \param prefix Prefix for which to lookup the entry
     * \returns a tuple:
     * get<0>: `const NDNPitEntry&`: a valid PIT entry (if record does not exist, it will be created)
     * get<1>: `bool`: true if a new entry was created
     * get<2>: `bool`: true if a PIT entry exists and Nonce that present in header has been already seen
     * get<3>: `bool`: true if the find operation is successful, that is either
     * there is already a pit entry
     * or there is no pit entry, but there is some outgoing face for the name in interest
     */
    boost::tuple<const NDNPitEntry &, bool, bool, bool> Lookup(const InterestHeader &header);

    boost::posix_time::time_duration GetPitEntryPruningTimeout() const
    {
        return m_PitEntryPruningTimout;
    }

    /**
     * \brief Set FIB table
     */
    void SetFib(Ptr<NDNFib> fib);

    /** \brief Remove expired records from PIT */
    void CleanExpired();

protected:
    // inherited from Object class
    virtual void DoDispose ();

private:
    /**
     * \brief Perform longest prefix match
     *
     * \param name Name to match
     * \returns If entry found a valid iterator will be returned, otherwise end()
     */
    NDNPitEntryContainer::type::iterator LongestPrefixMatch(const NameComponents &name) const;

    /**
     * \brief Set cleanup timeout
     *
     * Side effect: current clean up even (if any) will be cancelled and a new event started
     *
     * \param timeout cleanup timeout
     */
    void SetCleanuptime_durationout(const boost::posix_time::time_duration &timeout);

    /**
     * \brief Get cleanup timeout
     *
     * \returns cleanup timeout
     */
    boost::posix_time::time_duration GetCleanuptime_durationout() const;

    friend std::ostream &operator<<(std::ostream &os, const NDNPit &fib);

    boost::posix_time::time_duration m_PitEntryPruningTimout;
    boost::posix_time::time_duration m_PitEntryDefaultLifetime;

    Ptr<NDNFib> m_fib; ///< \brief Link to FIB table
    // PitBucket    m_bucketsPerFace; ///< \brief pending interface counter per face

    // /**
    //  * \brief maximum number of buckets. Automatically computed based on link capacity
    //  * averaging over 1 second (bandwidth * 1second)
    //  */
    // PitBucket    maxBucketsPerFace;

    // PitBucket    leakSize; ///< size of a periodic bucket leak
};

///////////////////////////////////////////////////////////////////////////////

std::ostream &operator<<(std::ostream &os, const NDNPit &fib);
std::ostream &operator<<(std::ostream &os, const NDNPitEntry &entry);
// std::ostream& operator<< (std::ostream& os, const NDNFibFaceMetric &metric);

class NDNPitEntryNotFound {};

} // namespace vndn

#endif  /* _NDN_PIT_H_ */
