/*
 * Copyright (c) 2013 Francesco Berni <kurojishi@gmail.com>
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
 */

#include "corelib/log.h"
#include "corelib/ptr.h"
#include "corelib/singleton.h"
#include "ndn-l3-protocol.h"
#include "ndn-fib.h"
#include "ndn-management.h"
#include "network/buffer.h"

#include <boost/property_tree/json_parser.hpp>
#include <boost/scoped_ptr.hpp>

#define MANAGEMENT_QUEUE "management"
#define MANAGEMENT_QUEUE_SIZE 50 //maximum number of messages in the queue
#define LOCAL_FACE_METRIC 0

using namespace boost::interprocess;
using boost::property_tree::ptree;

NS_LOG_COMPONENT_DEFINE ("NDNManagementInterface");


namespace vndn
{

NDNManagementInterface::NDNManagementInterface()
{
    message_queue::remove(MANAGEMENT_QUEUE);
    management_queue.reset(new message_queue(create_only,MANAGEMENT_QUEUE,MANAGEMENT_QUEUE_SIZE,BUFLEN)); //instance message_queue trought boost scoped pointers
    StartInternalThread();
    NS_LOG_INFO("Management Thread Started");
}

NDNManagementInterface::~NDNManagementInterface()
{
    try {
        pthread_cancel(_thread);
        message_queue::remove(MANAGEMENT_QUEUE);
    } catch(boost::interprocess::interprocess_exception) {
        NS_LOG_ERROR("Failed to stop thread and/or remove message_queue");
    }
    NS_LOG_INFO("Management Thread Stopped");
}

void * NDNManagementInterface::InternalThreadEntryFunc(void * This)
{
    ((NDNManagementInterface *)This)->InternalThreadEntry();
    return NULL;
}

bool NDNManagementInterface::StartInternalThread()
{
    return (pthread_create(&_thread, NULL, InternalThreadEntryFunc, this) == 0);
}

void NDNManagementInterface::prefixRegistration(ptree forwarding_entry)
{
    Ptr<NDNFib> m_fib = Singleton<NDNL3Protocol>::Get()->GetFib();
    Ptr<NDNFace> m_face = Singleton<NDNL3Protocol>::Get()->GetFace(forwarding_entry.get<uint32_t>("faceID"));
    NameComponents prefix(forwarding_entry.get<std::string>("Name"));
    if (m_face == 0) {
        NS_LOG_ERROR(forwarding_entry.get<uint32_t>("faceID") << "not found");
    }
    //for now selfreg and prefixreg are the same as we only register prefixes on the local face
    if (forwarding_entry.get<std::string>("Action") == "selfreg" || forwarding_entry.get<std::string>("Action") == "prefixreg") {
        m_fib->Add(prefix,m_face,LOCAL_FACE_METRIC);
    } else if (forwarding_entry.get<std::string>("Action") == "unreg") {
        if (m_fib->RemovePrefix(prefix,m_face) < 0) {
            NS_LOG_ERROR(prefix << " is not a registered prefix in the ndn daemon");
        }
    } else {
        NS_LOG_WARN("Unsupported Action " << forwarding_entry.get<std::string>("Action") << " for " << forwarding_entry.get<std::string>("ServiceType"));
    }
}

void NDNManagementInterface::InternalThreadEntry()
{
    for(;;) {
        char buffer[BUFLEN];
        message_queue::size_type recvd_size;
        unsigned int priority;
        management_queue->receive(buffer, BUFLEN, recvd_size, priority);
        NS_LOG_INFO("Message received from Management Thread");

        std::istringstream is(buffer);
        ptree container;
        read_json(is, container);

        if (container.get<std::string>("ServiceType") == "ForwardingEntry") {
            prefixRegistration(container);
        } else {
            NS_LOG_WARN("Unsupported Action " << container.get<std::string>("Action") << " for " << container.get<std::string>("ServiceType"));
        }
    }
}

} //namespace vndn
