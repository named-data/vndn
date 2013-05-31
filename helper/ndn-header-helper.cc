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

#include "ndn-header-helper.h"

#include "corelib/log.h"
#include "network/packet.h"
#include "network/header.h"
//#include "corelib/object.h"

#include "network/ndn-interest-header.h"
#include "network/ndn-content-object-header.h"
#include <iomanip>

NS_LOG_COMPONENT_DEFINE ("NDNHeaderHelper");


namespace vndn
{

NDNHeaderHelper::Type
NDNHeaderHelper::GetNDNHeaderType (Ptr<const Packet> packet)
{
    uint8_t type[2];
    uint32_t read = packet->CopyData (type, 2);

    if (read != 2) throw NDNUnknownHeaderException();

    if (type[0] == INTEREST_BYTE0 && type[1] == INTEREST_BYTE1) {
        return NDNHeaderHelper::INTEREST;
    } else if (type[0] == CONTENT_OBJECT_BYTE0 && type[1] == CONTENT_OBJECT_BYTE1) {
        return NDNHeaderHelper::CONTENT_OBJECT;
    }

    throw NDNUnknownHeaderException();
}

} // namespace vndn
