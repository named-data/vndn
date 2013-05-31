/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
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
 *         Alexander Afanasyev <alexander.afanasyev@ucla.edu>
 */

#ifndef _NDN_CONTENT_OBJECT_HEADER_H_
#define _NDN_CONTENT_OBJECT_HEADER_H_

#include <string>
#include <vector>
#include <list>
#include "corelib/ptr.h"
#include "network/header.h"
#include "ndn-name-components.h"

namespace vndn
{

/**
 * CCNx XML definition of ContentObject
 *
 * Only few important fields are actually implemented in the simulation
 *
 *
 * ContentObjectHeader serializes/deserializes header up-to and including <Content> tags
 * Necessary closing tags should be added using ContentObjectTail
 *
 * This hacks are necessary to optimize memory use (i.e., virtual payload)
 *
 * "<ContentObject><Signature>..</Signature><Name>...</Name><SignedInfo>...</SignedInfo><Content>"
 *
 */

class ContentObjectHeader : public Header
{
public:
    /**
     * Constructor
     *
     * Creates a null header
     **/
    ContentObjectHeader ();

    /**
     * \brief Set interest name
     *
     * Sets name of the interest. For example, SetName( NameComponents("prefix")("postfix") );
     **/
    void
    SetName (const Ptr<NameComponents> &name);

    Ptr<const NameComponents>
    GetName () const;

    // void
    // SetSignature ();

    // ?
    // GetSignature () const;

    // void
    // SetSignedInfo ();

    // ?
    // GetSignedInfo () const;


    //////////////////////////////////////////////////////////////////
    //virtual void Print (std::ostream &os) const;
    virtual uint32_t GetSize (void) const;
    virtual void Serialize (Buffer &buffer) const;
    virtual uint32_t Deserialize (const Buffer &buf);

private:
    Ptr<NameComponents> m_name;
};

class ContentObjectHeaderException {};

} // namespace vndn

#endif // _NDN_CONTENT_OBJECT_HEADER_H_
