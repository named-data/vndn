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

#ifndef _NDN_PIT_ENTRY_OUTGOING_FACE_H_
#define _NDN_PIT_ENTRY_OUTGOING_FACE_H_

#include "corelib/ptr.h"
#include "daemon/ndn-face.h"

namespace vndn
{

/**
 * \ingroup ndn
 * \brief PIT state component for each outgoing interest
 */
struct NDNPitEntryOutgoingFace
{
    Ptr<NDNFace> m_face;                    ///< \brief face of the outgoing Interest
    boost::posix_time::ptime m_sendTime;    ///< \brief time when the first outgoing interest is sent (for RTT measurements)
    ///< \todo handle problem of retransmitted interests... Probably, we should include something similar
    ///<       to time_durationStamp TCP option for retransmitted (i.e., only lost interests will suffer)
    uint32_t m_retxCount;     ///< \brief number of retransmission
    bool m_waitingInVain;     ///< \brief when flag is set, we do not expect data for this interest, only a small hope that it will happen

public:
    NDNPitEntryOutgoingFace(Ptr<NDNFace> face);

    /**
     * @brief Update outgoing entry upon retransmission
     */
    void UpdateOnRetransmit();

    bool operator==(const NDNPitEntryOutgoingFace &dst)
    {
        return *m_face == *dst.m_face;
    }

    bool operator==(Ptr<NDNFace> face)
    {
        return *m_face == *face;
    }

    /**
     * \brief Comparison operator used by boost::multi_index::identity<>
     */
    bool operator<(const NDNPitEntryOutgoingFace &m) const
    {
        return *m_face < *(m.m_face);    // return identity of the face
    }
};

} // namespace vndn

#endif  /* NDN_PIT_ENTRY_OUTGOING_FACE_H */
