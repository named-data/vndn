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

#include "ll-metadata-over-ip.h"

namespace vndn
{

LLMetadataOverIP::LLMetadataOverIP()
{
    requestSourceInfoType = OVER_IP;
}

LLMetadataOverIP::LLMetadataOverIP(in_addr_t sourceIP, unsigned short sourcePort)
{
    
    sourceList.insert(sourceList.begin(), std::pair<in_addr_t, unsigned short>(sourceIP,sourcePort));
    requestSourceInfoType = OVER_IP;
}

LLMetadataOverIP::LLMetadataOverIP(std::list< std::pair<in_addr_t, unsigned short> > sourceList)
{
    std::list<std::pair<in_addr_t, unsigned short> >::iterator it;
    for (it = sourceList.begin(); it != sourceList.end(); it++) {
        this->sourceList.insert(this->sourceList.begin(), *it);
    }
}

LLMetadataOverIP::~LLMetadataOverIP()
{
}

in_addr_t LLMetadataOverIP::getIpAddr()
{
    return sourceList.begin()->first;
}
    
unsigned short LLMetadataOverIP::getPort()
{
    return sourceList.begin()->second;
}


std::list< std::pair<in_addr_t, unsigned short> > &LLMetadataOverIP::getSourceList()
{
    return sourceList;
}

} /* namespace vndn */
