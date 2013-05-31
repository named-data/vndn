/*
 * Copyright (c) 2011 University of California, Los Angeles
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
 * Author: Alexander Afanasyev <alexander.afanasyev@ucla.edu>
 */

#ifndef _NDN_PIT_ENTRY_INCOMING_FACE_H_
#define _NDN_PIT_ENTRY_INCOMING_FACE_H_

#include "corelib/ptr.h"
#include "daemon/ndn-face.h"

namespace vndn
{

/**
 * \ingroup ndn
 * \brief PIT state component for each incoming interest (not including duplicates)
 */
struct NDNPitEntryIncomingFace
{
    Ptr<NDNFace> m_face;                    ///< \brief face of the incoming Interest
    boost::posix_time::ptime m_arrivalTime; ///< \brief arrival time of the incoming Interest

public:
    /**
     * \brief Constructor
     * \param face face of the incoming interest
     * \param lifetime lifetime of the incoming interest
     */
    NDNPitEntryIncomingFace(Ptr<NDNFace> face);

    bool operator==(const NDNPitEntryIncomingFace &dst)
    {
        return *m_face == *(dst.m_face);
    }

    bool operator==(Ptr<NDNFace> face)
    {
        return *m_face == *face;
    }

    /**
     * \brief Comparison operator used by boost::multi_index::identity<>
     */
    bool operator<(const NDNPitEntryIncomingFace &m) const
    {
        return *m_face < *(m.m_face);    // return identity of the face
    }
};

} // namespace vndn

#endif  /* NDN_PIT_ENTRY_INCOMING_FACE_H */
