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
 *
 */

#include "ndn-face.h"

#include <stdio.h>
#include <sys/socket.h>

#include "corelib/log.h"
#include "corelib/assert.h"
#include "corelib/singleton.h"
#include "daemon/ndn-l3-protocol.h"
#include "helper/event-monitor.h"
#include "network/packet.h"

using boost::date_time::not_a_date_time;
using boost::posix_time::microsec_clock;


NS_LOG_COMPONENT_DEFINE("NDNFace");


namespace vndn
{

NDNFace::NDNFace()
    : m_bucket(0.0)
    , m_bucketMax(-1.0)
    , m_bucketLeak(0.0)
    , m_lastLeakTime(not_a_date_time)
    , m_metric(0)
{
}

NDNFace::~NDNFace()
{
}

NDNFace::NDNFace(const NDNFace &)
{
}

NDNFace &NDNFace::operator=(const NDNFace &)
{
    return *this;
}

bool NDNFace::IsBelowLimit()
{
    NS_LOG_FUNCTION_NOARGS();

    LeakBucket();

    if (m_bucketMax > 0) {
        NS_LOG_DEBUG ("Limits enabled: " << m_bucketMax << ", current: " << m_bucket);
        if (m_bucket + 1.0 > m_bucketMax) {
            return false;
        }

        m_bucket += 1.0;
    }

    return true;
}

bool NDNFace::Send(const Ptr<const Packet> &p)
{
    int size = p->GetSize();
    int sent;
    if ((sent = send(m_app_fd, p->GetRawBuffer(), size, 0)) < 0) {
        NS_LOG_ERROR("send() failed.");
        return false;
    } else {
        NS_LOG_INFO("Sent " << sent << " bytes through fd " << m_app_fd);
        return true;
    }
}

bool NDNFace::Send(const Ptr<const Packet> &p, RequestSourceInfo *metadata)
{
    return Send(p);
}

bool NDNFace::Receive(const Ptr<const Packet> &packet)
{
    NS_LOG_FUNCTION_NOARGS();

    /// \todo Implement tracing, if requested

    Singleton<NDNL3Protocol>::Get()->Receive(this, packet);
    //m_protocolHandler (this, packet);

    return true;
}

void NDNFace::LeakBucket()
{
    if (m_lastLeakTime == not_a_date_time) {
        m_lastLeakTime = microsec_clock::local_time();
        return;
    }

    boost::posix_time::time_duration interval = microsec_clock::local_time() - m_lastLeakTime;
    const double leak = m_bucketLeak * interval.seconds();
    if (leak >= 1.0) {
        m_bucket = std::max(0.0, m_bucket - leak);
        m_lastLeakTime = microsec_clock::local_time();
    }

    // NS_LOG_DEBUG ("max: " << m_bucketMax << ", Current bucket: " << m_bucket << ", leak size: " << leak << ", interval: " << interval << ", " << m_bucketLeak);
}

void NDNFace::SetBucketMax(double bucket)
{
    NS_LOG_FUNCTION(this << bucket);
    m_bucketMax = bucket;
}

void NDNFace::SetBucketLeak(double leak)
{
    NS_LOG_FUNCTION(this << leak);
    m_bucketLeak = leak;
}

void NDNFace::SetMetric(uint16_t metric)
{
    NS_LOG_FUNCTION(metric);
    m_metric = metric;
}

uint16_t NDNFace::GetMetric(void) const
{
    NS_LOG_FUNCTION_NOARGS();
    return m_metric;
}

/**
 * These are face states and may be distinct from
 * NetDevice states, such as found in real implementations
 * (where the device may be down but face state is still up).
 */

int NDNFace::getMonitorFd() const
{
    return m_app_fd;
}

void NDNFace::exceptHandler()
{
    NS_LOG_FUNCTION_NOARGS();
}

void NDNFace::readHandler(EventMonitor &daemon)
{
    // read message from app_fd and send to NDNL3Protocol
    Ptr<Packet> newPacket;

    try {
        newPacket = Packet::InitFromFD(m_app_fd);
    } catch (PacketException) {
        NS_LOG_ERROR("Packet::InitFromFD() failed.");
        Ptr<Monitorable> pThis(this);
        daemon.erase(pThis);
        return;
    }

    if (newPacket->GetSize() <= 0) {
        Ptr<Monitorable> pThis(this);
        daemon.erase(pThis);
        Singleton<NDNL3Protocol>::Get()->RemoveFace(this);
    } else {
        NS_LOG_INFO("Got a packet of size " << newPacket->GetSize());
        Singleton<NDNL3Protocol>::Get()->Receive(this, newPacket);
    }
}

bool NDNFace::operator==(const NDNFace &face) const
{
    // NS_ASSERT_MSG (m_node->GetId () == face.m_node->GetId (),
    //                "Faces of different nodes should not be compared to each other");

    return getMonitorFd() == face.getMonitorFd();
}

bool NDNFace::operator<(const NDNFace &face) const
{
    // NS_ASSERT_MSG (m_node->GetId () == face.m_node->GetId (),
    //                "Faces of different nodes should not be compared to each other");

    return getMonitorFd() < face.getMonitorFd();
}

std::ostream &NDNFace::Print(std::ostream &os) const
{
    os << "fd=" << getMonitorFd();
    return os;
}

std::ostream &operator<<(std::ostream &os, const NDNFace &face)
{
    face.Print(os);
    return os;
}

} // namespace vndn
