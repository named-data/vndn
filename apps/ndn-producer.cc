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
 * Author: Lucas Wang <lucas@cs.ucla.edu>
 *         Giulio Grassi <giulio.grassi86@gmail.com>
 *         Davide Pesavento <davidepesa@gmail.com>
 *         Francesco Berni <kurojishi@gmail.com>
 */

#include "ndn-producer.h"
#include "corelib/log.h"
#include "network/packet.h"
#include "network/ndn-content-packet.h"
#include "network/ndn-interest-header.h"
#include "network/ndn-content-object-header.h"
#include "helper/ndn-header-helper.h"

#include <unistd.h>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/interprocess/ipc/message_queue.hpp>

#define MANAGEMENT_QUEUE "management"

using namespace boost::interprocess;
using namespace boost::property_tree;

NS_LOG_COMPONENT_DEFINE("NDNProducer");

namespace vndn
{

NDNProducer::NDNProducer(int sockfd)
    : m_sock_fd(sockfd)
{
    NS_LOG_FUNCTION_NOARGS();
}

void NDNProducer::readHandler(EventMonitor &)
{
    Ptr<Packet> newPacket;
    try {
        newPacket = Packet::InitFromFD(m_sock_fd);
    } catch (PacketException) {
        NS_LOG_WARN("receive a packet by initFromFD failed");
        return;
    }
    /* read the interest packet from the socket and reply with content packet */
    if (newPacket->GetSize() <= 0) {
        NS_LOG_INFO("Got a packet of size 0. Exiting...");
        exit(0);
    }

    NDNHeaderHelper::Type packetType;
    try {
        packetType = newPacket->GetHeaderType();
    } catch (NDNUnknownHeaderException) {
        NS_LOG_WARN("Received a packet with an unknown header type");
        return;
    }
    if (packetType == NDNHeaderHelper::INTEREST) {
        NS_LOG_INFO("Received an interest packet.");

        Ptr<InterestHeader> interest = GetHeader<InterestHeader>(*newPacket);
        std::string replyContent = "Codroipo!" + interest->GetName()->GetLastComponent();
        NS_LOG_INFO("Replying with content: " << replyContent);

        Ptr<NDNContentPacket> contentPkt = Create<NDNContentPacket>(
                    Create<NameComponents>(*(interest->GetName())),
                    (const uint8_t *)replyContent.c_str(),
                    replyContent.length());

        int sent = write(m_sock_fd, contentPkt->GetRawBuffer(), contentPkt->GetSize());
        NS_LOG_INFO("Sent " << sent << " bytes of content with name '" << * (interest->GetName()) << "'");
    }
}

int NDNProducer::getMonitorFd() const
{
    return m_sock_fd;
}

int NDNProducer::read(NameComponents & nameComponent)
{
    Ptr<Packet> newPacket;
    try {
        newPacket = Packet::InitFromFD(m_sock_fd);
    } catch (PacketException) {
        NS_LOG_WARN("receive a packet by initFromFD failed");
        return -1;
    }
    if (newPacket->GetSize() <= 0) {
        NS_LOG_INFO("Got a packet of size 0. Exiting...");
        return -1;
    }

    NDNHeaderHelper::Type headerType;
    try {
        headerType = newPacket->GetHeaderType();
    } catch (NDNUnknownHeaderException) {
        NS_LOG_WARN("Received a packet with an unknown header type");
        return -1;
    }
    if (headerType == NDNHeaderHelper::INTEREST) {
        Ptr<InterestHeader> interest = GetHeader<InterestHeader>(*newPacket);
        nameComponent = *(interest->GetName());//->GetLastComponent();
        NS_LOG_INFO("Received interest :" <<nameComponent << " " << *interest->GetName());
        return 1;
    } else {
        NS_LOG_INFO("Received a packet, but it's not an interest packet");
        return 0;
    }
}

int NDNProducer::sendContent(NameComponents& name, std::string content)
{
    NS_LOG_FUNCTION_NOARGS();
    int sent = sendContent(name,(uint8_t*) content.c_str(),content.length());
    NS_LOG_INFO("Sent " << sent << " bytes of content with name " << name<<" content "<<content);
    return sent;
}

int NDNProducer::sendContent(NameComponents & name, const uint8_t * content, int size)
{
    NS_LOG_FUNCTION_NOARGS();
    Ptr<NDNContentPacket> contentPkt = Create<NDNContentPacket>(Create<NameComponents>(name), content, size);
    int sent = write(m_sock_fd, contentPkt->GetRawBuffer(), contentPkt->GetSize());
    NS_LOG_INFO("Sent " << sent << " bytes of content with name " << name);
    return sent;
}

void NDNProducer::registerName(NameComponents &name, uint32_t face_fd)
{
    message_queue management_queue(open_only,MANAGEMENT_QUEUE);
    ptree forwarding_entry;
    forwarding_entry.put("ServiceType","ForwardingEntry");
    forwarding_entry.put("Action","selfreg");
    forwarding_entry.put("faceID",face_fd);
    std::stringstream prefix;
    prefix << name;
    forwarding_entry.put("Name",prefix.str());
    std::ostringstream oss;
    write_json(oss,forwarding_entry);
    management_queue.send(oss.str().c_str(),oss.str().size(),0);
}

void NDNProducer::deregisterName(NameComponents &name, uint32_t face_fd)
{
    message_queue management_queue(open_only,MANAGEMENT_QUEUE);
    ptree forwarding_entry;
    forwarding_entry.put("ServiceType","ForwardingEntry");
    forwarding_entry.put("Action","unreg");
    forwarding_entry.put("faceID",face_fd);
    std::stringstream prefix;
    prefix << name;
    forwarding_entry.put("Name",prefix.str());
    std::ostringstream oss;
    write_json(oss,forwarding_entry);
    management_queue.send(oss.str().c_str(),oss.str().size(),0);
}

} // namespace vndn
