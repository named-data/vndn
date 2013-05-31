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

#include "application-map.h"
#include "traffic-app.h"
#include "ndn-app-socket.h"
#include "ndn-producer.h"
#include "corelib/log.h"
#include "corelib/ptr.h"
#include "helper/event-monitor.h"
#include "utils/geo/map.h"
#include "utils/geo/location-service.h"
#include "utils/geo/gps-info.h"
#include "utils/gpsd-util.h"

#include <errno.h>
#include <time.h>
#include <sstream>
#include <string>
#include <stdlib.h>
#include <syslog.h>

#include <boost/algorithm/string/join.hpp>

using namespace vndn;

NS_LOG_COMPONENT_DEFINE("TrafficProducer");


int main(int argc, char **argv)
{
    ::openlog("traffic-producer", LOG_ODELAY | LOG_PID, LOG_LOCAL6);

    ApplicationMap map;

    NDNAppSocket socketconnector;
    if (socketconnector.getSocketFD() < 0) {
        return 1;
    }

    GpsdUtil gpsd = GpsdUtil();
    if (gpsd.sendRequestToGpsd()==-1) {
        NS_LOG_ERROR("Failed to send the initial command to gpsd, exiting.");
        return -1;
    }
    int gpsSocket = gpsd.getSocketFd();
    if (gpsSocket==-1) {
        NS_LOG_ERROR("Invalid gpsd socket (-1), exiting.");
        return -1;
    }

    geo::LocationService locationS = geo::LocationService();

    time_t startingTime = time(NULL);
    int startingTimeDiscretized = ((int) startingTime / 60)/MINUTES_GRANULARITY;
    startingTimeDiscretized = startingTimeDiscretized * MINUTES_GRANULARITY;
    int sizeOfSpeedTable = 100;
    int speedTable[sizeOfSpeedTable];
    for (int i=0; i<sizeOfSpeedTable; i++) {
        speedTable[i] = rand()%70;
    }

    Ptr<NDNProducer> producer = Create<NDNProducer>(socketconnector.getSocketFD());
    int maxfd = socketconnector.getSocketFD();
    if (maxfd < gpsSocket) {
        maxfd = gpsSocket;
    }

    NameComponents new_prefix("traffic");
    producer->registerName(new_prefix,socketconnector.getFaceFD());
    
    fd_set read_fd;
    FD_ZERO(&read_fd);
    int res;

    char buffer[2000];
    memset(buffer, 0, 2000);

    while (true) {
        FD_ZERO(&read_fd);
        FD_SET(socketconnector.getSocketFD(), &read_fd);
        FD_SET(gpsSocket, &read_fd);
        res = select(maxfd + 1, &read_fd, NULL, NULL, NULL);
        if (res == -1) {
            NS_LOG_ERROR("select() failed: " << strerror(errno));
            return -1;
        } else {
            if (FD_ISSET(socketconnector.getSocketFD(), &read_fd)) {
                //interest received
                NameComponents interestNameComponent;
                res = producer->read(interestNameComponent);
                if (res == 0) {
                    NS_LOG_WARN("Reading from NDNProducer returned 0, continuing anyway.");
                    continue;
                } else if (res == -1) {
                    NS_LOG_ERROR("Error reading data from ndnd, exiting.");
                    return -1;
                }

                NameComponents::iterator it;
                std::string interestStringComponent[NUMBER_OF_TRAFFIC_INTEREST_COMPONENT];
                it = interestNameComponent.begin();
                bool interestOk = true;
                for (int i=0; i<NUMBER_OF_TRAFFIC_INTEREST_COMPONENT; i++) {
                    if (it!=interestNameComponent.end()) {
                        interestStringComponent[i]=(std::string)*it;
                        it++;
                    } else {
                        interestOk=false;
                        break;
                    }
                }

                if (interestOk) {
                    if (interestStringComponent[0].compare("traffic") != 0) {
                        NS_LOG_DEBUG("Not a traffic interest, discarding packet.");
                    } else {
                        NS_LOG_INFO("Interest received:");
                        NS_LOG_INFO("  type: " << interestStringComponent[0]);
                        NS_LOG_INFO("  position: " << interestStringComponent[1]);
                        NS_LOG_INFO("  time: " << interestStringComponent[2]);
                        NS_LOG_INFO("  request: " << interestStringComponent[3]);
                        const geo::MapElement *myPosition = locationS.getPositionInTheMap();
                        geo::Coordinate myC = locationS.getCoordinate();
                        double distance = map.getDistance(myC, interestStringComponent[1]);
                        if (distance>50){
                            NS_LOG_INFO("I'm not in the right position, I can't reply: my position is " << myPosition->printId()
                                        << " and I received a request for " << interestStringComponent[1]);
                        } else {
                            //check the time
                            std::stringstream ss;
                            size_t pos = interestStringComponent[2].find(",");
                            if (pos == std::string::npos) {
                                NS_LOG_WARN("This interest refers to an invalid time and will be discarded.");
                            } else {
                                std::string requestTime = interestStringComponent[2].substr(0,pos);
                                int offset;
                                if (startingTimeDiscretized<atoi(requestTime.c_str())) {
                                    offset = atoi(requestTime.c_str())-startingTimeDiscretized;
                                } else {
                                    offset = startingTimeDiscretized -atoi(requestTime.c_str());
                                }
                                offset = (offset / MINUTES_GRANULARITY) % sizeOfSpeedTable;
                                ss << speedTable[offset];
                                std::list<std::string> nameReply;
                                for (int i=0; i<NUMBER_OF_TRAFFIC_INTEREST_COMPONENT; i++) {
                                    nameReply.push_back(interestStringComponent[i]);
                                }
                                NS_LOG_INFO("Sending content packet with payload: " << speedTable[offset]);
                                NameComponents contentName = NameComponents(nameReply);
                                NS_LOG_JSON(log::AppRepliedToInterest,
                                            "name" << boost::algorithm::join(nameReply, "/") <<
                                            "data" << speedTable[offset]);
                                res = producer->sendContent(contentName, ss.str());
                                if (res==-1) {
                                    NS_LOG_ERROR("sendContent() failed.");
                                }
                            }
                        }
                    }
                }
                memset(buffer, 0, 2000);
            } else if (FD_ISSET(gpsSocket, &read_fd)) {
                int len = read(gpsSocket, buffer, 2000);
                if (len==-1) {
                    NS_LOG_ERROR("Failed to read from gpsd, exiting.");
                    return -1;
                }
                locationS.updatePosition((uint8_t *)buffer,len);
            }
        }
    }

    return 0;
}
