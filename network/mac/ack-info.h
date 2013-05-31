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

#ifndef ACKINFO_H_
#define ACKINFO_H_

#include "geo-storage.h"

namespace vndn
{

class AckInfo
{
public:
    AckInfo();
    AckInfo(int tos);

    virtual ~AckInfo();

    /**
     * \brief get the number of ack received
     * */
    int getNumberOfAck();

    /**
     * \brief get the number of ack that has to be received to consider the packet acked
     * */
    int getTos();

protected:

    /**
     * Number of ack received for this packet
     * */
    int receivedAckNum;

    /**
     * TOS; index of number of ack that has to be received (0-100): 100 = all possible ack.
     * */
    int tos;


    /**
     *  position of local node when it sent the packet
     * */
    //GeoStorage localNode;
};

} /* namespace vndn */
#endif /* ACKINFO_H_ */
