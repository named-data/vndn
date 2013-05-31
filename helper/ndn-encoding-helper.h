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

#ifndef _NDN_ENCODING_HELPER_H_
#define _NDN_ENCODING_HELPER_H_

#include <sys/types.h>
#include <boost/date_time/posix_time/posix_time_types.hpp>

#include "ccnb-parser/ccnb-parser-common.h"
#include "corelib/ptr.h"
#include "network/buffer.h"

using namespace boost::posix_time;

namespace vndn
{

class NameComponents;

class InterestHeader;
class ContentObjectHeader;

/**u
 * \brief Helper to encode/decode ccnb formatted NDN message
 *
 */
class NDNEncodingHelper
{
public:
    /**
     * \brief Serialize InterestHeader to Buffer::Iterator
     * @param start Buffer to store serialized InterestHeader
     * @param interest Pointer to InterestHeader to be serialized
     * @return length of serialized InterestHeader
     */
    static size_t
    Serialize (Buffer &start, const InterestHeader &interest);

    /**
     * \brief Compute the size of serialized InterestHeader
     * @param interest Pointer to InterestHeader
     * @return length
     */
    static size_t
    GetSize (const InterestHeader &interest);

    static size_t
    Serialize (Buffer &start, const ContentObjectHeader &contentObject);

    static size_t
    GetSize (const ContentObjectHeader &contentObject);

private:
    static size_t
    AppendBlockHeader (Buffer &start, size_t value, CcnbParser::ccn_tt block_type);

    static size_t
    EstimateBlockHeader (size_t value);

    static size_t
    AppendNumber (Buffer &start, uint32_t number);

    static size_t
    EstimateNumber (uint32_t number);

    static size_t
    AppendCloser (Buffer &start);

    static size_t
    AppendNameComponents (Buffer &start, const NameComponents &name);

    static size_t
    EstimateNameComponents (const NameComponents &name);

    /**
     * Append a binary timestamp as a BLOB using the ccn binary
     * Timestamp representation (12-bit fraction).
     *
     * @param start start iterator of  the buffer to append to.
     * @param time - Time object
     *
     * @returns written length
     */
    static size_t
    AppendTimestampBlob (Buffer &start, const time_duration &time);

    static size_t
    EstimateTimestampBlob (const time_duration &time);

    /**
     * Append a tagged BLOB
     *
     * This is a ccnb-encoded element with containing the BLOB as content
     *
     * @param start start iterator of  the buffer to append to.
     * @param dtag is the element's dtab
     * @param data points to the binary data
     * @param size is the size of the data, in bytes
     *
     * @returns written length
     */
    static size_t
    AppendTaggedBlob (Buffer &start, CcnbParser::ccn_dtag dtag,
                      const uint8_t *data, size_t size);

    static size_t
    EstimateTaggedBlob (CcnbParser::ccn_dtag dtag, size_t size);
};

} // namespace vndn

#endif // _NDN_ENCODING_HELPER_H_
