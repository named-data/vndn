/* -*- Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil -*- */
/*
 * Copyright (c) 2011 University of California, Los Angeles
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Alexander Afanasyev <alexander.afanasyev@ucla.edu>
 */

#include "ccnb-parser-timestamp-visitor.h"
#include "../syntax-tree/ccnb-parser-blob.h"
#include <boost/date_time/posix_time/posix_time.hpp>

using namespace boost::posix_time;

namespace vndn
{

namespace CcnbParser
{

boost::any/*Time*/
TimestampVisitor::visit (Blob &n)
{
    // Buffer n.m_blob;
    if (n.m_blobSize < 2)
        throw CcnbDecodingException ();

    const char *start = n.m_blob;

    intmax_t sec = 0;
    intmax_t usec = 0;

    for (uint32_t i = 0; i < n.m_blobSize - 2; i++) {
        sec = (sec << 8) | (uint8_t)start[i];
    }
    uint8_t combo = start[n.m_blobSize - 2]; // 4 most significant bits hold 4 least significant bits of number of sec
    sec = (sec << 4) | (combo >> 4);

    usec = combo & 0x0F; /*00001111*/ // 4 least significant bits hold 4 most significant bits of fraction of sec
    usec = (usec << 8) | start[n.m_blobSize - 1];
    usec = (intmax_t) ((usec / 4096.0/*2^12*/) * 1000000 /*up-convert microseconds*/);

    return boost::any(seconds(sec) + microseconds(usec));
}

boost::any
TimestampVisitor::visit (Udata &n)
{
    // std::string n.m_udata;
    throw CcnbDecodingException ();
}

}
}
