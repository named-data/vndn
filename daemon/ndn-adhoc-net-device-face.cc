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

#include "ndn-adhoc-net-device-face.h"
#include "corelib/log.h"

NS_LOG_COMPONENT_DEFINE ("NDNAdhocNetDeviceFace");

namespace vndn
{

void *startDeviceThread(void *param)
{
    try {
        while (true) {
            NDNDeviceAdapter ndnDeviceAdapt(((NDNDeviceAdapter::NDNDeviceParams *)param)->deviceName);
            ndnDeviceAdapt.start((NDNDeviceAdapter::NDNDeviceParams * )param); //this method should keep going till the ndnDeviceAdpater is running
            NS_LOG_ERROR("NDNDeviceAdapter for device " << ((NDNDeviceAdapter::NDNDeviceParams *)param)->deviceName << " died. Retrying...");
            sleep(5);
        }
    } catch (const char *err) {
        NS_LOG_ERROR("Failed to create NDNDeviceAdapter for ad-hoc face: " << err << ". Exiting.");
    }

    return 0;
}

NDNAdhocNetDeviceFace::NDNAdhocNetDeviceFace(std::string deviceNameP)
    : NDNFace()
{
    deviceName = deviceNameP;
    m_app_fd = ::eventfd(0, EFD_SEMAPHORE);
    if (m_app_fd == -1) {
        std::string err = "Failed to create eventfd for incoming packet: ";
        err += strerror(errno);
        throw NDNLinkLayerCommunication(err);
    }
    param.incomingEventFd = m_app_fd;
    outgoingPktTrigger = ::eventfd(0, EFD_SEMAPHORE);
    if (outgoingPktTrigger == -1) {
        std::string err = "Failed to create eventfd for outgoing packet: ";
        err += strerror(errno);
        throw NDNLinkLayerCommunication(err);
    }

    param.outgoingEventFd = outgoingPktTrigger;
    param.deviceName = deviceName;
    param.numEl = LLUpperLayerCommunicationService::NDNDevicePktExchangeSize;
    //TODO check is NDNPktSharedWithDeviceAdapter is already initialize
    param.incomingSharedMemoryPtr = &NDNIncomingPktSharedWithDeviceAdapter[0];
    param.outgoingSharedMemoryPtr = &NDNOutgoingPktSharedWithDeviceAdapter[0];
    for (int i = 0; i < LLUpperLayerCommunicationService::NDNDevicePktExchangeSize; i++) {
        if (sem_init(&NDNOutgoingPktSharedWithDeviceAdapter[i].mutex, 0, 1) == -1) {
            std::string err = "Failed the outgoing sem_init(): ";
            err += strerror(errno);
            throw NDNLinkLayerCommunication(err);
        }
        if (sem_init(&NDNIncomingPktSharedWithDeviceAdapter[i].mutex, 0, 1) == -1) {
            std::string err = "Failed the incoming sem_init(): ";
            err += strerror(errno);
            throw NDNLinkLayerCommunication(err);
        }
    }
    incomingPktSharedMemoryIndex = 0;
    outgoingPktSharedMemoryIndex = 0;
    NS_LOG_DEBUG("incomingSharedMemoryPtr = " << NDNIncomingPktSharedWithDeviceAdapter);
    NS_LOG_DEBUG("outgoingSharedMemoryPtr = " << NDNOutgoingPktSharedWithDeviceAdapter);

    ndnDeviceAdapterThreadId = pthread_create(&NDNDeviceAdapterT, NULL, startDeviceThread, &param);
    if (ndnDeviceAdapterThreadId == -1) {
        std::string err = "Thread creation failed: ";
        err += strerror(errno);
        throw NDNLinkLayerCommunication(err);
    }

    sleep(2); //is not the best way to do it, but for now it works. we wait a while to see if ndnDeviceAdatper is still on

    int ret = pthread_kill(NDNDeviceAdapterT, 0);
    if (ret == ESRCH) {
        void *res;
        pthread_join(NDNDeviceAdapterT, &res);
        throw NDNLinkLayerCommunication("NDNAdhocNetDeviceFace thread died");
    }

    NS_LOG_DEBUG("eventfd: " << m_app_fd);
}


bool NDNAdhocNetDeviceFace::Send(const Ptr<const Packet> &p)
{
    NS_LOG_FUNCTION_NOARGS();

    int sent;
    //wait to get the acces to shared struct
    sem_wait(&NDNOutgoingPktSharedWithDeviceAdapter[outgoingPktSharedMemoryIndex].mutex);
    if (NDNOutgoingPktSharedWithDeviceAdapter[outgoingPktSharedMemoryIndex].used) {
        NS_LOG_WARN("NDNDeviceAdapter has not processed the object yet");

        //checking is NDNAdHocNetDeviceFace is still alive
        int ret = pthread_kill(NDNDeviceAdapterT, 0);
        if (ret == ESRCH) {
            NS_LOG_ERROR("NDNAdhocNetDeviceFace thread died");
            void *res;
            pthread_join(NDNDeviceAdapterT, &res);
        }
        return false;
    }

    NDNOutgoingPktSharedWithDeviceAdapter[outgoingPktSharedMemoryIndex].size = p->GetSize(); //p->GetSerializedSize();
    NDNOutgoingPktSharedWithDeviceAdapter[outgoingPktSharedMemoryIndex].used = true;
    NDNOutgoingPktSharedWithDeviceAdapter[outgoingPktSharedMemoryIndex].type = 0; //not used yet

    if ((p->llmetadata != NULL) && (p->llmetadataptr != NULL)) {
        if (p->llmetadataptr->getRequestSourceInfoType() == OVER_ADHOC) {
            NDNOutgoingPktSharedWithDeviceAdapter[outgoingPktSharedMemoryIndex].metadata = p->llmetadataptr;
        } else {
            NDNOutgoingPktSharedWithDeviceAdapter[outgoingPktSharedMemoryIndex].metadata = NULL;
        }
    } else {
        NDNOutgoingPktSharedWithDeviceAdapter[outgoingPktSharedMemoryIndex].metadata = NULL;
    }
    *(p->llmetadata) = NULL;
    memcpy(NDNOutgoingPktSharedWithDeviceAdapter[outgoingPktSharedMemoryIndex].data, p->GetRawBuffer(), p->GetSize());

    //release lock
    sem_post(&NDNOutgoingPktSharedWithDeviceAdapter[outgoingPktSharedMemoryIndex].mutex);
    outgoingPktSharedMemoryIndex = (outgoingPktSharedMemoryIndex + 1) % LLUpperLayerCommunicationService::NDNDevicePktExchangeSize;
    uint64_t u = 1;
    sent = write(outgoingPktTrigger, &u, sizeof(uint64_t));
    if (sent == -1) {
        NS_LOG_ERROR("Failed to send a trigger to NDNDeviceAdapter: " << strerror(errno));
        return false;
    }

    NS_LOG_DEBUG("Packet sent to NDNDeviceAdapter");
    return true;
}

void NDNAdhocNetDeviceFace::readHandler(EventMonitor &daemon)
{
    NS_LOG_FUNCTION_NOARGS();

    uint64_t u;
    int s = read(m_app_fd, &u, sizeof(uint64_t));
    if (s < 0) {
        NS_LOG_ERROR("Reading the trigger from NDNDeviceAdapter failed: " << strerror(errno));
        //Ptr<Monitorable> pThis(this);
        //daemon.erase(pThis); // remove the current face from the daemon
        //exit(0)
        //TODO manage error: ignore everything and keep going or kill the NDNDeviceAdapter thread and try again???
        return;
    }

    //wait to get the acces to shared struct
    sem_wait(&NDNIncomingPktSharedWithDeviceAdapter[incomingPktSharedMemoryIndex].mutex);

    unsigned char buf[BUFLEN];
    memcpy(buf, NDNIncomingPktSharedWithDeviceAdapter[incomingPktSharedMemoryIndex].data, NDNIncomingPktSharedWithDeviceAdapter[incomingPktSharedMemoryIndex].size);
    NDNIncomingPktSharedWithDeviceAdapter[incomingPktSharedMemoryIndex].used = false;
    int size = NDNIncomingPktSharedWithDeviceAdapter[incomingPktSharedMemoryIndex].size;

    NS_LOG_INFO("Got " << size << " bytes from fd " << m_app_fd);
    const Ptr<Packet> newPacket = Packet::InitFromBuffer(buf, size);

    //metadata
    newPacket->llmetadataptr = (NDNIncomingPktSharedWithDeviceAdapter[incomingPktSharedMemoryIndex].metadata);
    newPacket->llmetadata = &(newPacket->llmetadataptr);
    //newPacket->setLLMetadata(NDNIncomingPktSharedWithDeviceAdapter[incomingPktSharedMemoryIndex].metadata);

    //release lock
    sem_post(&NDNIncomingPktSharedWithDeviceAdapter[incomingPktSharedMemoryIndex].mutex);

    incomingPktSharedMemoryIndex = (incomingPktSharedMemoryIndex + 1) % LLUpperLayerCommunicationService::NDNDevicePktExchangeSize;

    Receive(newPacket);
}

std::ostream &NDNAdhocNetDeviceFace::Print(std::ostream &os) const
{
    os << "dev=adhoc(" << deviceName << ")";
    return os;
}

} // namespace vndn
