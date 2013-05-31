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
 * Author: Ilya Moiseenko <iliamo@cs.ucla.edu>
 *         Giulio Grassi <giulio.grassi86@gmail.com>
 *         Davide Pesavento <davidepesa@gmail.com>
 */

#include "ndn-consumer.h"
#include "corelib/log.h"
#include "corelib/ptr.h"
#include "network/ndn-content-object-header.h"
#include "network/ndn-interest-header.h"
#include "network/packet.h"

#include <cstdio>
#include <cstdlib>
#include <errno.h>
#include <sys/socket.h>
#include <unistd.h>

NS_LOG_COMPONENT_DEFINE("NDNConsumer");

namespace vndn
{

NDNConsumer::NDNConsumer(int sockfd)
    : m_sock_fd(sockfd)
{
    srand(time(NULL));
    NS_LOG_FUNCTION_NOARGS ();

    m_interestLifeTime = seconds(4);
}
int NDNConsumer::SendPacket(NameComponents &name, int32_t minSuffixComponents, int32_t maxSuffixComponents, bool childSelector, int tos)
{
    Ptr<InterestHeader> interestHeader = Create<InterestHeader>();
    Ptr<NameComponents> namePtr = Create<NameComponents>(name);
    interestHeader->SetNonce(rand());
    interestHeader->SetName(namePtr);
    interestHeader->SetInterestLifetime(m_interestLifeTime);
    interestHeader->SetChildSelector(m_childSelector);
    if (m_exclude.size() > 0) {
        interestHeader->SetExclude(Create<NameComponents>(m_exclude));
    }
    interestHeader->SetMaxSuffixComponents(maxSuffixComponents);
    interestHeader->SetMinSuffixComponents(minSuffixComponents);
    Ptr<Packet> packet = Create<Packet>();
    packet->AddHeader(interestHeader);

    int sent;
    if ((sent = write(m_sock_fd, (void *)packet->GetRawBuffer(), packet->GetSize())) < 0) {
        NS_LOG_ERROR("Failed to write on socket: " << strerror(errno));
    } else {
        NS_LOG_INFO("Sent interest '" << name << "' of " << sent << " bytes to the daemon.");
    }

    return sent;
}
    
int NDNConsumer::SendPacket(NameComponents &name)
{
    return SendPacket(name, 0, 0, false, 0);
}


int NDNConsumer::read(char *buffer, NameComponents & name)
{
    Ptr<Packet> newPacket;
    try {
        newPacket = Packet::InitFromFD(m_sock_fd);
    } catch (PacketException) {
        NS_LOG_WARN("receive a packet by initFromFD failed");
        //Ptr<Monitorable> pThis(this);
        //daemon.erase(pThis);
        return -1;
    }
    if (newPacket->GetSize() <= 0) {
        NS_LOG_INFO("Got a packet of size 0. Exiting...");
        return -1;
    }
    NDNHeaderHelper::Type headerType;
    try {
        headerType = newPacket->GetHeaderType();
    }
    catch (NDNUnknownHeaderException) {
        NS_LOG_WARN("Received a packet with an unknown header type");
        return -1;
    }
    if (headerType == NDNHeaderHelper::CONTENT_OBJECT) {
        NS_LOG_INFO("received a content of "<< newPacket->GetSize() <<" bytes");
        //OnContentObject(newPacket);
        Ptr<ContentObjectHeader> contentHeader = GetHeader<ContentObjectHeader>(*newPacket);
        memcpy(buffer, newPacket->GetPayload(contentHeader->GetSize()), newPacket->GetSize()-contentHeader->GetSize()/*contentHeader->GetSize()*/);
        name = *(contentHeader->GetName());
        return newPacket->GetSize()-contentHeader->GetSize();//contentHeader->GetSize();
        
    } else {
        NS_LOG_INFO("Received a packet, but it's not a content packet");
        return 0;
    }
}

void NDNConsumer::readHandler(EventMonitor &)
{
    Ptr<Packet> newPacket;
    try {
        newPacket = Packet::InitFromFD(m_sock_fd);
    } catch (PacketException) {
        NS_LOG_WARN("receive a packet by initFromFD failed");
        //Ptr<Monitorable> pThis(this);
        //daemon.erase(pThis);
        return;
    }
    if (newPacket->GetSize() <= 0) {
        NS_LOG_INFO("Got a packet of size 0. Exiting...");
        exit(0);
    }

    NDNHeaderHelper::Type headerType;
    try {
       headerType = newPacket->GetHeaderType();
    }
    catch (NDNUnknownHeaderException) {
        NS_LOG_WARN("Received a packet with an unknown header type");
        return;
    }
    if (headerType == NDNHeaderHelper::CONTENT_OBJECT) {
        OnContentObject(newPacket);
    } else {
        NS_LOG_INFO("Received a packet, but it's not a content packet");
    }
}

int NDNConsumer::getMonitorFd() const
{
    return m_sock_fd;
}

void NDNConsumer::OnContentObject(const Ptr<const Packet> &contentPacket)
{
    NS_LOG_INFO("Get a content packet of size " << contentPacket->GetSize());

    Ptr<ContentObjectHeader> contentHeader = GetHeader<ContentObjectHeader>(*contentPacket);
    NS_LOG_INFO("header size:" << contentHeader->GetSize());

    const char *plainText = contentPacket->GetPayload(contentHeader->GetSize());

    NS_LOG_INFO("********************** Data ***************************");
    NS_LOG_INFO(plainText);
    NS_LOG_INFO("*******************************************************");
}

void NDNConsumer::OnNack(const Ptr<const InterestHeader> &interest, Ptr<Packet> origPacket)
{
    NS_LOG_DEBUG ("Nack type: " << interest->GetNack());

    NS_LOG_FUNCTION (this << interest);
}

} // namespace vndn
