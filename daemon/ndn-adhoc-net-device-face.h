/*
 * Copyright (c) 2012-2013 Giulio Grassi <giulio.grassi86@gmail.com>
 *                         Davide Pesavento <davidepesa@gmail.com>
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

#ifndef NDN_ADHOC_NET_DEVICE_FACE_H
#define NDN_ADHOC_NET_DEVICE_FACE_H

#include <stdio.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/un.h>
#include <sys/eventfd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>

#include "ndn-face.h"
#include "ndn-l3-protocol.h"
#include "helper/event-monitor.h"
#include "network/packet.h"
#include "network/mac/link-layer.h"
#include "network/mac/ll-upper-layer-comm-service.h"
#include "network/mac/ndn-device-adapter.h"

namespace vndn
{

class EventMonitor;

/**
 * \ingroup ndn-face
 * \brief Implementation of layer-2 (Ethernet) NDN face
 *
 * This class defines basic functionality of NDN face. Face is core
 * component responsible for actual delivery of data packet to and
 * from NDN stack
 *
 * NDNNetDevice face is permanently associated with one NetDevice
 * object and this object cannot be changed for the lifetime of the
 * face
 *
 * It used shared memory to communicate with NDN-Link Adaptation Layer (NDN-LAL).
 * One section of the shared memory is reserved for the incoming traffic (packets and relative additional information that NDN-LAL received from the network), the seconds for the outgoig traffic (packet and relative metadata that NDN wants to send over the network through NDN-LAL.
 *
 * \see NDNLocalFace, NDNNetDeviceFace, NDNIpv4Face, NDNUdpFace
 */
class NDNAdhocNetDeviceFace : public NDNFace
{
public:
    /**
    * /brief Creates a NDNAdhocNetDeviceFace and the relative NDNDeviceAdapter
    *
    * It creates a NDNAdhocNetDeviceFace and binds it to a network interface
    * It creates also a NDNDeviceAdapter and the relative thread, that is going to run the NDN-LAL for this network interface
    * The shared memory and all the objects (semaphores ...) used to mamange the communication between the face and NDN-LAL are created in this phase
    *
    * \param deviceNameP name of the network interface
    * */
    NDNAdhocNetDeviceFace(std::string deviceNameP);

    /**
     * \brief Read data from NDN-LAL, using the incoming shared memory
     *
     * Reads an NDN packet and its metadata (if present) form the incoming shared memory.
     * The NDNDevicePktExchange stored inside the shared memory will be set as unused, so that NDN-LAL can use it again
     * */
    virtual void readHandler(EventMonitor &em);

    /**
     *\brief Sends a packet to NDN-LAL requesting its transmission over the network
     *
     * Sends a packet to NDN-LAL using the outoigin shared memory. It stores the NDN packet plus the relative metadata (if present)
     * If the NDNDevicePktExchange that should be used (check outgoingPktSharedMemoryIndex) is still set as used, it means that NDN-LAL has not read yet the data that is stores. In this case, the face will discard the new packet and abort the transmission
     * \param p pointer to the NDN packet that has to be sent over the network
     * \return true is the packet has been sent to NDN-LAL, false otherwise
     * */
    virtual bool Send(const Ptr<const Packet> &p);

    virtual std::ostream &Print(std::ostream &os) const;

private:
    /**Trigger (eventfd) used by netDeviceFace to signal to NDNDeviceAdapter that there is something ready stored in the shared memory */
    int outgoingPktTrigger;
    /**Name of the network interface associate to this ndn-face*/
    std::string deviceName;

    /**
     * shared memory (with NDNDeviceAdapter) used to store packets that have to be sent out over the network
     * */
    LLUpperLayerCommunicationService::NDNDevicePktExchange NDNOutgoingPktSharedWithDeviceAdapter[LLUpperLayerCommunicationService::NDNDevicePktExchangeSize];
    /**
     * shared memory (with NDNDeviceAdapter) used to store packets received by NDNDeviceAdapter from the network
     * */
    LLUpperLayerCommunicationService::NDNDevicePktExchange NDNIncomingPktSharedWithDeviceAdapter[LLUpperLayerCommunicationService::NDNDevicePktExchangeSize];

    /**
     * Indicates which is the next block in the shared memory (incoming pkt section) that can be used
     * */
    int incomingPktSharedMemoryIndex;

    /**
     * Indicates which is the next block in the shared memory (outpogin pkt section) that can be used
     * */
    int outgoingPktSharedMemoryIndex;

    /**
     * Used to store al the parameter that NDNDeviceAdapter needs to run (NDNDeviceAdapter is createb by this face and run as a new thread)
     * */
    struct NDNDeviceAdapter::NDNDeviceParams param;

    int ndnDeviceAdapterThreadId;
    pthread_t NDNDeviceAdapterT;

};


class NDNLinkLayerCommunication
{
public:
    NDNLinkLayerCommunication(const std::string &str) : msg(str) {}
    std::string error() const { return msg; }

private:
    std::string msg;
};

} // namespace vndn

#endif // NDN_ADHOC_NET_DEVICE_FACE_H
