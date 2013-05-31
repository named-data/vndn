/*
 * Copyright (c) 2013 Giulio Grassi <giulio.grassi86@gmail.com>
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

#ifndef ACKINFOBYCOORDINATE_H_
#define ACKINFOBYCOORDINATE_H_

#include "ack-info.h"
#include "utils/geo/coordinate.h"

namespace vndn
{

class AckInfoByCoordinate : public AckInfo
{
public:
    AckInfoByCoordinate(int tos) : AckInfo(tos) {}
    AckInfoByCoordinate(geo::Coordinate c) : previousHop(c) {}
    virtual ~AckInfoByCoordinate() {}

    geo::Coordinate getCoordinate() { return previousHop; }

protected:
    geo::Coordinate previousHop;
};

}

#endif
