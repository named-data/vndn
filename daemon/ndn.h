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

#ifndef _NDN_H_
#define _NDN_H_

namespace vndn
{
/**
 * \internal
 * \brief Private namespace for NDN content store implementation
 */
namespace __ndn_private
{
class i_face {};
class i_metric {};
class i_nth {};
class i_prefix {};
class i_ordered {}; ///< tag for Boost.MultiIndex container (ordered by prefix)
class i_mru {};
}

} // vndn

#endif /* _NDN_H_ */
