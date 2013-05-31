/*
 * Copyright (c) 2012 University of California, Los Angeles
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
 *
 * Author: Lucas Wang <lucas@cs.ucla.edu>
 */

#include "ndn-content-packet.h"
#include "ndn-content-object-header.h"

namespace vndn
{

NDNContentPacket::NDNContentPacket(Ptr<NameComponents> name, const uint8_t *payload, int payload_size)
{
    /* add header to content packet */
    Ptr<ContentObjectHeader> content_header = Create<ContentObjectHeader>();
    content_header->SetName(name);
    AddHeader(content_header);

    /* create payload packet */
    AddPayload(payload, payload_size);
}

}
