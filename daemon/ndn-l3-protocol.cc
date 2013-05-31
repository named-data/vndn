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

#include "ndn-l3-protocol.h"

#include "corelib/log.h"
#include "corelib/ptr.h"
#include "corelib/singleton.h"
#include "ndn-fib.h"
#include "cs/content-store-impl.h"
#include "pit/ndn-pit.h"
#include "ndn-face.h"
#include "ndn-forwarding-strategy.h"
#include "ndn-net-device-face.h"
#include "helper/ndn-header-helper.h"
#include "helper/event-monitor.h"
#include "network/packet.h"
#include "network/ndn-interest-header.h"
#include "network/ndn-content-object-header.h"
#include "network/request-source-ip-info.h"
#include "network/mac/ll-metadata-over-ip.h"
#include "utils/lru-policy.h"

#include <boost/foreach.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/tuple/tuple.hpp>

namespace ll = boost::lambda;
using namespace boost::tuples;

NS_LOG_COMPONENT_DEFINE ("NDNL3Protocol");


namespace vndn
{

const uint16_t NDNL3Protocol::ETHERNET_FRAME_TYPE = 0x7777;
const struct timeval NDNL3Protocol::REAP_INTERVAL = {10, 0}; // Reap expired interests every 10 seconds

NDNL3Protocol::NDNL3Protocol()
    : m_cacheUnsolicitedData(true)
    , m_nacksEnabled(false)
{
    NS_LOG_FUNCTION_NOARGS();

    m_contentStore = Create<ContentStoreImpl<lru_policy_traits> >();
    m_fib = Create<NDNFib>();
    m_pit = Create<NDNPit>();
    m_pit->SetFib(m_fib);
}

NDNL3Protocol::~NDNL3Protocol()
{
    NS_LOG_FUNCTION_NOARGS();
}

void NDNL3Protocol::SetForwardingStrategy(Ptr<NDNForwardingStrategy> forwardingStrategy)
{
    m_forwardingStrategy = forwardingStrategy;
    m_forwardingStrategy->SetPit(m_pit);
}

Ptr<NDNForwardingStrategy> NDNL3Protocol::GetForwardingStrategy(void) const
{
    return m_forwardingStrategy;
}

uint32_t NDNL3Protocol::AddFace(const Ptr<NDNFace> &face)
{
    NS_LOG_FUNCTION_NOARGS();
    NS_LOG_DEBUG("Adding " << *face);

    m_faces.push_back(face);
    return face->getMonitorFd();
}

void NDNL3Protocol::TrimPitEntriesAssociatedWithFace(Ptr<NDNFace> face)
{
    std::list<boost::reference_wrapper<const NDNPitEntry> > entriesToRemoves;
    BOOST_FOREACH(const NDNPitEntry & pitEntry, *m_pit) {
        m_pit->modify(m_pit->iterator_to(pitEntry),
                      ll::bind(&NDNPitEntry::RemoveAllReferencesToFace, ll::_1, face));

        // If this face is the only for the associated FIB entry, then FIB entry will be removed.
        // Thus, we have to remove the whole PIT entry
        if (pitEntry.m_fibEntry &&
                pitEntry.m_fibEntry->m_faces.size() == 1 &&
                pitEntry.m_fibEntry->m_faces.begin()->GetFace() == face) {
            entriesToRemoves.push_back(boost::cref(pitEntry));
        }
    }

    BOOST_FOREACH(const NDNPitEntry & removedEntry, entriesToRemoves) {
        m_pit->erase(m_pit->iterator_to(removedEntry));
    }
}

void NDNL3Protocol::RemoveFace(Ptr<NDNFace> face)
{
    NS_LOG_FUNCTION(*face);

    TrimPitEntriesAssociatedWithFace(face);
    m_fib->RemoveFromAll(face);

    NDNFaceList::iterator face_it = find(m_faces.begin(), m_faces.end(), face);
    if (face_it == m_faces.end())
        NS_LOG_WARN("Attempt to remove face that doesn't exist.");
    else
        m_faces.erase(face_it);
}

Ptr<NDNFace> NDNL3Protocol::GetFace(uint32_t index) const
{
    // this function is not supposed to be called often, so linear search is fine
    BOOST_FOREACH (const Ptr<NDNFace> &face, m_faces) {
        if (face->getMonitorFd() == (int)index)
            return face;
    }
    NS_LOG_WARN("NO Face found with FD" << index);
    return 0;
}

const std::vector<Ptr<NDNFace> > &NDNL3Protocol::GetAllFaces() const
{
    return m_faces;
}

// Callback from lower layer
void NDNL3Protocol::Receive(const Ptr<NDNFace> &face, const Ptr<const Packet> &packet)
{
    NS_LOG_FUNCTION(*face);

    try {
        NDNHeaderHelper::Type type = NDNHeaderHelper::GetNDNHeaderType(packet);
        switch (type) {
        case NDNHeaderHelper::INTEREST: {
            // TODO: solve this issue with size != 0
            /*
            if (packet->GetSize() != 0) {
                NS_LOG_WARN("Received invalid interest packet with payload, ignoring.");
                return;
            }*/

            NS_LOG_INFO("Received interest packet.");
            Ptr<InterestHeader> interestHeader = GetHeader<InterestHeader>(*packet);

            if (interestHeader->GetNack() > 0)
                OnNack(face, interestHeader, packet);
            else
                OnInterest(face, interestHeader, packet);
            break;
        }
        case NDNHeaderHelper::CONTENT_OBJECT: {
            NS_LOG_INFO("Received content packet.");
            Ptr<ContentObjectHeader> contentHeader = GetHeader<ContentObjectHeader>(*packet);
            OnData(face, contentHeader, packet);
            break;
        }
        }
    } catch (NDNUnknownHeaderException) {
        NS_LOG_WARN("Received packet with unknown header.");
    }
}

void NDNL3Protocol::OnNack(const Ptr<NDNFace> &incomingFace,
                           const Ptr<const InterestHeader> &header,
                           const Ptr<const Packet> &packet)
{
    NS_LOG_FUNCTION(*incomingFace << header << packet);

    tuple<const NDNPitEntry &, bool, bool, bool> ret = m_pit->Lookup(*header);
    NDNPitEntry const &pitEntry = ret.get<0>();
    bool isNew = ret.get<1>();
    bool isDuplicated = ret.get<2>();

    if (isNew || !isDuplicated) { // potential flaw
        // somebody is doing something bad
        return;
    }

    NDNPitEntryOutgoingFaceContainer::type::iterator outFace = pitEntry.m_outgoing.find(incomingFace);
    if (outFace == pitEntry.m_outgoing.end()) {
        return;
    }

    // This was done in error. Never, never do anything, except normal leakage. This way we ensure that we will not have losses,
    // at least when there is only one client
    //
    // incomingFace->LeakBucketByOnePacket ();

    NS_LOG_INFO("Nack on " << boost::cref(*incomingFace));

    m_pit->modify(m_pit->iterator_to(pitEntry),
                  ll::bind (&NDNPitEntry::SetWaitingInVain, ll::_1, outFace));

    // If NACK is NACK_GIVEUP_PIT, then neighbor gave up trying to and removed it's PIT entry.
    // So, if we had an incoming entry to this neighbor, then we can remove it now

    if (header->GetNack() == InterestHeader::NACK_GIVEUP_PIT) {
        m_pit->modify(m_pit->iterator_to(pitEntry),
                      ll::bind(&NDNPitEntry::RemoveIncoming, ll::_1, incomingFace));
    }

    if (pitEntry.m_fibEntry) {
        m_fib->m_fib.modify(m_fib->m_fib.iterator_to(*pitEntry.m_fibEntry),
                            ll::bind(&NDNFibEntry::UpdateStatus,
                                     ll::_1, incomingFace, NDNFibFaceMetric::NDN_FIB_YELLOW));
    }

    if (pitEntry.m_incoming.size() == 0) { // interest was actually satisfied
        // no need to do anything
        return;
    }

    if (!pitEntry.AreAllOutgoingInVain()) {
        NS_LOG_DEBUG("Not all outgoing are in vain.");
        // Don't do anything, we are still expecting data from some other face
        return;
    }

    NS_ASSERT_MSG(m_forwardingStrategy != 0, "Need a forwarding protocol object to process packets");

    Ptr<Packet> nonNackInterest = Create<Packet>();
    Ptr<InterestHeader> nonNackHeader = Create<InterestHeader>(*header);
    nonNackHeader->SetNack(InterestHeader::NORMAL_INTEREST);
    nonNackInterest->AddHeader(nonNackHeader);

    bool propagated = m_forwardingStrategy->PropagateInterest(pitEntry, incomingFace,
                                                              nonNackHeader, nonNackInterest);

    // ForwardingStrategy will try its best to forward packet to at least one interface.
    // If no interests was propagated, then there is not other option for forwarding or
    // ForwardingStrategy failed to find it.
    if (!propagated) {
        GiveUpInterest(pitEntry, nonNackHeader, nonNackInterest);
    }
}

void NDNL3Protocol::handleDuplicateInterest(const Ptr<NDNFace> &incomingFace,
        const Ptr<const InterestHeader> &header)
{
    /**
     * This condition will handle "routing" loops and also recently satisfied interests.
     * Every time interest is satisfied, PIT entry (with empty incoming and outgoing faces)
     * is kept for another small chunk of time.
     */

    if (m_nacksEnabled) {
        NS_LOG_DEBUG("Sending NACK_LOOP");
        Ptr<InterestHeader> nackHeader = Create<InterestHeader>(*header);
        nackHeader->SetNack(InterestHeader::NACK_LOOP);
        Ptr<Packet> nack = Create<Packet>();
        nack->AddHeader(nackHeader);

        incomingFace->Send(nack);
    }
}

bool NDNL3Protocol::updatePITForInterest(const NDNPitEntry &pitEntry,
                                         const Ptr<NDNFace> &incomingFace,
                                         const Ptr<const InterestHeader> &header)
{
    NS_LOG_FUNCTION_NOARGS();

    NDNPitEntryIncomingFaceContainer::type::iterator inFace = pitEntry.m_incoming.find(incomingFace);

    bool isRetransmitted = false;
    if (inFace != pitEntry.m_incoming.end()) {
        // NDNPitEntryIncomingFace.m_arrivaltime_duration keeps track arrival time of the first packet... why?
        isRetransmitted = true;
        // this is almost definitely a retransmission. But should we trust the user on that?
    } else {
        m_pit->modify(m_pit->iterator_to(pitEntry),
                      ll::var(inFace) = ll::bind(&NDNPitEntry::AddIncoming, ll::_1, incomingFace));
    }

    NS_LOG_DEBUG("IsRetx: " << isRetransmitted);

    // update PIT entry lifetime
    m_pit->modify(m_pit->iterator_to(pitEntry),
                  ll::bind(&NDNPitEntry::UpdateLifetime, ll::_1,
                           header->GetInterestLifetime()));

    return isRetransmitted;
}

Ptr<NDNFib> NDNL3Protocol::GetFib()
{
    return m_fib;
}

void NDNL3Protocol::SetFib(Ptr<NDNFib> fib)
{
    m_fib = fib;
    NS_ASSERT_MSG(m_fib != 0, "FIB must not be null");
    m_pit->SetFib(m_fib);
}

/* check content store for header
 * if found content that can match the name in header,
 * modify the pitEntry accordingly, and return true
 * otherwise return false
 */
bool NDNL3Protocol::checkContentStoreForInterest(const Ptr<const InterestHeader> &header,
                                                 const NDNPitEntry &pitEntry)
{
    NS_LOG_FUNCTION_NOARGS();

    Ptr<const Packet> contentObject;
    Ptr<const ContentObjectHeader> contentObjectHeader; // used for tracing
    boost::tie(contentObjectHeader, contentObject) = m_contentStore->Lookup(header);
    if (contentObject != 0) {
        NS_LOG_INFO("Found in content store.");
        SatisfyPendingInterests(pitEntry, contentObject);
        return true;
    }
    return false;
}

void NDNL3Protocol::OnInterest(const Ptr<NDNFace> &incomingFace,
                               const Ptr<const InterestHeader> &header,
                               const Ptr<const Packet> &packet)
{
    NS_LOG_FUNCTION_NOARGS();
    //m_pit->Print();

    tuple<const NDNPitEntry &, bool, bool, bool> ret = m_pit->Lookup(*header);
    NDNPitEntry const &pitEntry = ret.get<0>();
    bool success = ret.get<3>();
    bool isDuplicated = ret.get<2>();

    //TODO add a method to NDNFace that will do this job (so that we can avoi the type check)
    if (packet->llmetadataptr != NULL) {
        if (packet->llmetadataptr->getRequestSourceInfoType() == OVER_IP) {
            //packet from ndn-hub-over-ip-device-face
            if (pitEntry.getSourceMetadata() == NULL) { //the PitEntry has been created now, we need to store memory for RequestSourceIPInfo
                RequestSourceIPInfo *rsIpInfo = new RequestSourceIPInfo();
                rsIpInfo->addIP(((LLMetadataOverIP *)packet->llmetadataptr)->getIpAddr(), ((LLMetadataOverIP *)packet->llmetadataptr)->getPort());
                free(packet->llmetadataptr);
                m_pit->modify(m_pit->iterator_to(pitEntry),
                              ll::bind(&NDNPitEntry::setSourceMetadata, ll::_1,
                                       rsIpInfo));
            } else {
                RequestSourceIPInfo *rsIpInfo = new RequestSourceIPInfo();
                RequestSourceIPInfo *rsPit = (RequestSourceIPInfo *) pitEntry.getSourceMetadata();
                std::list<std::pair<in_addr_t, unsigned short> >::iterator it;
                for (it = rsPit->getAllSource().begin(); it != rsPit->getAllSource().end(); it++) {
                    rsIpInfo->addIP(it->first, it->second);
                }
                if (!isDuplicated) {
                    rsIpInfo->addIP(((LLMetadataOverIP *)packet->llmetadataptr)->getIpAddr(),((LLMetadataOverIP *)packet->llmetadataptr)->getPort());
                }
                //TODO this is just temporary (hopefully). The new IP has to be added in some smarter way than a loop
                m_pit->modify(m_pit->iterator_to(pitEntry),
                              ll::bind(&NDNPitEntry::setSourceMetadata, ll::_1,
                                       rsIpInfo));
            }
        }
    }

    //bool isRetransmitted = false;
    if (success)
        updatePITForInterest(pitEntry, incomingFace, header);

    /* check content store first */
    if (checkContentStoreForInterest(header, pitEntry))
        return;

    /* check PIT */
    //bool isNew = ret.get<1>();

    if (isDuplicated) {
        NS_LOG_INFO("This is a duplicate interest.");
        handleDuplicateInterest(incomingFace, header);
        return;
    }

    if (success) {
        /* use forwarding strategy to decide how to forward the interest packet */
        //TODO if the packet cames from adhoc and has metadata, if we forward it in a different face, free of LLmetadata
        NS_LOG_INFO("Forwarding interest ...");
        bool propagated = m_forwardingStrategy->PropagateInterest(pitEntry, incomingFace, header, packet);
        if (!propagated) {
            NS_LOG_DEBUG ("Not propagated");
            GiveUpInterest (pitEntry, header, packet);
        }
    } else {
        NS_LOG_WARN("There are no outgoing faces for this interest.");
    }
}

bool NDNL3Protocol::HandleUnsolicitedData(const Ptr<const ContentObjectHeader> &header,
                                          const Ptr<const Packet> &payload)
{
    NS_LOG_INFO("Received unsolicited data.");

    if (m_cacheUnsolicitedData) {
        // Optimistically add or update entry in the content store
        NS_LOG_DEBUG("Caching unsolicited data " << header->GetName());
        bool newEntry = m_contentStore->Add(header, payload);
        if (!newEntry) {
            NS_LOG_DEBUG("Content is already in cache.");
            // stop processing if data was already in cache
            return false;
        }
    } else {
        NS_LOG_DEBUG("Ignoring unsolicited data.");
        return false;
    }

    return true;
}

void NDNL3Protocol::SatisfyPendingInterests(const NDNPitEntry  &pitEntry,
                                            const Ptr<const Packet> &content_packet)
{
    NS_LOG_FUNCTION_NOARGS();
    NS_LOG_DEBUG("There are pit entries from " << pitEntry.m_incoming.size() << " faces.");

    BOOST_FOREACH (const NDNPitEntryIncomingFace & incoming, pitEntry.m_incoming) {
        incoming.m_face->Send(content_packet, pitEntry.sourceMetadata);
    }
    m_pit->Remove(pitEntry);
}

// Processing ContentObjects
void NDNL3Protocol::OnData(const Ptr<NDNFace> &incomingFace,
                           const Ptr<const ContentObjectHeader> &header,
                           const Ptr<const Packet> &packet)
{
    NS_LOG_FUNCTION_NOARGS();

    try {
        NS_LOG_DEBUG("Looking for content with name " << *(header->GetName()) << " against PIT.");
        const NDNPitEntry &pitEntry = m_pit->Lookup(*header);

        NDNPitEntryOutgoingFaceContainer::type::iterator previously_sent_out_face = pitEntry.m_outgoing.find(incomingFace);

        if (previously_sent_out_face == pitEntry.m_outgoing.end()) {
            NS_LOG_DEBUG("Ignoring unsolicited data.");
            return;
        }

        if (packet->llmetadataptr != NULL) {
            if (packet->llmetadataptr->getRequestSourceInfoType() == OVER_IP) { //we don't need metadata on received content
                //packet from ndn-hub-over-ip-device-face
                /*
                if(pitEntry.getSourceMetadata()==NULL){ //the PitEntry has been created now, we need to store memory for RequestSourceIPInfo
                    RequestSourceIPInfo * rsIpInfo = new RequestSourceIPInfo();
                    pitEntry.setSourceMetadata(rsIpInfo);
                }
                pitEntry.getSourceMetadata()->addIP(((LLMetadataOverIP *)packet->llmetadataptr)->getIpAddr());
                */
                free(packet->llmetadataptr);
                //packet->llmetadata=NULL;
                //packet->llmetadataptr=NULL;
            }
        }

        if (pitEntry.m_fibEntry) {
            // Update metric status for the incoming interface in the corresponding FIB entry
            // marking the incomingFace as green, this does not require the incoming face to
            // belong to our forwarding face
            m_fib->m_fib.modify(m_fib->m_fib.iterator_to(*pitEntry.m_fibEntry),
                                ll::bind(&NDNFibEntry::UpdateStatus, ll::_1,
                                         incomingFace, NDNFibFaceMetric::NDN_FIB_GREEN));
        }

        // Add or update entry in the content store
        m_contentStore->Add(header, packet);

        SatisfyPendingInterests(pitEntry, packet);

    } catch (NDNPitEntryNotFound) {
        NS_LOG_INFO("Cannot find PIT entry for data packet.");
        HandleUnsolicitedData(header, packet);
    }
}

void NDNL3Protocol::GiveUpInterest(const NDNPitEntry &pitEntry,
                                   const Ptr<const InterestHeader> &header,
                                   const Ptr<const Packet> &packet)
{
    NS_LOG_FUNCTION_NOARGS();

    if (m_nacksEnabled) {
        Ptr<Packet> nackPacket = Create<Packet>();
        Ptr<InterestHeader> nackHeader = Create<InterestHeader>(*header);
        nackHeader->SetNack(InterestHeader::NACK_GIVEUP_PIT);
        nackPacket->AddHeader(nackHeader);

        BOOST_FOREACH(const NDNPitEntryIncomingFace &incoming, pitEntry.m_incoming) {
            NS_LOG_DEBUG("Send NACK for " << boost::cref(*nackHeader->GetName ()) << " to " << boost::cref(*incoming.m_face));
            incoming.m_face->Send(nackPacket);
        }
    }

    // All incoming interests cannot be satisfied. Remove them
    m_pit->modify(m_pit->iterator_to(pitEntry),
                  ll::bind(&NDNPitEntry::ClearIncoming, ll::_1));

    // Remove also outgoing
    m_pit->modify(m_pit->iterator_to(pitEntry),
                  ll::bind(&NDNPitEntry::ClearOutgoing, ll::_1));

    // Set pruning timout on PIT entry (instead of deleting the record)
    m_pit->modify(m_pit->iterator_to(pitEntry),
                  ll::bind(&NDNPitEntry::SetExpireTime, ll::_1,
                           microsec_clock::local_time() + m_pit->GetPitEntryPruningTimeout()));
}

void NDNL3Protocol::Reap(int fd, short events, void *args)
{
    NS_LOG_FUNCTION_NOARGS();

    Ptr<NDNL3Protocol> protocol = Singleton<NDNL3Protocol>::Get();
    Ptr<NDNPit> pit = protocol->m_pit;

    //NS_LOG_DEBUG("-------------PIT before reaping---------------");
    //pit->Print();
    pit->CleanExpired();
    //NS_LOG_DEBUG("-------------PIT after reaping---------------");
    //pit->Print();

    // schedule next Reap timer
    EventMonitor *em = (EventMonitor *)args;
    em->addTimer(&NDNL3Protocol::Reap, em, &NDNL3Protocol::REAP_INTERVAL);
}

}
