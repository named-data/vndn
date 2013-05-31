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

#ifndef GPSDPARSER_H_
#define GPSDPARSER_H_

#include "gps-info.h"

namespace vndn {
namespace geo {

/**
 * \brief Parses the gpsd output and stores the location information into a gpsInfo object
 *
 * The gpsd command necessary to correctly parsed the data is the following: ?WATCH={\"enable\":true,\"json\":true}"
 */
class GpsdParser
{
public:
    /**
     * \brief Parses the gpsd output and stores the location information into a gpsInfo object
     *
     * \param data pointer to the buffer where the gpsd output is stored
     * \param len size of the gpsd output
     * \param info pointer to the gpsInfo object that will store all the useful information extracted by gpsd: lat, long, altitude (not implemented), speed (not implemented), heading(not implemented);
     */
    int parseData(char *data, int len, GpsInfo *info);
};

} /* namespace geo */
} /* namespace vndn */

#endif /* GPSDPARSER_H_ */
