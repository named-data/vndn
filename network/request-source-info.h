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

#ifndef REQUEST_SOURCE_INFO_H
#define REQUEST_SOURCE_INFO_H

namespace vndn
{

/**
 * \brief It store information about the node that generates or sends the interest
 *
 * Each type of NDNDeviceFace can use this object to store some information about the interest source. The NDNDeviceFace is the one that decides which type of information will be stored. A RequestSourceInfo is just an interface
 * The expiration time of this kinf of information is not managed inside this object. It has to be done externally
 * */
class RequestSourceInfo
{
public:
    RequestSourceInfo() {}
    virtual ~RequestSourceInfo() {}
};

}

#endif  //REQUEST_SOURCE_INFO_H
