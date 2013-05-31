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

#include "ndn-app-socket.h"
#include "corelib/log.h"

#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define NDND_SOCKET_PATH "/var/run/ndnd.sock"

NS_LOG_COMPONENT_DEFINE("NDNAppSocket");


namespace vndn
{

NDNAppSocket::NDNAppSocket()
{
    if ((m_sock_fd = ::socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
        NS_LOG_ERROR("Could not create socket: " << strerror(errno));
        exit(1);
    }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, NDND_SOCKET_PATH);

    if (::connect(m_sock_fd, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
        NS_LOG_ERROR("Failed to connect to daemon: " << strerror(errno));
        exit(1);
    }

    uint32_t buffer;
    if (::recv(m_sock_fd, &buffer,sizeof(uint32_t), 0) < 0) {
        NS_LOG_ERROR("Failed to receive FaceID: " << strerror(errno));
        exit(1);
    }

    m_face_fd = buffer;

    NS_LOG_DEBUG("Connected to NDN daemon via fd=" << m_sock_fd);
}

NDNAppSocket::~NDNAppSocket()
{
    ::close(m_sock_fd);
}

}
