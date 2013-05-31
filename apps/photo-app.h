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

#ifndef PHOTO_APP_H_
#define PHOTO_APP_H_

#include <stdint.h>


#define NUMBER_OF_PHOTO_INTEREST_COMPONENT 5
/** /'car-photo'/map-segment/car-id(0 for the first interest)/timestamp/sequence-number(0 for the first interest)   */

#define MAX_PHOTO_CONTENT_SIZE 1300
#define PHOTO_TYPE_OF_SERVICE "photo-traffic"

#define PHOTO_MINUTES_GRANULARITY 1


struct PhotoHeader {
    uint32_t id;
    uint32_t numberOfContents;
    uint32_t size;
}__attribute__((packed));


/**
 * INTEREST
 * /'car-photo'/map-segment/timestamp/sequence-content-number
 * Example:
 * /car-photo/1,2/1234567/3
 * /car-photo/1/1234567/5
 *
 * if we want to have multiple producer:
 * first interest:
 * /'car-photo'/map-segment/timestamp/sequence-content-number
 * reply:
 * number of content, car id, part of the photo
 * next interest:
 * /'car-photo'/map-segment/timestamp/car-id/sequence-content-number
 *
 * but we have to check which type of interest we receive
 *  1 more component before car-id
 *  car-id == 0 for the first interest   ------BEST SOLUTION ----
 *  string attached to sequence and or car-id
 * */

#endif /* PHOTO_APP_H_ */
