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

#ifndef _NDN_HEADER_HELPER_H_
#define _NDN_HEADER_HELPER_H_

#include "corelib/ptr.h"

#define INTEREST_BYTE0 0x01
#define INTEREST_BYTE1 0xD2

#define CONTENT_OBJECT_BYTE0 0x04
#define CONTENT_OBJECT_BYTE1 0x82

namespace vndn
{

class Header;
class Packet;

/**
 * \ingroup ndn-helpers
 *
 * \brief Class implementing functionality to detect NDN packet type and
 * create the corresponding object
 *
 * NDN doesn't really have a header, so we need this class to
 * determine type of NDN packet and return corresponent header class,
 * InterestHeader or ContentObjectHeader
 *
 * Throws NDNUnknownHeaderException if header type couldn't be determined
 */
class NDNHeaderHelper
{
public:
    enum Type {INTEREST, CONTENT_OBJECT};

    /**
     * Static function to create an appropriate NDN header
     *
     * It peeks first 2 bytes of a packet.
     *
     *  All interests start with
     *   +-----------------+  +---+---------+-------+
     *   | 0 0 0 0 0 0 0 1 |  | 1 | 1 0 1 0 | 0 1 0 |   (0x01 0xD2)
     *   +-----------------+  +---+---------+-------+
     *
     *  All content objects start with
     *   +-----------------+  +---+---------+-------+
     *   | 0 0 0 0 0 1 0 0 |  | 1 | 0 0 0 0 | 0 1 0 |   (0x04 0x82)
     *   +-----------------+  +---+---------+-------+
     *                          ^             ^^^^^
     *                          |               |
     *                      terminator      DTAG (Dictionary TAG)
     *
     * \see http://www.ndn.org/releases/latest/doc/technical/BinaryEncoding.html
     */

    static Type
    GetNDNHeaderType (Ptr<const Packet> packet);
};

/**
 * \ingroup ndn
 * \defgroup ndn-exceptions Exceptions
 */
/**
 * \ingroup ndn-exceptions
 * \brief Exception thrown if NDN stack receives unrecognized
 * message type
 */
class NDNUnknownHeaderException {};

} // namespace vndn

#endif // _NDN_HEADER_HELPER_H_
