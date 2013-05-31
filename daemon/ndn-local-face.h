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
 *         Ilya Moiseenko <iliamo@cs.ucla.edu>
 */

#ifndef NDN_LOCAL_FACE_H
#define NDN_LOCAL_FACE_H

#include "ndn-face.h"

namespace vndn
{

class InterestHeader;
class ContentObjectHeader;
class Packet;

/**
 * \ingroup ndn-face
 * \brief Implementation of application NDN face
 *
 * This class defines basic functionality of NDN face. Face is core
 * component responsible for actual delivery of data packet to and
 * from NDN stack
 *
 * \see NDNLocalFace, NDNNetDeviceFace, NDNIpv4Face, NDNUdpFace
 */
class NDNLocalFace  : public NDNFace
{
public:

    /**
     * \brief Default constructor
     */
    NDNLocalFace (int app_fd);
    virtual ~NDNLocalFace();

    virtual std::ostream &
    Print (std::ostream &os) const;

private:
    NDNLocalFace ();
    NDNLocalFace (const NDNLocalFace &); ///< \brief Disabled copy constructor
    NDNLocalFace &operator= (const NDNLocalFace &); ///< \brief Disabled copy operator

    //Ptr<NDNAppSocket> m_app;
};

std::ostream &operator<< (std::ostream &os, const NDNLocalFace &localFace);

} // namespace vndn

#endif
