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
#include "ndn-producer.h"
#include "utils/geo/map.h"
#include "utils/geo/location-service.h"
#include "utils/geo/gps-info.h"
#include "utils/gpsd-util.h"

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <list>
#include <unistd.h>
#include <netinet/in.h>
#include <syslog.h>

#include <boost/algorithm/string/join.hpp>

using namespace vndn;

NS_LOG_COMPONENT_DEFINE("PhotoProducer");

//fswebcam  output.jpeg --skip 20 --loop 10 --timestamp "%Y%m%d %H:%M" ~/%Y%m%d_%H%M%S.jpg
//fswebcam  output.jpeg --skip 20 --loop 10   -> next image will overwrite the previous one
//fswebcam  photo.jpeg --skip 10


class Content
{
public:
    Content() { memset(data,0,1400); }
    virtual ~Content() {}

    uint8_t data[1400];
    unsigned int size;
};

class Photo
{
public:
    Photo() : contentSize(0) {}
    virtual ~Photo() {}

    std::string id; //timestamp,location
    uint32_t numberOfPacket; //indicates how many content are necessary to store the entire photo
    uint32_t contentSize;
    Content * photoContent;
};


int main(int argc, char **argv)
{
    bool first = true;
    if (argc < 4) {
        std::cerr << "Invalid number of arguments. Specify: car id, gps port, webcam device path" << std::endl;
        return -1;
    }

    ::openlog("photo-producer", LOG_ODELAY | LOG_PID, LOG_LOCAL6);

    std::string carID = std::string(argv[1]);

    NDNAppSocket socketconnector;
    if (socketconnector.getSocketFD() < 0) {
        return 1;
    }

    GpsdUtil gpsd = GpsdUtil(atoi(argv[2]));
    if (gpsd.sendRequestToGpsd()==-1) {
        NS_LOG_ERROR("Failed to send the initial command to gpsd, exiting.");
        return -1;
    }
    int gpsSocket = gpsd.getSocketFd();
    if (gpsSocket==-1) {
        NS_LOG_ERROR("Invalid gpsd socket (-1), exiting.");
        return -1;
    }

    Ptr<NDNProducer> producer = Create<NDNProducer>(socketconnector.getSocketFD());
    int maxfd = socketconnector.getSocketFD();
    if (maxfd < gpsSocket) {
        maxfd = gpsSocket;
    }
    fd_set read_fd;
    FD_ZERO(&read_fd);
    int res;

    char buffer[2000];
    memset(buffer, 0, 2000);
    geo::LocationService locationS = geo::LocationService();
    std::string webcamDevice = std::string(argv[3]);
    std::string deviceParameter = "-d";
    deviceParameter.append(webcamDevice);
    char devicePath[deviceParameter.size()];
    memcpy(devicePath, deviceParameter.c_str(), deviceParameter.size());
    char pathFile[] = "/tmp/photo.jpeg";
    char command[] = "/usr/bin/fswebcam";
    char skipFrame[] = "--skip 10";
    std::list<Photo> photoStorage;
    Content *contentToSend = 0;
    NameComponents interestNameCmp, contentNameCmp;

    ApplicationMap map;

    NameComponents new_prefix("photo");
    producer->registerName(new_prefix,socketconnector.getFaceFD());
    
    while (true) {
        FD_ZERO(&read_fd);
        FD_SET(socketconnector.getSocketFD(), &read_fd);
        FD_SET(gpsSocket, &read_fd);
        memset(buffer, 0, 2000);
        res = select(maxfd + 1, &read_fd, NULL, NULL, NULL);
        if (res == -1) {
            NS_LOG_ERROR("select() failed: " << strerror(errno));
            return -1;
        } else {
            if (FD_ISSET(socketconnector.getSocketFD(), &read_fd)) {
                //interest received
                res = producer->read(interestNameCmp);
                if (res == 0) {
                    NS_LOG_WARN("Reading from NDNProducer returned 0, continuing anyway.");
                    continue;
                } else if (res == -1 ) {
                    NS_LOG_ERROR("Error reading data from ndnd, exiting.");
                    return -1;
                }

                //checking if it's a photo interest
                NameComponents::iterator it;
                std::string interestStringComponent[NUMBER_OF_PHOTO_INTEREST_COMPONENT];
                it = interestNameCmp.begin();
                bool interestOk = true;
                for (int i=0; i<NUMBER_OF_PHOTO_INTEREST_COMPONENT; i++) {
                    if (it!=interestNameCmp.end()) {
                        interestStringComponent[i]=*it;
                        it++;
                    } else {
                        interestOk=false;
                        NS_LOG_INFO("Packet discarded, wrong number of components.");
                        break;
                    }
                }

                if (interestOk) {
                    if (interestStringComponent[0].compare(PHOTO_TYPE_OF_SERVICE) != 0) {
                        NS_LOG_DEBUG("Not a photo interest, discarding packet.");
                    } else {
                        NS_LOG_INFO("Interest received:");
                        NS_LOG_INFO("  type: " << interestStringComponent[0]);
                        NS_LOG_INFO("  position: " << interestStringComponent[1]);
                        NS_LOG_INFO("  time: " << interestStringComponent[3]);
                        NS_LOG_INFO("  car id: " << interestStringComponent[2]);
                        NS_LOG_INFO("  sequence number: " << interestStringComponent[4]);

                        if ((interestStringComponent[2].compare(carID)!=0) &&
                                (interestStringComponent[2].compare("0")!=0)) {
                            //interest for another car
                            NS_LOG_INFO("This interest is for another car.");
                            continue;
                        }
                        unsigned int segmentNumber = atoi(interestStringComponent[4].c_str());
                        //checking is we already processed the photo for this interest
                        std::string interestId = interestStringComponent[3];
                        interestId.append("||");
                        interestId.append(interestStringComponent[1]);
                        std::list<Photo>::iterator it;
                        bool photoAlreadyProcessed =false;
                        for (it=photoStorage.begin(); it!=photoStorage.end(); it++) {
                            if( (*it).id.compare(interestId)==0) {
                                //element found
                                photoAlreadyProcessed = true;
                                break;
                            }
                        }
                        if (photoAlreadyProcessed) {
                            Photo *processedPhoto = &(*it);
                            if (segmentNumber > processedPhoto->numberOfPacket) {
                                NS_LOG_WARN("Wrong sequence number: " << segmentNumber);
                                continue;
                            }
                            contentToSend = &(processedPhoto->photoContent[segmentNumber]);
                        } else {
                            const geo::MapElement *myPosition = locationS.getPositionInTheMap();
                            geo::Coordinate myC = locationS.getCoordinate();
                            double distance = map.getDistance(myC, interestStringComponent[1]);
                            if (distance>50){
                                NS_LOG_INFO("I'm not in the right position, I can't reply: my position is " << myPosition->printId()
                                            << " and I received a request for " << interestStringComponent[1]);
                                continue;
                            } else {
                                //I can reply, but I don't have yet the photo. I need to make it now
                                pid_t pid = fork();
                                if (pid == -1) {
                                    //error: discard the pkt and continue
                                    NS_LOG_ERROR("Could not take the photo, fork() failed: " << strerror(errno));
                                    continue;
                                } else if (pid == 0) {
                                    // child
                                    char * const argv[] = {command, devicePath, pathFile, NULL, skipFrame, NULL};
                                    if (execv(command, argv)==-1) {
                                        NS_LOG_ERROR("Could not take the photo, execv() failed: " << strerror(errno));
                                    }
                                    exit(-1);
                                } else {
                                    int status;
                                    //int res = waitpid(pid, &status, WUNTRACED | WCONTINUED); //waiting for the son to get the photo
                                    pid_t res = wait(&status);
                                    if (res==-1) {
                                        NS_LOG_ERROR("Command to get the photo failed, discarding the packet.");
                                        continue;
                                    }
                                    NS_LOG_DEBUG("Photo taken.");
                                    int photoFd = open(pathFile, O_RDONLY);
                                    if (photoFd==-1) {
                                        NS_LOG_ERROR("Failed to open the photo file, discarding the packet.");
                                        continue;
                                    }

                                    //storing the photo in content storage
                                    std::list<Content> contentList;
                                    int photoSize=0;
                                    int contentNumber=0;
                                    bool photoEnd=false;
                                    while(!photoEnd) {
                                        Content c;
                                        if(contentNumber==0) {
                                            res = read(photoFd, buffer, MAX_PHOTO_CONTENT_SIZE - sizeof(PhotoHeader));
                                        } else {
                                            res = read(photoFd, buffer, MAX_PHOTO_CONTENT_SIZE);
                                        }
                                        if (res==0) {
                                            //end of file
                                            photoEnd = true;
                                        } else if (res ==-1) {
                                            NS_LOG_ERROR("Error reading photo file, discarding the photo.");
                                            continue;
                                        } else {
                                            if(contentNumber==0) {
                                                memcpy(&(c.data[sizeof(PhotoHeader)]), buffer, res);
                                                res+=sizeof(PhotoHeader);
                                            } else {
                                                memcpy(c.data, buffer, res);
                                            }
                                            contentNumber++;
                                            c.size=res;
                                            photoSize+=res;
                                            NS_LOG_DEBUG("partial size: " << res);
                                            contentList.push_back(c);
                                        }
                                    }
                                    NS_LOG_DEBUG("photo size: " << photoSize);
                                    Photo * newPhoto = new Photo();
                                    newPhoto->id = interestId;
                                    newPhoto->numberOfPacket = contentNumber;
                                    newPhoto->photoContent = new Content[contentNumber];
                                    NS_LOG_DEBUG("number of chunks: "<<newPhoto->numberOfPacket);
                                    std::list<Content>::iterator it;
                                    int index=0;
                                    for (it=contentList.begin(); it!=contentList.end(); it++) {
                                        newPhoto->photoContent[index]=*it;
                                        newPhoto->contentSize+=(*it).size;
                                        index++;
                                    }
                                    newPhoto->contentSize-=sizeof(PhotoHeader);
                                    if (atoi(interestStringComponent[4].c_str())==0) { //it shouldn't be happen otherwise
                                        PhotoHeader hdr;
                                        hdr.id = htonl(atoi(carID.c_str()));
                                        hdr.numberOfContents =htonl(newPhoto->numberOfPacket);
                                        NS_LOG_DEBUG("number of content in hdr "<<ntohl(hdr.numberOfContents));
                                        hdr.size=htonl(newPhoto->contentSize);
                                        memcpy(newPhoto->photoContent[0].data, &hdr, sizeof(PhotoHeader));
                                    }
                                    photoStorage.push_back(*newPhoto);
                                    if (segmentNumber > newPhoto->numberOfPacket) {
                                        NS_LOG_WARN("Wrong segment #" << segmentNumber << " was requested.");
                                        continue;
                                    }
                                    contentToSend=&(newPhoto->photoContent[segmentNumber]);
                                }
                            }
                        }

                        std::list<std::string> nameReply;
                        for (int i=0; i<NUMBER_OF_PHOTO_INTEREST_COMPONENT; i++) {
                            nameReply.push_back(interestStringComponent[i]);
                        }

                        //writing the content
                        if (!contentToSend) {
                            NS_LOG_ERROR("Error, contentToSend is null.");
                            continue;
                        }

                        NS_LOG_JSON(log::AppRepliedToInterest,
                                    "name" << boost::algorithm::join(interestNameCmp, "/"));

                        if (first) {
                            PhotoHeader *photoHdr = (PhotoHeader *) contentToSend->data;
                            uint32_t requiredChunk = ntohl(photoHdr->numberOfContents);
                            uint32_t carId = ntohl(photoHdr->id);
                            NS_LOG_INFO("Sending photo header: car = " << carId << "; number of chunks = " << requiredChunk);
                            first = false;
                        }
                        NS_LOG_INFO("Sending photo content:");
                        NS_LOG_INFO("  position: " << interestStringComponent[1]);
                        NS_LOG_INFO("  time: " << interestStringComponent[3]);
                        NS_LOG_INFO("  car id: " << interestStringComponent[2]);
                        NS_LOG_INFO("  sequence number: " << interestStringComponent[4]);
                        contentNameCmp = NameComponents(nameReply);
                        res = producer->sendContent(contentNameCmp, (const uint8_t *) contentToSend->data, contentToSend->size);
                        if (res==-1) {
                            NS_LOG_ERROR("sendContent() failed.");
                        }
                    }
                }
            } else if (FD_ISSET(gpsSocket, &read_fd)) {
                int len = read (gpsSocket, buffer, 2000);
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
