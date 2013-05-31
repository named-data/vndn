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

#include "map-element.h"
#include "corelib/log.h"

NS_LOG_COMPONENT_DEFINE("geo.MapElement");

namespace vndn {
namespace geo {

MapElement::MapElement()
{
    hash =0;
    stringId = printId();
}

MapElement::~MapElement()
{
}

bool MapElement::hasOneIntersectionInCommon(MapElement el) const
{
    NS_LOG_WARN("hasOneIntersectionInCommon should be called on a Junction or on a Link, not on a MapElement");
    return false;
}

std::string MapElement::printId() const
{
    NS_LOG_DEBUG("id from mapelement: hashcode: " <<getHashCode());
    std::stringstream ss;
    ss << getHashCode();
    /*firstIntersection.getId();
        if (isASegment()) {
        ss << ",";
        ss << secondIntersection.getId();
        }*/
    return ss.str();
}

}
}
