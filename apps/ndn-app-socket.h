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

#ifndef NDN_APP_SOCKET_H
#define NDN_APP_SOCKET_H

#include <stdint.h>

namespace vndn
{

class NDNAppSocket
{
public:
    /**
     * \brief Constructor that creates a local socket that can be used to communicate with the NDN daemon
     */
    NDNAppSocket();

    ~NDNAppSocket();

    /**
     * \brief Get the id for the face the localsocket is connected to
     * \return the file descriptor of the localface socket
     * */
    uint32_t getFaceFD() const { return m_face_fd; }

    int getSocketFD() const { return m_sock_fd; }

private:
    int m_sock_fd; ///\brief socket file descriptor
    uint32_t m_face_fd; ///\brief fd of the local face
};

}

#endif
