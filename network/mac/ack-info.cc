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

#include "ack-info.h"

namespace vndn
{

AckInfo::AckInfo()
{
    receivedAckNum = 0;
}
    
AckInfo::AckInfo(int tosP)
{
    receivedAckNum = 0;
    tos = tosP;
}

AckInfo::~AckInfo()
{
    // TODO Auto-generated destructor stub
}

int AckInfo::getNumberOfAck()
{
    return receivedAckNum;
}


int AckInfo::getTos()
{
    return tos;
}

} /* namespace vndn */
