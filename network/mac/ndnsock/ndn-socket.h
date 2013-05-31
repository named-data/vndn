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

#ifndef NDNSOCKET_H_
#define NDNSOCKET_H_

#include "ndn-socket-exception.h"

#include <cstring>
#include <errno.h>
#include <iostream>
#include <string>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <linux/if_arp.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>

namespace vndn
{

/**
 * \brief Supports broadcast communication (MAC layer)
 *
 * Specifies the API for the broadcast communication at MAC layer with no use of IP protocol
 * Although mostly for broadcast communication, it can support also unicast communication without IP (unicast at MAC layer)
 * */
class NdnSocket
{

#define NULLDEVICE "empty"
#define NULLDESTINATION { 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 }

#define DEBUG 1
#define MACPROTO 0x88b5
#define PRINTABLE_MAC_ADDRESS(x) \
    std::hex<<(int)x[0]<<":"<< std::hex<<(int)x[1]<<":"<< std::hex<<(int)x[2]<<":"<< \
    std::hex<<(int)x[3]<<":"<< std::hex<<(int)x[4]<<":"<< std::hex<<(int)x[5]<<std::dec

public:
    /**
     *\brief Create a  NdnSocket, that allows broadcast communication
     *
     * Creates the NDNsocket without bind it to any device. The socket will NOT be ready to use.
     */
    NdnSocket() {}

    /**
     * \brief Create the objects that allows broadcast communication
     *
     *Creates the NDNsocket and binds it to the specified device. The socket will be ready to use.
     *\param deviceName Name of the device name on which the socket has to be binded. Use the string "ALLDEVICE" to bind the socket to all the available network interfaces. Be careful, multiple sockets cannot be binded to the same device
     */
    NdnSocket (std::string deviceName) {}


    /**
     * \brief Sends a broadcast message
     *
     * \param data address of the data that has to be sent
     * \param len length of data
     * \return size of data read from the network
     * */
    virtual int send (void *data, int len) = 0;

    /**
     * \brief Sends a pkt to the specified destination (MAC address)
     *
     * \param data address of the data that has to be sent
     * \param len length of data
     * \param destMacAddress destination MAC address
     * \return size of data read from the network
     * */
    virtual int sendTo (void *data, int len, const unsigned char destMacAddress[ETH_ALEN]) = 0;

    /**
     * \brief Read data from the network
     *
     * \param buffer buffer where the received data can be stored
     * \param maxSize max data size
     * \param flag if 1, it means that the ndnSocket has to store a ndnSocketMetaData at the head of the msg
     * \return size of the received data (plus metadata if present). -1 if there was an error
     * */
    virtual int read (void *buffer, int maxSize, int flag) = 0;

    /**
     * \brief Read data from the network
     *
     * It's equal to read (buffer, maxSize, flag=0)
     * \param buffer buffer where the received data can be stored
     * \param maxSize max data size
     * \return size of the received data. -1 if there was an error
     * */
    virtual int read (void *buffer, int maxSize) = 0;

    /**
     * \brief Read n bytes from the network
     *
     * \param buffer buffer where the received data can be stored
     * \param dataSize size of the data that has to be read
     * \return size of the received data. -1 if there was an error
     * */
    virtual int readn(void *buffer, int dataSize) = 0;

    /**
     * \brief get the interface MAC address
     *
     * \return address of an unsigned char [6 (ETH_ALEN)] where the interface MAC address is stored
     * */
    virtual unsigned char *getDeviceMacAddress() = 0;

    /**
     * \brief Set the destination MAC address
     *
     * \param destMacAddress destination MAC address
     * */
    virtual void setDestination(const unsigned char destMacAddress[ETH_ALEN]) = 0;

    /**
     * \brief Delete any information about the destination
     * */
    virtual void unsetDestination() = 0;

    /**
     * \brief Set the network interface that has to be used by ndnSocket
     * \param deviceName name of the network interface that has to be used to send/receive packets
     * */
    virtual void setDevice (std::string deviceName) = 0;

    /**
     * \brief Get the name of the device used by the class
     * \return Name of the network interface used by ndnSocket to send/receive packets
     * */
    virtual std::string getDevice () = 0;

    /**
     * \briefDeletes any information about the network interface used by ndnSocket
     * */
    virtual void unsetDevice() = 0;

    /**
     *\brief Get the id of a selectable device used to send/receive packets
     *\return a file descriptor that can be used by select to know when there is data available
     * */
    virtual int getSocket () = 0;

    /**
     * \brief Close the file descriptor used to send/receive packets through the network
     * */
    virtual void closeSocket () = 0;

    virtual ~NdnSocket() {}


    /**
     * \brief Metadata
     *
     * It stores the source MAC address of a packet
     * This struct can be used to pass information from the ndnSocket to the higher lever
     * */
    struct ndnSocketMetaData {
        unsigned char sourceMacAddress[ETH_ALEN];
    };
};

} /* namespace vndn */

#endif /* NDNSOCKET_H_ */
