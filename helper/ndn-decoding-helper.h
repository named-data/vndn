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

#ifndef _NDN_DECODING_HELPER_H_
#define _NDN_DECODING_HELPER_H_

#include <cstring>
#include "network/buffer.h"

namespace vndn
{

class InterestHeader;
class ContentObjectHeader;

/**
 * \brief Helper class to decode ccnb formatted NDN message
 */
class NDNDecodingHelper
{
public:
    /**
     * \brief Deserialize Buffer::Iterator to InterestHeader
     * @param start Buffer containing serialized NDN message
     * @param interest Pointer to the InterestHeader to hold deserialized value
     * @return Number of bytes used for deserialization
     */
    static size_t
    Deserialize (const Buffer &start, InterestHeader &interest);

    /**
     * \brief Deserialize Buffer::Iterator to ContentObjectHeader
     * @param start Buffer containing serialized NDN message
     * @param contentObject Pointer to the ContentObjectHeader to hold deserialized value
     * @return Number of bytes used for deserialization
     */
    static size_t
    Deserialize (const Buffer &start, ContentObjectHeader &contentObject);
};
} // namespace vndn

#endif // _NDN_DECODING_HELPER_H_
