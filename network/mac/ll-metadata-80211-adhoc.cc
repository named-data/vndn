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

#include "ll-metadata-80211-adhoc.h"

namespace vndn
{

LLMetadata80211AdHoc::LLMetadata80211AdHoc()
{
    TOS = 0;
    requestSourceInfoType = OVER_ADHOC;
}

LLMetadata80211AdHoc::~LLMetadata80211AdHoc()
{
}

LLMetadata80211AdHoc::LLMetadata80211AdHoc(GeoStorage &gs)
{
    requestSourceInfoType = OVER_ADHOC;
    previousHopInfo = gs;
    TOS = 0;
}


const GeoStorage &LLMetadata80211AdHoc::getPreviousHopInfo()
{
    return previousHopInfo;
}

GeoStorage *LLMetadata80211AdHoc::getPreviousHopInfoAddr()
{
    return &previousHopInfo;
}
void LLMetadata80211AdHoc::setPreviousHopInfo(const GeoStorage &previousHopInfo)
{
    this->previousHopInfo = previousHopInfo;
}

void LLMetadata80211AdHoc::setTos(int value)
{
    if (0 <= value && value <= 100)
        TOS = value;
}
int LLMetadata80211AdHoc::getTos()
{
    return TOS;
}

} /* namespace vndn */
