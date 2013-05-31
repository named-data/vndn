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

#include "ll-device.h"
#include "corelib/log.h"

NS_LOG_COMPONENT_DEFINE ("LLDevice");


namespace vndn
{

LLDevice::LLDevice()
{
    policy = NULL;
    ndnSocket = NULL;
}

LLDevice::LLDevice(LLPolicy *pol, NdnSocket *sock)
{
    policy = pol;
    ndnSocket = sock;
}

LLDevice::~LLDevice()
{

}

std::string LLDevice::getName() const
{
    return name;
}

void LLDevice::setName(std::string name)
{
    this->name = name;
}

NdnSocket *LLDevice::getNdnSocket() const
{
    return ndnSocket;
}

void LLDevice::setNdnSocket(NdnSocket *ndnSocket)
{
    this->ndnSocket = ndnSocket;
}

LLPolicy *LLDevice::getPolicy() const
{
    return policy;
}

void LLDevice::setPolicy(LLPolicy *policy)
{
    this->policy = policy;
}

} /* namespace vndn */
