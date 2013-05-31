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
 * Author: Ilya Moiseenko <iliamo@cs.ucla.edu>
 */

#ifndef NDN_HASH_HELPER_H
#define NDN_HASH_HELPER_H

#include <iostream>
#include <string>
#include <boost/foreach.hpp>
#include "network/ndn-name-components.h"

namespace vndn
{

/**
 * \ingroup ndn-helpers
 * \brief Helper providing hash value for the name prefix
 *
 * The whole prefix is considered as a long string with '/' delimiters
 *
 * \todo Testing is required to determine if this hash function
 * actually provides good hash results
 */
struct NDNPrefixHash : public std::unary_function<NameComponents, std::size_t> {
    std::size_t operator() (const NameComponents &prefix) const {
        std::size_t hash = 23;
        BOOST_FOREACH (const std::string & str, prefix.GetComponents()) {
            hash += str.size();
            hash = ((hash << 6) ^ (hash >> 27)) + '/';
            BOOST_FOREACH(char c, str) {
                hash = ((hash << 6) ^ (hash >> 27)) + c;
            }
        }
        //cout << "hash for " << prefix << ":" << hash << endl;

        return hash;
    }
};

} // namespace vndn

#endif // NDN_HASH_HELPER_H
