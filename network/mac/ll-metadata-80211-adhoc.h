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

#ifndef LLMETADATA80211ADHOC_H_
#define LLMETADATA80211ADHOC_H_

#include "ll-metadata.h"
#include "geo-storage.h"

namespace vndn
{

/**
 * \brief It stores information about the previous hop that sent the packet
 *
 * Consists of additional information added to each packet that are sent/received through an 802.11 Ad-Hoc interface
 * This LLMetadata is stored locally, is doesn't go on the network
 * It stores gps coordinates of the previous hop (or of the local node if the packet is generated locally)
 * It stores also the desired TOS for the packet
 * */
class LLMetadata80211AdHoc : public LLMetadata
{
public:

    /**
     * \brief Creates an empty metadata80211
     * */
    LLMetadata80211AdHoc();

    /**
     * \brief Creates a metadata80211 with the specified geoStorage
     * \param gs GeoStorage information about previous hop (or about the local node if the packet is generated locally)
     * */
    LLMetadata80211AdHoc(GeoStorage &gs);

    virtual ~LLMetadata80211AdHoc();


    /**
     * \brief Get the geoStorage information about the previous hop
     * \return the pointer of the GeoStorage that stores the information about previous hop position
     * */
    const GeoStorage &getPreviousHopInfo();

    /**
    * \brief Get the geoStorage information about the previous hop
    * \return information about previous hop position
    * */
    GeoStorage *getPreviousHopInfoAddr();

    /**
     * \brief Set the geoStorage information of previous hop
     * \param previousHopInfo GeoStorage relative to previous hop
     * */
    void setPreviousHopInfo(const GeoStorage &previousHopInfo);

    /**
     * \brief Set the value of TOS
     * \param value desired TOS value
     * */
    void setTos(int value);

    /**
     * \brief gives the TOS value
     * \return value of the TOS
     * */
    int getTos();


protected:
    /**
     * It stores all the information that the NDNDeviceAdapter got about the previous hop (latitude, longitude)
     * If the pkt is generated locally (the previous hop doesn't exist since the source is local) the previousHopInfo content is undefined
     * */
    GeoStorage previousHopInfo;

    /**
     * Index (0-100) of desired reliability (100: the pkt has to go in every available path, 0 only one path is necessary)
     * */
    int TOS;
};

} /* namespace vndn */

#endif /* LLMETADATA80211ADHOC_H_ */
