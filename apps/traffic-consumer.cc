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

#include <errno.h>
#include <time.h>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <syslog.h>

#include "corelib/log.h"
#include "corelib/ptr.h"
#include "helper/event-monitor.h"
#include "ndn-app-socket.h"
#include "ndn-consumer.h"

#include "application-map.h"
#include "traffic-app.h"

using namespace vndn;

NS_LOG_COMPONENT_DEFINE("TrafficConsumer");

//#define TEST_WITH_NO_LOG_SYSTEM
//#define TEST_WITH_NO_GPS

/**
 * periodically create an interest:
 * traffic/geocoordinates.../time
 *
 */

static void usage()
{
    std::cout << "Usage: ./consumer timeout-retransmission (-1 if no retransmission), how many seconds the consumer has to wait for the next request\n";
}

void buildInterestName(std::list<std::string> & name, ApplicationMap map)
{
    name.clear();
    name.push_back("traffic");
#ifdef TEST_WITH_NO_GPS
    name.push_back("1,2");
#else
    name.push_back(map.getRandomCoordinate());
#endif
    // name.append("/");
    time_t nowSecond = time(NULL);
    unsigned int nowMinutesDiscretized = ((unsigned int) nowSecond / 60)/MINUTES_GRANULARITY;
    nowMinutesDiscretized = nowMinutesDiscretized * MINUTES_GRANULARITY;

    int timeIndex = rand()%NUMBER_OF_TIME_INTERVAL;
    //discretized minute in a day: 24 * 60 / interval
    std::stringstream out;
    out << nowMinutesDiscretized - timeIndex*MINUTES_GRANULARITY;
    out << ",";
    out << nowMinutesDiscretized - timeIndex*MINUTES_GRANULARITY + TRAFFIC_REQUEST_TTL;
    name.push_back(out.str());
    name.push_back("speed");
}

void resetTimerNextDeadline(timeval * tt, int retransmissionDeadline) {
    tt->tv_sec = retransmissionDeadline;
    tt->tv_usec = 0;
}

void setTimerNextDeadline(timeval * tt, timeval previousTime, int retransmissionDeadline) {
    timeval tmpTime;
    gettimeofday(&tmpTime,NULL);
    if (tmpTime.tv_usec >= previousTime.tv_usec) {
        tmpTime.tv_usec = tmpTime.tv_usec-previousTime.tv_usec;
    } else {
        tmpTime.tv_usec = 1000000 - previousTime.tv_usec + tmpTime.tv_usec;
        tmpTime.tv_sec--;
    }
    if(tmpTime.tv_sec < previousTime.tv_sec) {
        resetTimerNextDeadline(tt, retransmissionDeadline);
        return;
    } else {
        tmpTime.tv_sec = tmpTime.tv_sec-previousTime.tv_sec;
    }
    if (tmpTime.tv_usec > 0) {
        tt->tv_usec = 1000000 - tmpTime.tv_usec;
        tmpTime.tv_sec++;
    } else {
        tt->tv_usec = 0;
    }
    tt->tv_sec = retransmissionDeadline -  tmpTime.tv_sec;
}


int main(int argc, char **argv)
{
    if (argc < 3) {
        std::cerr << "ERROR, invalid number of arguments" << std::endl;
        usage();
        return -1;
    }

    int retransmissionDeadline = atoi(argv[1]);
    int secondToSleep = atoi(argv[2]);
    if (secondToSleep < 0) {
        std::cerr << "Error, seconds to sleep has a negative value." << std::endl;
        return -1;
    }

    ::openlog("traffic-consumer", LOG_ODELAY | LOG_PID, LOG_LOCAL6);

    NS_LOG_DEBUG("Retransmission deadline = " << retransmissionDeadline);

#ifdef TEST_WITH_NO_LOG_SYSTEM
    int logFile = open("/root/traffic-consumer-log-file", O_WRONLY|O_CREAT,S_IRWXU );
    if (logFile==-1) {
        NS_LOG_ERROR("Failed to open log file.");
        return -1;
    }
#endif

    ApplicationMap map;

    NDNAppSocket socketconnector;
    if (socketconnector.getSocketFD() < 0)
        return 1;

    Ptr<NDNConsumer> consumer = Create<NDNConsumer>(socketconnector.getSocketFD());
    int maxfd = socketconnector.getSocketFD();
    fd_set read_fd;
    FD_ZERO(&read_fd);
    struct timeval tvNextDeadline, tvSelectTime;
    int res;

    char buffer[2000];
    memset(buffer, 0, 2000);
    bool sendAgain = true;

    std::list<std::string> interestComponentName;
    buildInterestName(interestComponentName, map);
    NameComponents interestComponent = NameComponents(interestComponentName);
    int numberOfRetransmission=0;
    std::list<std::string>::iterator i;
    std::string listName;
    for(i=interestComponentName.begin(); i!=interestComponentName.end();i++) {
        listName.append("/");
        listName.append(*i);
    }
    resetTimerNextDeadline(&tvNextDeadline, retransmissionDeadline);
    gettimeofday(&tvSelectTime,NULL);
    while (true) {
        if (sendAgain) {
            res = consumer->SendPacket(interestComponent);
            if (res < 0) {
                NS_LOG_ERROR("SendPacket() failed, exiting.");
                return -1;
            }

            NS_LOG_JSON(log::AppExpressedInterest, "name" << listName);
            NS_LOG_INFO("Interest sent out for '" << listName <<
                        "'; transmission number = " << numberOfRetransmission);
            numberOfRetransmission++;
        }

        FD_SET(socketconnector.getSocketFD(), &read_fd);
        if (retransmissionDeadline == -1) {
            //no retransmission required
            res = select(maxfd + 1, &read_fd, NULL, NULL, NULL);
        } else {
            res = select(maxfd + 1, &read_fd, NULL, NULL, &tvNextDeadline);
        }
        int e = errno;
        sendAgain = true;
        if (res == -1) {
            NS_LOG_ERROR("select() failed: " << strerror(e));
            return -1;
        } else if (res == 0) {
            // timer expired
            NS_LOG_JSON(log::InterestTimedOut, "name" << listName);
            NS_LOG_INFO("Timer expired, no content received, resending interest.");
            gettimeofday(&tvSelectTime, NULL);
            resetTimerNextDeadline(&tvNextDeadline, retransmissionDeadline);
        } else {
            if (FD_ISSET(socketconnector.getSocketFD(), &read_fd)) {
                // content received
                NameComponents contentName;
                res = consumer->read(buffer, contentName);
                if (res == 0) {
                    sendAgain = false;
                    setTimerNextDeadline(&tvNextDeadline, tvSelectTime, retransmissionDeadline);
                    continue;
                } else if (res == -1) {
                    NS_LOG_WARN("Could not read data from ndnd, continuing anyway.");
                    setTimerNextDeadline(&tvNextDeadline, tvSelectTime, retransmissionDeadline);
                    continue;
                }

                for (NameComponents::iterator it = contentName.begin(); it!=contentName.end(); it++){
                    NS_LOG_DEBUG("component received: "<<*it);
                }
                NS_LOG_JSON(log::InterestSatisfied, "name" << listName);
                NS_LOG_INFO("Received content for '" << listName << "' with data: " << buffer <<
                            "; this interest has been transmitted " << numberOfRetransmission << " time(s).");
                std::cout << "Content received: " << buffer << "\n";
                std::cout << "Number of transmission: " << numberOfRetransmission << "\n";
                std::cout << "***********************************\n";

#ifdef TEST_WITH_NO_LOG_SYSTEM
                std::string logString = "\n ***************\nContent received \nNumber of retransmission: ";
                std::stringstream out;
                out << numberOfRetransmission;
                logString.append(out.str());
                logString.append("\nspeed: ");
                res = write(logFile,logString.c_str(), logString.size());
                res = write(logFile,buffer, 2);
#endif

                memset(buffer, 0, 2000);

                //sleep for a while
                sleep(secondToSleep);

                buildInterestName(interestComponentName, map);
                interestComponent = NameComponents(interestComponentName);
                listName="";
                for(i=interestComponentName.begin(); i!=interestComponentName.end();i++) {
                    listName.append("/");
                    listName.append(*i);
                }
                numberOfRetransmission=0;
                gettimeofday(&tvSelectTime,NULL);
                resetTimerNextDeadline(&tvNextDeadline, retransmissionDeadline);
            }
        }
    }

    return 0;
}
