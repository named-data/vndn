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

#include "corelib/log.h"
#include "ndn-pit.h"
#include "network/ndn-interest-header.h"
#include "network/ndn-content-object-header.h"

#include <iostream>
#include <boost/bind.hpp>
#include <boost/lambda/lambda.hpp>

using namespace boost::tuples;

NS_LOG_COMPONENT_DEFINE("NDNPit");

namespace vndn
{

using namespace __ndn_private;

NDNPit::NDNPit()
{
}

NDNPit::~NDNPit()
{
    DoDispose();
}

void NDNPit::DoDispose()
{
    // if (m_cleanupEvent.IsRunning ())
    //   m_cleanupEvent.Cancel ();

    clear();
}

void NDNPit::Remove(const NDNPitEntry &pitEntry)
{
    get<i_prefix>().erase(pitEntry.GetPrefix());
}

void NDNPit::CleanExpired()
{
    NS_LOG_FUNCTION_NOARGS();

    ptime now = microsec_clock::local_time();

    while (!empty()) {
        NDNPit::index<i_timestamp>::type::iterator entry = get<i_timestamp>().begin();
        if (now >= entry->GetExpireTime()) { // is the record stale?
            get<i_timestamp>().erase(entry);
        } else
            break; // nothing else to do. All later records will not be stale
    }
}

void NDNPit::SetFib(Ptr<NDNFib> fib)
{
    m_fib = fib;
}

void NDNPit::Print()
{
    NS_LOG_DEBUG("Pit table has " << get<i_prefix>().size() << " entries:");
    BOOST_FOREACH(const NDNPitEntry &pitEntry, get<i_prefix>()) {
        NS_LOG_DEBUG("  " << pitEntry.GetPrefix());
    }
}

const NDNPitEntry &NDNPit::Lookup(const ContentObjectHeader &header) const
{
    NS_LOG_FUNCTION_NOARGS();

    NDNPitEntryContainer::type::iterator entry = get<i_prefix>().find(*header.GetName());
    if (entry == end())
        throw NDNPitEntryNotFound();

    return *entry;
}

boost::tuple<const NDNPitEntry &, bool, bool, bool> NDNPit::Lookup(const InterestHeader &header)
{
    NS_LOG_FUNCTION_NOARGS();

    bool isDuplicate = false;
    bool isNew = true;
    Ptr<const NameComponents> name = header.GetName();

    NDNPitEntryContainer::type::iterator entry = LongestPrefixMatch(*name);

    if (entry == end()) {
        NDNFibEntryContainer::type::iterator match = m_fib->LongestPrefixMatch(*name);
        const NDNFibEntry *fibEntry = match != m_fib->m_fib.end() ? &*match : 0;
        time_duration lifetime = header.GetInterestLifetime() == seconds(0) ? m_PitEntryDefaultLifetime : header.GetInterestLifetime();

        entry = insert(end(), NDNPitEntry(name, lifetime, fibEntry));
    } else {
        isNew = false;
        isDuplicate = entry->IsNonceSeen(header.GetNonce());
    }

    if (!isDuplicate && entry != end()) {
        modify(entry, boost::bind(&NDNPitEntry::AddSeenNonce, boost::lambda::_1, header.GetNonce()));
    }

    return make_tuple(boost::cref(*entry), isNew, isDuplicate, true);
}

NDNPitEntryContainer::type::iterator NDNPit::LongestPrefixMatch(const NameComponents &name) const
{
    NS_LOG_FUNCTION(this << name);

    for (size_t componentsCount = name.GetComponents().size() + 1;
         componentsCount > 0;
         componentsCount--) {
        NameComponents subPrefix(name.GetSubComponents(componentsCount - 1));
        NDNPitEntryContainer::type::iterator match = get<i_prefix>().find(subPrefix);

        if (match != end()) {
            NS_LOG_INFO("Found PIT entry with prefix: " << subPrefix);
            return match;
        }
    }

    return end();
}

} // namespace vndn
