/*
 * Copyright (c) 2011 University of California, Los Angeles
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Alexander Afanasyev <alexander.afanasyev@ucla.edu>
 */

#ifndef NDN_CONTENT_STORE_IMPL_H_
#define NDN_CONTENT_STORE_IMPL_H_

#include "ndn-content-store.h"
#include "network/packet.h"
#include "network/ndn-interest-header.h"
#include "network/ndn-content-object-header.h"
#include <boost/foreach.hpp>
#include "utils/trie-with-policy.h"

namespace vndn
{

class NameComponents;

template<class Policy>
class ContentStoreImpl : public ContentStore,
        protected trie_with_policy < NameComponents,
        smart_pointer_payload_traits< Entry >,
        Policy >
{
private:
    typedef trie_with_policy < NameComponents,
            smart_pointer_payload_traits< Entry >,
            Policy > super;

public:
    virtual boost::tuple<Ptr<const ContentObjectHeader>, Ptr<const Packet> >
    Lookup (Ptr<const InterestHeader> interest);

    virtual bool
    Add (Ptr<const ContentObjectHeader> header, Ptr<const Packet> packet);

    // virtual bool
    // Remove (Ptr<InterestHeader> header);

    virtual void Print (std::ostream &os) const;

private:
    void SetMaxSize (uint32_t maxSize);
    uint32_t GetMaxSize () const;
};


template<class Policy>
boost::tuple<Ptr<const ContentObjectHeader>, Ptr<const Packet> >
ContentStoreImpl<Policy>::Lookup (Ptr<const InterestHeader> interest)
{
    // NS_LOG_FUNCTION (this << interest->GetName ());

    /// @todo Change to search with predicate
    typename super::const_iterator node = this->deepest_prefix_match (*(interest->GetName ()));

    if (node != this->end ()) {
        // NS_LOG_DEBUG ("cache hit with " << node->payload ()->GetHeader ()->GetName ());
        return boost::make_tuple (node->payload ()->GetHeader (),
                                  node->payload ()->GetPacket ());
    } else {
        // NS_LOG_DEBUG ("cache miss for " << interest->GetName ());
        return boost::tuple<Ptr<ContentObjectHeader>, Ptr<const Packet> > (0, 0);
    }
}

template<class Policy>
bool ContentStoreImpl<Policy>::Add (Ptr<const ContentObjectHeader> header, Ptr<const Packet> packet)
{
    // NS_LOG_FUNCTION (this << header->GetName ());

    return
        this->insert (*(header->GetName ()), Create<Entry> (header, packet))
        .second;
}

template<class Policy>
void ContentStoreImpl<Policy>::Print (std::ostream &os) const
{
    for (typename super::policy_container::const_iterator item = this->getPolicy ().begin ();
            item != this->getPolicy ().end ();
            item++)
        // BOOST_FOREACH (const typename super::parent_trie &item, this->getPolicy ())
    {
        os << item->payload ()->GetName () << std::endl;
    }
}

template<class Policy>
void ContentStoreImpl<Policy>::SetMaxSize (uint32_t maxSize)
{
    this->getPolicy ().set_max_size (maxSize);
}

template<class Policy>
uint32_t ContentStoreImpl<Policy>::GetMaxSize () const
{
    return this->getPolicy ().get_max_size ();
}

} // namespace vndn

#endif // NDN_CONTENT_STORE_IMPL_H_
