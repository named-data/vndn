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

#ifndef LLMETADATAOVERIP_H_
#define LLMETADATAOVERIP_H_

#include <netinet/in.h>
#include <list>
#include <utility>

#include "ll-metadata.h"

namespace vndn
{

class LLMetadataOverIP :  public LLMetadata
{
public:
    LLMetadataOverIP();
    LLMetadataOverIP(in_addr_t sourceIP, unsigned short sourcePort);
    LLMetadataOverIP(std::list< std::pair<in_addr_t, unsigned short> > sourceList);
    virtual ~LLMetadataOverIP();

    in_addr_t getIpAddr();
    unsigned short getPort();
    
    std::list< std::pair<in_addr_t, unsigned short> > &getSourceList();




protected:
    std::list< std::pair<in_addr_t, unsigned short> > sourceList;
};

} /* namespace vndn */
#endif /* LLMETADATAOVERIP_H_ */
