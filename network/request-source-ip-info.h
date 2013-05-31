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

#ifndef REQUEST_SOURCE_IP_INFO_H
#define REQUEST_SOURCE_IP_INFO_H

#include <list>
#include <iostream>
#include <arpa/inet.h>

#include "request-source-info.h"


namespace vndn
{

/**
 * \brief It store the IP addresses of the nodes that sent the interest over IP
 *
 * It stores the list of IP address used by the nodes to send this interest over IP
 * The expiration time of this kinf of information is not managed inside this object. It has to be done externally
 * */
class RequestSourceIPInfo : public RequestSourceInfo
{

public:
    RequestSourceIPInfo();

    virtual ~RequestSourceIPInfo();

    /**
     * \brief Add an address to the IP list
     *
     * No check on the new ip is done
     * */
    void addIP(in_addr_t IPAddress, unsigned short port );

    /**
     * \brief return the list of ip addresses
     * */
    std::list< std::pair <in_addr_t, unsigned short> > &getAllSource();


protected:

    /**
     * \brief IP list (network byte order!!)
     * */
    std::list< std::pair <in_addr_t, unsigned short> > sourceList;

};
}


#endif  //REQUEST_SOURCE_IP_INFO_H
