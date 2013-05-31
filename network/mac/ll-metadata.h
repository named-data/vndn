/*
 * Copyright (c) 2012-2013 Giulio Grassi <giulio.grassi86@gmail.com>
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
 */

#ifndef LLMETADATA_H_
#define LLMETADATA_H_

namespace vndn
{

#define NULL_TYPE -1
#define OVER_IP 1
#define OVER_ADHOC 2

/**
 * \brief Class (interface) used to defined the metadata used in the communication between NDN (Net device face) and Link Layer (NDNDeviceAdapter)
 *
 * Sometimes when packet goes from a level to an other it could be useful to carry additional information (metadata) about itself
 * */
class LLMetadata
{
public:

    /**
     * \brief It creates an empty LLMetadata
     * */
    LLMetadata();

    virtual ~LLMetadata();

    int getRequestSourceInfoType();

protected:
    /**
     * \brief indicates which type of requestSourceInfo  the metadata stores
     * */
    int requestSourceInfoType;
};

} /* namespace vndn */
#endif /* LLMETADATA_H_ */
