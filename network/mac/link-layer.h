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

#ifndef LINKLAYER_H_
#define LINKLAYER_H_

#include "ll-header.h"

/**
 * Max size of a packet that goes on the network
 * It should be MAXLLSIZE + the header size that the NDN-link layer will add to the packet
 * */
#define MAXNETWORKPKTSIZE 1500

/** Specifies the max size of a packet that can be managed by the link layer (it's less than the max size supported by the network interface because the NDN-link layer has to add its header */
static const int MAXLLSIZE = MAXNETWORKPKTSIZE - sizeof(struct vndn::llHeader);

#endif /* LINKLAYER_H_ */
