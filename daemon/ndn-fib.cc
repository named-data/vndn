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

#include "ndn-fib.h"

#include "ndn.h"
#include "ndn-face.h"
#include "network/ndn-interest-header.h"
#include "corelib/assert.h"
#include "corelib/log.h"

#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
namespace ll = boost::lambda;

using namespace boost::tuples;
using namespace boost;

namespace vndn
{

//////////////////////////////////////////////////////////////////////
// Helpers
//////////////////////////////////////////////////////////////////////
namespace __ndn_private
{
struct NDNFibFaceMetricByFace {
    typedef NDNFibFaceMetricContainer::type::index<i_face>::type type;
};
}

//////////////////////////////////////////////////////////////////////

NS_LOG_COMPONENT_DEFINE ("NDNFib");

using namespace __ndn_private;


void NDNFibEntry::UpdateStatus(Ptr<NDNFace> face, NDNFibFaceMetric::Status status)
{
    NS_LOG_FUNCTION (this << boost::cref(*face) << status);

    NDNFibFaceMetricByFace::type::iterator record = m_faces.get<i_face> ().find (face);
    if (record == m_faces.get<i_face> ().end()) {
        // do nothing.  This will be the case for virtual cache faces
        return;
    }
    // NS_ASSERT_MSG (record != m_faces.get<i_face> ().end (),
    //                "Update status can be performed only on existing faces of NDNFibEntry");

    m_faces.modify(record, (&ll::_1)->* &NDNFibFaceMetric::m_status = status);

    // reordering random access index same way as by metric index
    m_faces.get<i_nth> ().rearrange (m_faces.get<i_metric> ().begin ());
}

void NDNFibEntry::AddOrUpdateRoutingMetric(Ptr<NDNFace> face, int32_t metric)
{
    NS_LOG_FUNCTION (this);
    NS_ASSERT_MSG (face != NULL, "Trying to Add or Update NULL face");

    NDNFibFaceMetricByFace::type::iterator record = m_faces.get<i_face> ().find (face);
    if (record == m_faces.get<i_face> ().end()) {
        m_faces.insert (NDNFibFaceMetric (face, metric));
    } else {
        m_faces.modify(record, (&ll::_1)->* &NDNFibFaceMetric::m_routingCost = metric);
    }

    // reordering random access index same way as by metric index
    m_faces.get<i_nth> ().rearrange (m_faces.get<i_metric> ().begin ());
}

const NDNFibFaceMetric &NDNFibEntry::FindBestCandidate(uint32_t skip/* = 0*/) const
{
    if (m_faces.size() == 0)
        throw NDNFibEntry::NoFaces();

    skip = skip % m_faces.size();
    return m_faces.get<i_nth>()[skip];
}

NDNFib::NDNFib()
{
}

void NDNFib::DoDispose(void)
{
    m_fib.clear();
}

NDNFibEntryContainer::type::iterator NDNFib::LongestPrefixMatch(const NameComponents &name) const
{
    NS_LOG_FUNCTION(this << name);

    for (size_t componentsCount = name.GetComponents().size() + 1;
         componentsCount > 0;
         componentsCount--) {
        NameComponents subPrefix(name.GetSubComponents(componentsCount - 1));
        NDNFibEntryContainer::type::iterator match = m_fib.get<i_prefix>().find(subPrefix);

        if (match != m_fib.end()) {
            NS_LOG_INFO("Found FIB entry with prefix: " << subPrefix);
            return match;
        }
    }

    return m_fib.end();
}

void NDNFib::Print() const
{
    NS_LOG_DEBUG("FIB has " << m_fib.size() << " entries:");
    BOOST_FOREACH(const NDNFibEntry & fibEntry, m_fib) {
        NDNPrefixHash hash;
        NS_LOG_DEBUG("  " << fibEntry.GetPrefix() << " with hash " << hash(fibEntry.GetPrefix()));
    }
}

NDNFibEntryContainer::type::iterator NDNFib::Add(const NameComponents &prefix,
                                                 Ptr<NDNFace> face,
                                                 int32_t metric)
{
    NS_LOG_FUNCTION(this << prefix << face << metric);

    NDNFibEntryContainer::type::iterator entry = m_fib.find(prefix);
    if (entry == m_fib.end()) {
        entry = m_fib.insert(m_fib.end(), NDNFibEntry(prefix));
    }

    NS_ASSERT_MSG(face != NULL, "Trying to modify NULL face");
    m_fib.modify(entry,
                 ll::bind(&NDNFibEntry::AddOrUpdateRoutingMetric,
                          ll::_1, face, metric));

    NS_LOG_INFO("Added new prefix " << prefix << " for face " << *face);
    return entry;
}

void NDNFib::Remove(const NDNFibEntry &entry, Ptr<NDNFace> face)
{
    NS_LOG_FUNCTION(entry << *face);

    m_fib.modify (m_fib.iterator_to (entry),
                  ll::bind (&NDNFibEntry::RemoveFace, ll::_1, face));
    if (entry.m_faces.size () == 0) {
        m_fib.erase (m_fib.iterator_to (entry));
    }
}

int NDNFib::RemovePrefix(const NameComponents &prefix, Ptr<NDNFace> face)
{
    NS_LOG_FUNCTION(this << prefix << *face);

    NDNFibEntryContainer::type::iterator entry = m_fib.find(prefix);

    if (entry == m_fib.end()) {
        return (-1);
    }
    m_fib.modify (entry,
                  ll::bind (&NDNFibEntry::RemoveFace, ll::_1, face));
    if (entry->m_faces.size () == 0) {
        m_fib.erase(entry);
    }
    NS_LOG_INFO("Removed prefix " << prefix << " from face " << *face);
    return (0);
}

void NDNFib::RemoveFromAll(Ptr<NDNFace> face)
{
    NS_LOG_FUNCTION(*face);

    for_each (m_fib.begin (), m_fib.end (),
              ll::bind (&NDNFib::Remove, this, ll::_1, face));
}

/**
 * \brief Get number of FIB entry (for python bindings)
 */
uint32_t NDNFib::GetNDNFibEntryCount() const
{
    return m_fib.size ();
}

/**
 * \brief Get FIB entry by index (for python bindings) BROKEN\POINTLESS
 */
const NDNFibEntry &NDNFib::GetNDNFibEntry(uint32_t index)
{
    NS_ASSERT (0 <= index && index < m_fib.size ());
    return m_fib.get<i_nth> () [index];
}

NDNFib::~NDNFib()
{
}

std::ostream &operator<< (std::ostream &os, const NDNFib &fib)
{
    os << "  Dest prefix      Interfaces(Costs)                  \n";
    os << "+-------------+--------------------------------------+\n";

    for (NDNFibEntryContainer::type::iterator entry = fib.m_fib.begin ();
            entry != fib.m_fib.end ();
            entry++) {
        os << entry->GetPrefix () << "\t" << *entry << "\n";
    }
    return os;
}

std::ostream &operator<< (std::ostream &os, const NDNFibEntry &entry)
{
    for (NDNFibFaceMetricContainer::type::index<i_nth>::type::iterator metric =
                entry.m_faces.get<i_nth> ().begin ();
            metric != entry.m_faces.get<i_nth> ().end ();
            metric++) {
        if (metric != entry.m_faces.get<i_nth> ().begin ())
            os << ", ";

        os << *metric;
    }
    return os;
}

std::ostream &operator<< (std::ostream &os, const NDNFibFaceMetric &metric)
{
    static const std::string statusString[] = {"", "g", "y", "r"};

    os << *metric.m_face << "(" << metric.m_routingCost << "," << statusString [metric.m_status] << "," << metric.m_face->GetMetric () << ")";
    return os;
}

} // namespace vndn
