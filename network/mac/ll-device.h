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

#ifndef LLDEVICE_H_
#define LLDEVICE_H_

#include <string>

#include "ndnsock/ndn-socket.h"
#include "ll-policy.h"

namespace vndn
{

/**
 * \brief Store some useful information about the network interface
 *
 * It store information as the name of the device, the NdnSocket binded on the device and used for the no-IP broadcast communication, the reference to the link layer policy that applies for that device
 * */
class LLDevice
{
public:

    /**
     * \brief Create an empty device
     * */
    LLDevice();

    /**
     * \brief Create a device and attach to it a NDN-LAL policy and a NdnSocket
     * \param pol NDN-LAL policy that has to manage all the communication through this device
     * \param sock NdnSocket used to send / receive packet on the network through this device
     * */
    LLDevice(LLPolicy *pol, NdnSocket *sock);
    virtual ~LLDevice();

    /**
     * \brief Get the name of the device
     * \return A string with the name of the network interface
     * */
    std::string getName() const;

    /**
     * \brief Set the network interface name
     * \param name device name
     * */
    void setName(std::string name);

    /**
     * \brief Get the NDNSocket binded to the device
     * \return a reference to the ndnSocket used for the broadcast communication through that device
     * */
    NdnSocket *getNdnSocket() const;

    /**
     * \brief Set the NDNSocket
     * \param ndnSocket reference to the NDNDRawSocket binded to the device
     * */
    void setNdnSocket(NdnSocket *ndnSocket);

    /**
     * \brief Get the link layer policy that applies on that device
     * \return the NOM policy reference
     * */
    LLPolicy *getPolicy() const;

    /**
     * \brief Set the link layer policy that applies to that device
     * \param policy link layer policy object
     * */
    void setPolicy(LLPolicy *policy);


protected:
    /**Link layer policy*/
    LLPolicy *policy;

    /**Reference to the NDNSocket binded to the device*/
    NdnSocket *ndnSocket;

    /**Network interface name*/
    std::string name;

};
} /* namespace vndn */
#endif /* LLDEVICE_H_ */
