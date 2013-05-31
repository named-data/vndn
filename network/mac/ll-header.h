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

#ifndef LL_HEADER_H_
#define LL_HEADER_H_

#include <string>

/** Defines the number of character used to define a coordinate (latitude or longitude) */
#define GPS_STRING_SIZE 12  //+ or - , 3 for integer part, . , 6 decimals: -123.456789

/** Defines the lat and longitude values that has to be stored when no valid gps data is available */
#define DEFAULT_COORDINATE_CHAR "999.999"
#define DEFAULT_COORDINATE_DOUBLE 999.999

namespace vndn {

/**
 * \brief Defines the "ndn link layer header".
 *
 * It encapsulates the ndn packet and is encapsulated by 802.11 header.
 * The header carries the following information: gps coordinates (in decimal
 * degree format), tos (type of service), heading of the node.
 */
struct llHeader {
    /**
     * Latitude of the node
     * */
    char lat[GPS_STRING_SIZE];

    /**
     * Longitude of the node
     * */
    char longitude[GPS_STRING_SIZE];

    /**
     * index (0-100) of desired reliability (100: the pkt has to go in every available path, 0 only one path is necessary)
     * */
    int tos;

    //TODO direction

} __attribute__((packed));

}

#endif // LL_HEADER_H_
