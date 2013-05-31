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

#include "gpsd-util.h"
#include "corelib/log.h"

#include <arpa/inet.h>
#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#define GPSDCOMMAND "?WATCH={\"enable\":true,\"json\":true}"  //?WATCH={"enable":true,"json":true}

NS_LOG_COMPONENT_DEFINE("GpsdUtil");


GpsdUtil::GpsdUtil(int port)
{
    NS_LOG_FUNCTION_NOARGS();

    struct sockaddr_in Local, Serv;
    char data[10000];
    int OptVal;
    socketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFd == -1) {
        throw;
    }
    int res = setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, (char *)&OptVal, sizeof(OptVal));
    if (res == -1) {
        throw;
    }
    memset(&Local, 0, sizeof(Local));
    Local.sin_family = AF_INET;
    Local.sin_addr.s_addr = htonl(INADDR_ANY);
    Local.sin_port = htons(0);
    res = bind(socketFd, (struct sockaddr *) &Local, sizeof(Local));
    if (res == -1)  {
        throw;
    }
    memset(&Serv, 0, sizeof(Serv));
    Serv.sin_family = AF_INET;
    Serv.sin_addr.s_addr = inet_addr("127.0.0.1");
    Serv.sin_port = htons(port);
    /* connection request */
    res = connect(socketFd, (struct sockaddr *) &Serv, sizeof(Serv));
    if (res == -1)  {
        NS_LOG_ERROR("Connection to gpsd failed: " << strerror(errno));
        exit(1);
    }

    NS_LOG_INFO("Connection to gpsd established");

    //read first line from gpsd (there is no useful data in this line)
    res = read(socketFd, data, 1000);
    if (res == -1) {
        NS_LOG_ERROR("First read on gpsd socket failed: " << strerror(errno));
        exit(1);
    }
}

GpsdUtil::~GpsdUtil()
{
    if (socketFd != -1) {
        close(socketFd);
        socketFd = -1;
    }
}

int GpsdUtil::sendRequestToGpsd()
{
    if (socketFd == -1) {
        return -1;
    }

    uint8_t buffer[1024];
    memcpy(buffer, GPSDCOMMAND, strlen(GPSDCOMMAND));
    int len = send(socketFd, buffer, strlen(GPSDCOMMAND), 0);
    if (len == -1) {
        return -1;
    }

    return len;
}
