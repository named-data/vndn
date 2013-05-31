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

#ifndef LLUPPERLAYERCOMMUNICATIONSERVICE_H_
#define LLUPPERLAYERCOMMUNICATIONSERVICE_H_

#include <errno.h>
#include <sys/eventfd.h>
#include <semaphore.h>
#include <string.h>

#include "link-layer.h"
#include "ll-metadata.h"


namespace vndn
{

/**
 * \brief Manage all the communications between NDN layer (NDNNetDeviceFace) and NDN-LL adaptation layer
 *
 * It uses shared memory and semaphore to manage the communication between NDN and LL
 * There are 2 section of shared memory, one for incoming packets (packet received from network), one for outgoing packet (packet that has to transmitted out)
 * Each section can be used as an array of NDNDevicePktExchange, where it can be stored packets, metadata and additional information to manage the communication between NDN and LL
 * In the incoming section, the NDN-Link adaptation layer will write data, while NDN will read data.
 * In the outgoing section, the NDN-Link adaptation layer will read data, while NDN will write data.
 * If the writer has to write data on an NDNDevicePktExchange that has not been processed yet, it will discard the new data
 * */
class LLUpperLayerCommunicationService
{
public:

    /**
     * \brief number of NDNDevicePktExchange stored in each shared memory (incoming and outgoing section)
     * */
    static const int NDNDevicePktExchangeSize = 100;

    /**
     * \brief Defines the struct stored in the shared memory
     * */
    struct NDNDevicePktExchange {
        /** semaphore to manage concurrency access of NDN and Link layer */
        sem_t mutex;
        /**Size of the stored data*/
        int size;
        /**Data (packet)*/
        uint8_t data[MAXLLSIZE];
        /**Metadata atached to the packet*/
        LLMetadata *metadata;
        /**If true, means that the reader has not processed the data yet*/
        bool used;
        int type; 
    };

    /**
     * \brief Create an empty LLUpperLayerCommunicationService
     *
     * All the variables are set using default value. LLUpperLayerCommunicationService can't be used to manage communication
     * */
    LLUpperLayerCommunicationService();

    /**
     * \brief Creates a LLUpperLayerCommunicationService, specifying shared memory and size
     *
     * \param numElP number of NDNDevicePktExchange stored in each section (incoming-outgoing) of shared memory
     * \param incomingSharedMemoryPtrP address of shared memory dedicated to incoming packets
     * \param outgoingSharedMemoryPtrP address of shared memory reserved to outgoing packets
     * \param outgoingTrigger fd used to communicate that there are new data available in outgoingSharedMemoryPtrP
     * \param incomingTrigger fd used to communicate that there are new data available in incomingSharedMemoryPtrP
     *
     * */
    LLUpperLayerCommunicationService(int numElP, struct NDNDevicePktExchange *incomingSharedMemoryPtrP, struct NDNDevicePktExchange *outgoingSharedMemoryPtrP, int outgoingTrigger, int incomingTrigger);

    virtual ~LLUpperLayerCommunicationService();

    /**
     * \brief Read from outgoing shared memory a NDNDevicePktExchange stored by NDN
     *
     * It will set the NDNDevicePktExchange field used as false. Now NDN can overwrite the data
     * \param buf the function will store the data got from incoming shared memory (the data field of NDNDevicePktExchange)
     * \param maxSize it defines the maximum size of data that can be stored in buf
     * \return size of the data (-1 if an error occurred)
     * */
    int readMessageFromNDN( void *buf, int maxSize);

    /**
     * \brief Read from outgoing shared memory a NDNDevicePktExchange stored by NDN (data plus metadata)
     *
     * It will set the NDNDevicePktExchange field used as false. Now NDN can overwrite the data
     * \param buf the function will store the data got from incoming shared memory (the data field of NDNDevicePktExchange)
     * \param maxSize it defines the maximum size of data that can be stored in buf
     * \param metadata After the function return, it will store the address of the metadata attached to the packet
     * \return size of the data (-1 if an error occurred)
     * */
    int readMessageFromNDN( void *buf, int maxSize, LLMetadata **metadata);

    /**
     * \brief write a new data in incoming shared memory for NDN, plus the LLMetadata attacched
     *
     * It will set the NDNDevicePktExchange field used as true. If used is already true, it discard the new data and return (signaling an error)
     * \param buf it stores the data that has to be written in the incoming shared memory (NDNDevicePktExchange.data)
     * \param size size of data
     * \param metadata metadata attached to the packet. It will be stored in NDNDevicePktExchange.metadata
     * \return the size of the data. -1 if an error occurred
     *
     * */
    int writeMessageToNDN( void *buf, int size, LLMetadata *metadata);

    /**
     * \brief write a new data in incoming shared memory for NDN
     *
     * It will set the NDNDevicePktExchange field used as true. If used is already true, it discard the new data and return (signaling an error)
     * \param buf it stores the data that has to be written in the incoming shared memory (NDNDevicePktExchange.data)
     * \param size size of data
     * \return the size of the data. -1 if an error occurred
     *
     * */
    int writeMessageToNDN( void *buf, int size);

    /**
     * \brief Get the incoming trigger used to signal NDN that new data is available in incoming shared memory
     *
     * \return the fd of the incoming trigger
     * */
    int getNdnIncomingTrigger() const;

    /**
     * \brief Set the incoming trigger used to signal NDN that new data is available in incoming shared memory
     *
     * \param ndnIncomingTrigger fd of the incoming trigger
     * */
    void setNdnIncomingTrigger(int ndnIncomingTrigger);

    /**
     * \brief Get the outgoing trigger used to signal NDN-Link adaptation layer that new data is available in outgoing shared memory
     *
     * \return the fd of the outgoing trigger
     * */
    int getNdnOutgoingTrigger() const;

    /**
     * \brief Set the outgoing trigger used to signal NDN-Link adaptation layer that new data is available in outgoing shared memory
     *
     * \param ndnOutgoingTrigger fd of the outgoing trigger
     * */
    void setNdnOutgoingTrigger(int ndnOutgoingTrigger);

    /**
     * \brief Set the index of the incoming shared memory
     * The index indicates the next incoming NDNDevicePktExchange that has to be used
     * \param index of the incoming shared memory
     * */
    void setIncomingSharedIndex(int incomingSharedIndex);

    /**
     * \brief Set the address of the incoming shared memory
     *
     * \param incomingSharedMemoryPtr address of the incoming shared memory used by NDN-Link adaptation layer to send messages to NDN
     * */
    void setIncomingSharedMemoryPtr(struct NDNDevicePktExchange *incomingSharedMemoryPtr);

    /**
     * \brief Set size of shared memory
     * \param numberOfSharedElement number of NDNDevicePktExchange for each shared memory (incoming and outgoing)
     * */
    void setNumberOfSharedElement(int numberOfSharedElement);

    /**
     * \brief Set the index of the outgoing shared memory
     * The index indicates the next outgoing NDNDevicePktExchange that has to be used
     * \param index of the outgoing shared memory
     * */
    void setOutgoingSharedIndex(int outgoingSharedIndex);

    /**
     * \brief Set the address of the outgoing shared memory
     *
     * \param outgoingSharedMemoryPtr address of the outgoing shared memory used by NDN to send messages to NDN-Link adaptation layer
     * */
    void setOutgoingSharedMemoryPtr(struct NDNDevicePktExchange *outgoingSharedMemoryPtr);

protected:

    /**\brief Address of the incoming shared memory used by NDN-Link adaptation layer to send messages to NDN*/
    struct NDNDevicePktExchange *incomingSharedMemoryPtr;
    /**\brief Address of the outgoing shared memory used by NDN to send messages to NDN-Link adaptation layer */
    struct NDNDevicePktExchange *outgoingSharedMemoryPtr;

    /**\brief index of next incoming NDNDevicePktExchanget hat has to be used  */
    int incomingSharedIndex;
    /**\brief index of next outoing NDNDevicePktExchanget hat has to be used  */
    int outgoingSharedIndex;
    /**Number of NDNDevicePktExchanget stored in each shared memory (incoming/outgoing)*/
    int numberOfSharedElement;


    /**Incoming trigger fd, used by NDN-Link adaptation layer to signal NDN that new data is available*/
    int NDNIncomingTrigger;

    /**Outoing trigger fd, used by NDN to signal NDN-Link adaptation layer that new data is available*/
    int NDNOutgoingTrigger;
};

} /* namespace vndn */
#endif /* UPPERLAYERCOMMUNICATIONSERVICE_H_ */
