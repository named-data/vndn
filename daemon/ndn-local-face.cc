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
 * Author:  Alexander Afanasyev <alexander.afanasyev@ucla.edu>
 *          Ilya Moiseenko <iliamo@cs.ucla.edu>
 */

#include "ndn-local-face.h"
#include "corelib/log.h"

NS_LOG_COMPONENT_DEFINE ("NDNLocalFace");

namespace vndn
{

NDNLocalFace::NDNLocalFace (int app_fd)
    : NDNFace ()
{
    m_app_fd = app_fd;
}

NDNLocalFace::~NDNLocalFace ()
{
}

NDNLocalFace::NDNLocalFace ()
    : NDNFace ()
{
}

NDNLocalFace::NDNLocalFace (const NDNLocalFace &)
    : NDNFace ()
{
}

NDNLocalFace &NDNLocalFace::operator= (const NDNLocalFace &)
{
    return *((NDNLocalFace *)0);
}



// void
// NDNLocalFace::SendImpl (Ptr<Packet> p)
// {
/* send the packet to the app through m_app_fd */

// try
//   {
//     NDNHeaderHelper::Type type = NDNHeaderHelper::GetNDNHeaderType (p);
//     switch (type)
//       {
//       case NDNHeaderHelper::INTEREST:
//         {
//           Ptr<InterestHeader> header = Create<InterestHeader> ();
//           p->RemoveHeader (*header);

//           if (header->GetNack () > 0)
//             m_app->OnNack (header, p);
//           else
//             m_app->OnInterest (header, p);

//           break;
//         }
//       case NDNHeaderHelper::CONTENT_OBJECT:
//         {
//           static NDNContentObjectTail tail;
//           Ptr<ContentObjectHeader> header = Create<ContentObjectHeader> ();
//           p->RemoveHeader (*header);
//           p->RemoveTrailer (tail);
//           m_app->OnContentObject (header, p/*payload*/);

//           break;
//         }
//       }
//   }
// catch (NDNUnknownHeaderException)
//   {
//   }
// }

std::ostream &NDNLocalFace::Print (std::ostream &os) const
{
    os << "dev=local(" << getMonitorFd() << ")";
    return os;
}

} // namespace vndn
