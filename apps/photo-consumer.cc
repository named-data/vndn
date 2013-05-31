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

#include "photo-app.h"
#include "application-map.h"
#include "corelib/log.h"
#include "corelib/ptr.h"
#include "helper/event-monitor.h"
#include "ndn-app-socket.h"
#include "ndn-consumer.h"
#include "utils/gpsd-util.h"

#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <list>
#include <netinet/in.h>
#include <syslog.h>
#include <unistd.h>

using namespace vndn;

NS_LOG_COMPONENT_DEFINE("PhotoConsumer");


void buildInterestName(std::list<std::string> & name, ApplicationMap map, int carID, int segmentNumber, time_t nowSecond, std::string positionInMap)
{
    name.clear();
    //type of service
    name.push_back(PHOTO_TYPE_OF_SERVICE);
    //position
    name.push_back(positionInMap);
    //name.push_back(map.getByIndex(0)->printId());
    //car id
    std::stringstream out;
    out << carID;
    name.push_back(out.str());
    //timestamp
    out.str("");
    NS_LOG_DEBUG("now second"<< nowSecond );
    unsigned int nowMinutesDiscretized = ((unsigned int) nowSecond / 60)/PHOTO_MINUTES_GRANULARITY;
    nowMinutesDiscretized = nowMinutesDiscretized * PHOTO_MINUTES_GRANULARITY;
    NS_LOG_DEBUG("now minutes disc" << nowMinutesDiscretized);
    out << nowMinutesDiscretized;
    name.push_back(out.str());
    //chunk number
    out.str("");
    out << segmentNumber;
    name.push_back(out.str());
}

std::string buildInterestName(std::list<std::string> & name, ApplicationMap map, int carID, int segmentNumber, time_t nowSecond)
{
    std::string positionInMap = map.getRandomCoordinate();
    buildInterestName(name, map, carID, segmentNumber, nowSecond, positionInMap);
    return positionInMap;
}

int openPhotoFile(int photoCounter)
{
    std::string pathFile = "/tmp/traffic-photo-";
    pathFile.append(g_log.Hostname());
    pathFile.append("-");
    std::stringstream out;
    out << photoCounter;
    pathFile.append(out.str());
    pathFile.append(".jpeg");
    int photoFd = ::creat(pathFile.c_str(), S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    return photoFd;
}

void getFileName(int photoCounter, std::string &fileName)
{
    fileName = "traffic-photo-";
    fileName.append(g_log.Hostname());
    fileName.append("-");
    std::stringstream out;
    out << photoCounter;
    fileName.append(out.str());
    fileName.append(".jpeg");
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
    tt->tv_sec = retransmissionDeadline - tmpTime.tv_sec;
}


int main(int argc, char **argv)
{
    if (argc < 3) {
        std::cerr << "Error, invalid number of arguments: how many seconds it waits to retransmit the same interest if no reply is received (-1 if no retransmission is required), how many seconds between one photo request and the next one." << std::endl;
        return -1;
    }

    int retransmissionDeadline = atoi(argv[1]);
    int secondToSleep = atoi(argv[2]);
    if (secondToSleep < 0) {
        std::cerr << "Error, seconds to sleep has a negative value." << std::endl;
        return -1;
    }

    ::openlog("photo-consumer", LOG_ODELAY | LOG_PID, LOG_LOCAL6);

    NS_LOG_DEBUG("Retransmission deadline = " << retransmissionDeadline);

    int receivedPhotoCounter=0;

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
    uint32_t chunkNumber = 0;
    uint32_t requiredChunk = -1;
    uint32_t carId = 0;
    std::string listName;
    NameComponents interestNameCmp, contentNameCmp;

    while (true) {
        int photoFd = openPhotoFile(receivedPhotoCounter);
        if(photoFd==-1) {
            NS_LOG_ERROR("Failed to open the file used to store the photo, exiting.");
            return -1;
        }

        receivedPhotoCounter++;
        chunkNumber=0;
        requiredChunk=-1;
        carId=0;
        time_t nowSecond = time(NULL);
        sendAgain = true;
        std::string requestedPosition = buildInterestName(interestComponentName, map, carId, chunkNumber, nowSecond);
        std::list<std::string>::iterator i;
        listName="";
        for(i=interestComponentName.begin(); i!=interestComponentName.end();i++) {
            listName.append("/");
            listName.append(*i);
        }
        resetTimerNextDeadline(&tvNextDeadline, retransmissionDeadline);
        gettimeofday(&tvSelectTime,NULL);

        while (requiredChunk != chunkNumber) {
            memset(buffer, 0, 2000);

            if (sendAgain) {
                interestNameCmp = NameComponents(interestComponentName);
                res = consumer->SendPacket(interestNameCmp);
                if (res < 0) {
                    NS_LOG_ERROR("SendPacket() failed, exiting.");
                    return -1;
                }

                NS_LOG_JSON(log::AppExpressedInterest, "name" << listName);
                NS_LOG_INFO("Interest sent out for '" << listName << "'");
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
                    res = consumer->read(buffer, contentNameCmp);
                    if (res == 0) {
                        sendAgain = false;
                        setTimerNextDeadline(&tvNextDeadline, tvSelectTime, retransmissionDeadline);
                        NS_LOG_WARN("Read from producer returned 0.");
                        continue;
                    } else if (res == -1) {
                        NS_LOG_WARN("Could not read data from ndnd, continuing anyway.");
                        setTimerNextDeadline(&tvNextDeadline, tvSelectTime, retransmissionDeadline);
                        continue;
                    }

                    NS_LOG_JSON(log::InterestSatisfied, "name" << listName);
                    NS_LOG_INFO("Received content for chunk #" << chunkNumber << " (size=" << res << ").");

                    // TODO store it
                    char *data = buffer;
                    int dataSize = res;
                    if (chunkNumber==0) {
                        //first content received
                        PhotoHeader *photoHdr = (PhotoHeader *) buffer;
                        requiredChunk = ntohl(photoHdr->numberOfContents);
                        carId = ntohl(photoHdr->id);
                        NS_LOG_INFO("Photo from car " << carId<< ", number of chunk: " << requiredChunk);
                        data = &(buffer[sizeof(PhotoHeader)]);
                        dataSize -= sizeof(PhotoHeader);
                    }
                    chunkNumber++;
                    res = write(photoFd, data, dataSize);
                    sendAgain=true;
                    buildInterestName(interestComponentName, map, carId, chunkNumber, nowSecond, requestedPosition);
                    std::list<std::string>::iterator i;
                    listName="";
                    for(i=interestComponentName.begin(); i!=interestComponentName.end();i++) {
                        listName.append(*i);
                        listName.append("/");
                    }
                    gettimeofday(&tvSelectTime,NULL);
                    resetTimerNextDeadline(&tvNextDeadline, retransmissionDeadline);
                }
            }
        }
        close(photoFd);

        std::string fileName;
        getFileName(receivedPhotoCounter - 1, fileName);
        NS_LOG_JSON(log::PhotoReceived,
                    "fileName" << fileName <<
                    "photoProducer" << carId);
        NS_LOG_INFO("Photo stored (from car id: " << carId << ").");

        sleep(secondToSleep);
    }

    return 0;
}
