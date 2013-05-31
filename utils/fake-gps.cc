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

#include <cstdio>
#include <cstdlib>
#include <errno.h>
#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

/**
 * FAKE GPS
 *
 * It can be used if you don't have a gps sensor. It periodically sends fake location updates thus simulating gpsd.
 * This version of fakeGps always sends the same position.
 *
 * How to use fakeGps instead of gpsd:
 *
 * Daemon:
 *  - network/mac/ndn-device-adapter.h: GPSPORT: same port used by the fakeGps process that will be connected to the daemon
 *  - utils/geo/gpsd-parser.h: #define FAKE: it tells the parser that it's getting data from fakeGps instead of gpsd (the data formats are different)
 *
 * If you are testing the code indoor and you want to "emulate" the distance between the nodes without really moving the nodes,
 * you can force the daemon to drop packets coming from a position "too far" from the node itself:
 *  - ll-nom-policy.h: enable #define TEST_GPS_MOVING
 *  - inside LLNomPolicy::isReachable(): set max distance (if the distance from the source of the node is greater than that value, the packet will be discarded)
 *
 * Applications:
 *  - utils/gpsd-util.h: in GpsdUtil constructor, use the same port number used by the fakeGps process that will be connected to the traffic producer
 *  - photoProducer: pass the fakeGps port as the second argument
 *  - photoProducer: by default the producer replies to an interest concerning an area A only if the producer itelf is in that area;
 *    if you want to disable this check, just disable the position check used to decide whether it can or cannot reply to an interest
 */


static const int TRACESIZE = 2;
static const int TRACENUMBER = 3;

struct Trace {
    double lat[TRACESIZE];
    double longitude[TRACESIZE];
    std::string name[TRACESIZE];
};

struct Message {
    double lat;
    double longitude;
};


int createGPSSocketServer(int port)
{
    int socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketfd == -1) {
        return -1;
    }

    int opt;
    int res = setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt));
    if (res == -1)  {
        std::cerr << "ERROR: setsockopt() failed: " << strerror(errno) << std::endl;
        return -1;
    }

    struct sockaddr_in local;
    memset(&local, 0, sizeof(local));
    local.sin_family = AF_INET;
    local.sin_addr.s_addr = inet_addr("127.0.0.1");
    local.sin_port = htons(port);
    res = bind(socketfd, (struct sockaddr *) &local, sizeof(local));
    if (res == -1)  {
        return -1;
    }

    listen(socketfd, 1);
    std::cout << "Listening ..." << std::endl;

    struct sockaddr_in adr_clnt;
    socklen_t len_inet;
    res = accept(socketfd, (struct sockaddr *)&adr_clnt, &len_inet);
    if (res == -1)  {
        std::cerr << "ERROR: accept() failed: " << strerror(errno) << std::endl;
        return -1;
    }

    return res;
}


int main(int argc, char *argv[])
{
    if (argc < 3) {
        std::cerr << "Usage: fakeGps <trace-index> <port>" << std::endl;
        return -1;
    }

    int mode = atoi(argv[1]);
    if (mode < 0 || mode > TRACENUMBER) {
        std::cerr << "ERROR: The trace index must be between 1 and " << TRACENUMBER << std::endl;
        return -1;
    }

    int port = atoi(argv[2]);
    int sock = createGPSSocketServer(port);
    if (sock == -1) {
        std::cerr << "ERROR: Failed to create fake gps socket" << std::endl;
        return -1;
    }

    Trace traces[TRACENUMBER];

    traces[0].lat[0] = 34.030007;
    traces[0].longitude[0] = -118.485188;
    traces[0].lat[1] = traces[0].lat[0];
    traces[0].longitude[1] = traces[0].longitude[0];
    traces[0].name[1] = traces[0].name[0];

    traces[1].lat[0] = 34.036302;
    traces[1].longitude[0] = -118.477249;
    traces[1].lat[1] = traces[1].lat[0];
    traces[1].longitude[1] = traces[1].longitude[0];
    
    traces[2].lat[0] = 34.043983;
    traces[2].longitude[0] = -118.467293;
    traces[2].lat[1] = traces[2].lat[0];
    traces[2].longitude[1] = traces[2].longitude[0];

    Message msg;
    int counter = 0;
    int index = 0;
    while (true) {
        counter++;
        if (counter == 10) {
            counter = 0;
            index = (index + 1) % TRACESIZE;
            std::cout << "sending: " << msg.lat << "," << msg.longitude << std::endl;
        }
        msg.lat = traces[mode].lat[index];
        msg.longitude = traces[mode].longitude[index];
        if (send(sock, &msg, sizeof(struct Trace), 0) == -1) {
            std::cerr << "ERROR: send() failed: " << strerror(errno) << std::endl;
            close(sock);
            return -1;
        }
        sleep(1);
    }
}
