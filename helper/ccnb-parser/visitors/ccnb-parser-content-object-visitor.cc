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

#include "ccnb-parser-content-object-visitor.h"
#include "ccnb-parser-name-components-visitor.h"

#include "../syntax-tree/ccnb-parser-block.h"
#include "../syntax-tree/ccnb-parser-dtag.h"

#include "network/ndn-name-components.h"
#include "corelib/assert.h"

#include "network/ndn-content-object-header.h"

#include <boost/foreach.hpp>

namespace vndn
{
namespace CcnbParser
{

// We don't really care about any other fields
void
ContentObjectVisitor::visit (Dtag &n, boost::any param/*should be ContentObjectHeader* */)
{
    // uint32_t n.m_dtag;
    // std::list<Ptr<Block> > n.m_nestedBlocks;
    static NameComponentsVisitor nameComponentsVisitor;

    ContentObjectHeader &contentObject = *(boost::any_cast<ContentObjectHeader *> (param));

    switch (n.m_dtag) {
    case CCN_DTAG_ContentObject:
        // process nested blocks
        BOOST_FOREACH (Ptr<Block> block, n.m_nestedTags) {
            block->accept (*this, param);
        }
        break;
    case CCN_DTAG_Name: {
        // process name components
        Ptr<NameComponents> name = Create<NameComponents> ();

        BOOST_FOREACH (Ptr<Block> block, n.m_nestedTags) {
            block->accept (nameComponentsVisitor, &(*name));
        }
        contentObject.SetName (name);
        break;
    }
    case CCN_DTAG_Signature: // ignoring
        break;
    case CCN_DTAG_SignedInfo: // ignoring
        break;
    case CCN_DTAG_Content: // !!! HACK
        // This hack was necessary for memory optimizations (i.e., content is virtual payload)
        NS_ASSERT_MSG (n.m_nestedTags.size() == 0, "Parser should have stopped just after processing <Content> tag");
        break;
    }
}

} // namespace CcnbParser
} // namespace vndn
