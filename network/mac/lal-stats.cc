/*
 * Copyright (c) 2012-2013 Giulio Grassi <giulio.grassi86@gmail.com>
 *                         Davide Pesavento <davidepesa@gmail.com>
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

#include "lal-stats.h"
#include "corelib/log.h"

NS_LOG_COMPONENT_DEFINE("LALStatistic");


namespace vndn
{

LALStatistic::LALStatistic()
{
    resetStats();
}

void LALStatistic::sendToLog()
{
    NS_LOG_JSON(log::LinkLayerStats,
                "stats"                 << log::JsonMapOpen <<
                "interestPktsSent"      << interestSent <<
                "contentPktsSent"       << contentSent <<
                "totalPktsSent"         << pktSent <<
                "interestSendRequests"  << sendingOutInterestRequest <<
                "contentSendRequests"   << sendingOutContentRequest <<
                "totalSendRequests"     << sendingOutRequest <<
                "interestPktsRecv"      << receivedInterest <<
                "contentPktsRecv"       << receivedContent <<
                "totalPktsRecv"         << receivedPacket <<
                "interestPktsAccepted"  << receivedInterestGoingUp <<
                "contentPktsAccepted"   << receivedContentGoingUp <<
                "totalPktsAccepted"     << receivedPacketGoingUp <<
                "ackCount"              << ackedPacket <<
                "retxCount"             << numberOfRetransmission <<
                "satisfiedInterests"    << interestAckedByContent <<
                log::JsonMapClose);

    resetStats();
}

void LALStatistic::resetStats()
{
    pktSent = 0;
    interestSent = 0;
    contentSent = 0;
    sendingOutRequest = 0;
    sendingOutContentRequest = 0;
    sendingOutInterestRequest = 0;
    receivedPacket = 0;
    receivedContent = 0;
    receivedInterest = 0;
    receivedPacketGoingUp = 0;
    receivedContentGoingUp = 0;
    receivedInterestGoingUp = 0;
    ackedPacket = 0;
    numberOfRetransmission = 0;
    interestAckedByContent = 0;
}

}
