/*
 * Copyright (c) 2012-2013 Giulio Grassi <giulio.grassi86@gmail.com>
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

#include "ll-upper-layer-comm-service.h"
#include "corelib/log.h"

#include <unistd.h>

NS_LOG_COMPONENT_DEFINE ("LLUpperLayerCommunicationService");

namespace vndn
{

LLUpperLayerCommunicationService::LLUpperLayerCommunicationService()
{
    outgoingSharedMemoryPtr = NULL;
    incomingSharedMemoryPtr = NULL;
    incomingSharedIndex = 0;
    outgoingSharedIndex = 0;
    numberOfSharedElement = 0;
    NDNIncomingTrigger = -1;
    NDNOutgoingTrigger = -1;

}
LLUpperLayerCommunicationService::LLUpperLayerCommunicationService(int numElP, struct NDNDevicePktExchange *incomingSharedMemoryPtrP, struct NDNDevicePktExchange *outgoingSharedMemoryPtrP, int outgoingTrigger, int incomingTrigger)
{
    outgoingSharedMemoryPtr = outgoingSharedMemoryPtrP;
    incomingSharedMemoryPtr = incomingSharedMemoryPtrP;
    incomingSharedIndex = 0;
    outgoingSharedIndex = 0;
    numberOfSharedElement = numElP;
    NDNIncomingTrigger = incomingTrigger;
    NDNOutgoingTrigger = outgoingTrigger;
    //TODO if trigger ==-1 throw exception

}

LLUpperLayerCommunicationService::~LLUpperLayerCommunicationService()
{
    if (NDNIncomingTrigger != -1)
        close(NDNIncomingTrigger);
    if (NDNOutgoingTrigger != -1)
        close(NDNOutgoingTrigger);
}


int LLUpperLayerCommunicationService::readMessageFromNDN(void *buf, int maxSize, LLMetadata **metadata)
{
    if (NDNOutgoingTrigger < 0) {
        NS_LOG_ERROR("file descriptor not valid");
        return -1;
    }
    uint64_t u;
    memset(buf, 0, maxSize);

    int len = read(NDNOutgoingTrigger, &u, sizeof(uint64_t));
    if (len < 0) {
        NS_LOG_ERROR("ERROR, read from local socket failed " << strerror(errno));
        return -1;
        //TODO exception + thread management
    }
    //lock
    if (!outgoingSharedMemoryPtr[outgoingSharedIndex].used) {
        NS_LOG_ERROR("ERROR outgoingSharedIndex is not used");
        return -1;
    }
    sem_wait(&outgoingSharedMemoryPtr[outgoingSharedIndex].mutex);
    memcpy(buf, outgoingSharedMemoryPtr[outgoingSharedIndex].data, outgoingSharedMemoryPtr[outgoingSharedIndex].size);
    outgoingSharedMemoryPtr[outgoingSharedIndex].used = false;
    int size = outgoingSharedMemoryPtr[outgoingSharedIndex].size;

    if (metadata != NULL) {
        *metadata = outgoingSharedMemoryPtr[outgoingSharedIndex].metadata;
    }
    //release lock
    sem_post(&outgoingSharedMemoryPtr[outgoingSharedIndex].mutex);
    outgoingSharedIndex = (outgoingSharedIndex + 1) % NDNDevicePktExchangeSize;
    //TODO is the memcpy necessary? tradeoff: using the memcpy it's possible to immediately release the lock (it then easier to manage the packetStorage), but we waste memory
    NS_LOG_DEBUG("NDNDeviceAdapter read from NDN bytes " << size);
    return size;
}

int LLUpperLayerCommunicationService::readMessageFromNDN( void *buf, int maxSize)
{
    return readMessageFromNDN(buf, maxSize, NULL);
}

int LLUpperLayerCommunicationService::writeMessageToNDN(void *buf, int size, LLMetadata *metadata)
{
    if (NDNIncomingTrigger < 0) {
        NS_LOG_ERROR("file descriptor not valid");
        return -1;
    }
    int sent;

    //wait to get the acces to shared struct
    //int value;
    //sem_getvalue(&incomingSharedPkt[incomingSharedIndex].mutex, &value);
    //std::cout<<"VALUE OF MUTEX " << value <<std::endl;
    sem_wait(&incomingSharedMemoryPtr[incomingSharedIndex].mutex);
    if (incomingSharedMemoryPtr[incomingSharedIndex].used) {
        //the NDNDeviceAdapter has not processed the object yet
        NS_LOG_WARN("NDNDeviceAdapter::writeMessageFromNDN, the NDNDeviceFace has not processed the object yet");
        //TODO is return -1 enough??
        return -1;
    }
    incomingSharedMemoryPtr[incomingSharedIndex].size = size;
    incomingSharedMemoryPtr[incomingSharedIndex].used = true;
    incomingSharedMemoryPtr[incomingSharedIndex].type = 0; //not used yet
    incomingSharedMemoryPtr[incomingSharedIndex].metadata = metadata;
    memcpy(incomingSharedMemoryPtr[incomingSharedIndex].data, buf, size);

    //release lock
    sem_post(&incomingSharedMemoryPtr[incomingSharedIndex].mutex);
    incomingSharedIndex = (incomingSharedIndex + 1) % NDNDevicePktExchangeSize;
    uint64_t u = 1;
    sent = write(NDNIncomingTrigger, &u, sizeof(uint64_t));
    if (sent == -1) {
        NS_LOG_ERROR("ERROR writeMessageToNDN, failed to send a trigger to NDNDeviceFace " << strerror(errno));
        return -1;
    }
    return size;

}

int LLUpperLayerCommunicationService::writeMessageToNDN(void *buf, int size)
{
    return writeMessageToNDN(buf, size, NULL);
}

int LLUpperLayerCommunicationService::getNdnIncomingTrigger() const
{
    return NDNIncomingTrigger;
}

void LLUpperLayerCommunicationService::setNdnIncomingTrigger(int ndnIncomingTrigger)
{
    NDNIncomingTrigger = ndnIncomingTrigger;
}

int LLUpperLayerCommunicationService::getNdnOutgoingTrigger() const
{
    return NDNOutgoingTrigger;
}

void LLUpperLayerCommunicationService::setNdnOutgoingTrigger(int ndnOutgoingTrigger)
{
    NDNOutgoingTrigger = ndnOutgoingTrigger;
}

void LLUpperLayerCommunicationService::setIncomingSharedIndex(int incomingSharedIndex)
{
    this->incomingSharedIndex = incomingSharedIndex;
}

void LLUpperLayerCommunicationService::setIncomingSharedMemoryPtr(struct NDNDevicePktExchange *incomingSharedMemoryPtr)
{
    this->incomingSharedMemoryPtr = incomingSharedMemoryPtr;
}

void LLUpperLayerCommunicationService::setNumberOfSharedElement(int numberOfSharedElement)
{
    this->numberOfSharedElement = numberOfSharedElement;
}

void LLUpperLayerCommunicationService::setOutgoingSharedIndex(int outgoingSharedIndex)
{
    this->outgoingSharedIndex = outgoingSharedIndex;
}

void LLUpperLayerCommunicationService::setOutgoingSharedMemoryPtr(struct NDNDevicePktExchange *outgoingSharedMemoryPtr)
{
    this->outgoingSharedMemoryPtr = outgoingSharedMemoryPtr;
}


} /* namespace vndn */
