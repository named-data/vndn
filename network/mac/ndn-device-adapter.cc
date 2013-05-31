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

#include "ndn-device-adapter.h"
#include "corelib/log.h"

#define STATISTIC_FREQUENCY_UPDATE 10 //how many seconds between one update and the next one. LAL_STATISTICS has to be defined to enable statistics

NS_LOG_COMPONENT_DEFINE("NDNDeviceAdapter");


namespace vndn
{

NDNDeviceAdapter::NDNDeviceAdapter(std::string devname)
{
    NS_LOG_FUNCTION(this << devname);

    const unsigned char broadcastAddr[ETH_ALEN] = { 0xff , 0xff , 0xff , 0xff , 0xff , 0xff };

    NdnRawSocket *ndnSocket;
    try {
        ndnSocket = new NdnRawSocket(devname);
    } catch (NdnSocketException e) {
        throw e.what();
    }

    LLNomPolicy *policy = new LLNomPolicy;
    device = LLDevice(policy, ndnSocket);
    device.setName(devname);
    device.getNdnSocket()->setDestination(broadcastAddr);  //right now only broadcast communication is allowed
    NS_LOG_DEBUG("socket id = " << device.getNdnSocket()->getSocket());

    buffer = new uint8_t[MAXNETWORKPKTSIZE + sizeof(NdnSocket::ndnSocketMetaData)];

    gpsdSocket = createGPSSocket();
    if (gpsdSocket == -1) {
        NS_LOG_ERROR("Could not open the gpsd socket");
        // will try to reconnect to gpsd later
    }

#ifdef LAL_STATISTICS
    gettimeofday(&nextStatisticUpdate, NULL);
    nextStatisticUpdate.tv_sec += STATISTIC_FREQUENCY_UPDATE;
#endif
}

NDNDeviceAdapter::~NDNDeviceAdapter()
{
    delete [] buffer;
    delete device.getNdnSocket();
}


int NDNDeviceAdapter::start(NDNDeviceParams *param)
{
    NS_LOG_INFO("device=" << device.getName());

    upperLayerComServ.setNumberOfSharedElement(param->numEl);
    upperLayerComServ.setIncomingSharedMemoryPtr(param->incomingSharedMemoryPtr);
    upperLayerComServ.setOutgoingSharedMemoryPtr(param->outgoingSharedMemoryPtr);
    upperLayerComServ.setNdnOutgoingTrigger(param->outgoingEventFd);
    upperLayerComServ.setNdnIncomingTrigger(param->incomingEventFd);
    device.getPolicy()->setLLupperLayerCommunication(&upperLayerComServ);
    //prepare struct for select
    fd_set read_fd;
    FD_ZERO(&read_fd);
    int maxfd = -1;
    if (maxfd < device.getNdnSocket()->getSocket()) {
        maxfd = device.getNdnSocket()->getSocket();
    }
    if (maxfd < upperLayerComServ.getNdnOutgoingTrigger()) {
        maxfd = upperLayerComServ.getNdnOutgoingTrigger();
    }
    struct timeval tvNextDeadline, auxTimeVal;
    tvNextDeadline.tv_sec = LLPolicy::NOTIMER;
    tvNextDeadline.tv_usec = LLPolicy::NOTIMER;
    auxTimeVal.tv_sec = LLPolicy::NOTIMER;
    auxTimeVal.tv_usec = LLPolicy::NOTIMER;
    int len = -1;
    int cmd;
    int selectResult;

    //send the request to gpsd
    if (gpsdSocket != -1) {
        memcpy(buffer, GPSCOMMAND, strlen(GPSCOMMAND));
        len = send(gpsdSocket, buffer, strlen(GPSCOMMAND), 0);
        if (len == -1) {
            NS_LOG_ERROR("Failed to send request to gpsd. Closing the socket.");
            close(gpsdSocket);
            gpsdSocket = -1;
        } else { //NDNDeviceAdapter can use gpsdSocket
            if (maxfd < gpsdSocket) {
                maxfd = gpsdSocket;
            }
        }
    }

    bool selectForGpsd = false;
    while (true) {
        selectForGpsd = false;
        FD_SET(device.getNdnSocket()->getSocket(), &read_fd);
        FD_SET(upperLayerComServ.getNdnOutgoingTrigger(), &read_fd);
        if (gpsdSocket == -1) {
            //try again to reconnect to gpsd
            gpsdSocket = createGPSSocket();
            //send the request to gpsd
            if (gpsdSocket != -1) {
                memcpy(buffer, GPSCOMMAND, strlen(GPSCOMMAND));
                len = send(gpsdSocket, buffer, strlen(GPSCOMMAND), 0);
                if (len == -1) {
                    NS_LOG_ERROR("Failed to send request to gpsd. Closing the socket.");
                    close(gpsdSocket);
                    gpsdSocket = -1;
                } else { //NDNDeviceAdapter can use gpsdSocket
                    if (maxfd < gpsdSocket) {
                        maxfd = gpsdSocket;
                    }
                }
            } else {
                //set GpsInfo with default error value
                locationService.noValidData();
            }
        }
        if (gpsdSocket > 0) {
            FD_SET(gpsdSocket, &read_fd);
        }
        if (tvNextDeadline.tv_sec == LLPolicy::NOTIMER) {
            if (gpsdSocket==-1) {
                NS_LOG_DEBUG("select with no timer and no gpsd, maxfd=" << maxfd);
                tvNextDeadline.tv_sec=2;  /**after a while with no traffic, we have to try again to connect to gpsd */
                tvNextDeadline.tv_usec=0;
                selectResult = select(maxfd + 1, &read_fd, NULL, NULL, &tvNextDeadline);
                tvNextDeadline.tv_sec = LLPolicy::NOTIMER;
                tvNextDeadline.tv_usec = LLPolicy::NOTIMER;
                selectForGpsd=true;
            } else {
                NS_LOG_DEBUG("select with no timer, maxfd=" << maxfd);
                selectResult = select(maxfd + 1, &read_fd, NULL, NULL, NULL);
            }
            if (selectResult == -1) {
                NS_LOG_ERROR("select failed: " << std::strerror(errno));
                return -1;
            }
        } else {
            NS_LOG_DEBUG("select with timer");
            selectResult = select(maxfd + 1, &read_fd, NULL, NULL, &tvNextDeadline);
            if (selectResult == -1) {
                NS_LOG_ERROR("select failed: " << std::strerror(errno));
                return -1;
            }
        }

#ifdef LAL_STATISTICS
        timeval tt;
        gettimeofday(&tt, NULL);
        if (( tt.tv_sec > nextStatisticUpdate.tv_sec ) ||
                (tt.tv_sec == nextStatisticUpdate.tv_sec && tt.tv_usec > nextStatisticUpdate.tv_usec )) {
            //time for an other statistic update
            device.getPolicy()->printStatistic();
            nextStatisticUpdate.tv_sec = tt.tv_sec + STATISTIC_FREQUENCY_UPDATE;
            nextStatisticUpdate.tv_usec = tt.tv_usec;
        }
#endif
        
        if (selectResult == 0 && (!selectForGpsd)) {
            NS_LOG_DEBUG("Timer expired");
            //timer expired
            //This means that NDNDeviceAdapter has to retransmit a pkt
            memset(buffer, 0, MAXNETWORKPKTSIZE);
            len = device.getPolicy()->getPktForRetransmission(buffer, locationService);
            if (len == -1) {
                NS_LOG_WARN("getPktForRetransmission() returned -1");
            } else {
                try {
                    device.getNdnSocket()->send(buffer, len);
                } catch (NdnSocketException) {
                    NS_LOG_ERROR("NdnSocket failed to send the packet");
                    //trying to create a new ndn socket
                    if(connectToNDNSocket()==-1) {
                        NS_LOG_ERROR("Failed to create a new NdnSocket");
                        return -1;
                    }
                }
            }
        } else {
            NS_LOG_DEBUG("select return");
            if (FD_ISSET(device.getNdnSocket()->getSocket(), &read_fd)) {
                //pkt from the network
                memset(buffer, 0, MAXNETWORKPKTSIZE);
                try {
                    len = device.getNdnSocket()->read(buffer, MAXNETWORKPKTSIZE, 1); //flag=1 -> we want to get metadata too (source mac address)
                } catch (NdnSocketException) {
                    NS_LOG_ERROR("read from NdnSocket failed: " << strerror(errno));
                    //trying to create a new ndn socket
                    if(connectToNDNSocket()==-1) {
                        NS_LOG_ERROR("Failed to create a new NdnSocket");
                        return -1;
                    }
                }
                NS_LOG_INFO("Reading from network: " << len);
                void *dataWithoutLLHeader;
                //TODO instead of dataWithoutLLHeader put the address of the correct element of the shared memory
                cmd = device.getPolicy()->pktFromNetwork(buffer, &len, dataWithoutLLHeader, locationService);
                if (cmd == LLPolicy::GOUPLAYER) { //NDNDeviceAdapter has to send the pkt to the ndn daemon
                    len = upperLayerComServ.writeMessageToNDN(dataWithoutLLHeader, len);
                    if (len == -1) {
                        NS_LOG_ERROR("Sending packet to NDN layer failed: " << strerror(errno));
                        return -1;
                    }
                    NS_LOG_INFO("Sent packet to upper layer: " << device.getName() << ", datalen: " << len);
                } else {
                    //NS_LOG_ERROR("Packet received from the network has been discarder (LLNomPolicy decision)");
                }
            }
            if (FD_ISSET(upperLayerComServ.getNdnOutgoingTrigger(), &read_fd)) {
                //pkt from NDN Daemon
                NS_LOG_INFO("Packet from the NDN daemon");
                LLMetadata80211AdHoc *metadata;
                len = upperLayerComServ.readMessageFromNDN(buffer, MAXNETWORKPKTSIZE, (LLMetadata **) &metadata);
                if (len < 0) {
                    NS_LOG_ERROR("Failed to read from LLupperLayerCommunicationService");
                    return -1;
                }
                if (device.getPolicy()->addOutgoingPkt(buffer, &len, metadata, locationService) == LLPolicy::GOTONETWORK) {
                    //pkt has to send to the network through ndn socket
                    try {
                        len = device.getNdnSocket()->send(buffer, len);
                    } catch (NdnSocketException) {
                        NS_LOG_ERROR("Send packet to the network failed: " << strerror(errno));
                        //trying to create a new ndn socket
                        if(connectToNDNSocket()==-1) {
                            NS_LOG_ERROR("Failed to create a new NdnSocket");
                            return -1;
                        }
                    }
                }
            }
        }
        if (gpsdSocket != -1) {
            if (FD_ISSET(gpsdSocket, &read_fd)) {
                NS_LOG_DEBUG("Received data from gpsd");
                memset(buffer, 0, MAXNETWORKPKTSIZE);
                len = read(gpsdSocket, buffer, MAXNETWORKPKTSIZE);
                if (len == 0) {
                    NS_LOG_ERROR("Failed to read from gpsd. Closing the socket.");
                    close(gpsdSocket);
                    gpsdSocket = -1;
                } else {
                    locationService.updatePosition(buffer, len);
                }
            }
        }
        //in the next loop tvNextDeadline and nextDevice will be set properly. If no deadline is found, first will remain 0
        if (device.getPolicy()->getNextDeadline(&tvNextDeadline.tv_sec, &tvNextDeadline.tv_usec) != LLPolicy::NOTIMER) {
            //set tvNextDeadline (it stores the time remaining for the next retransmission)
            //with this approach, we have a gettymeofday every pkt sent or received, but with a single update
            //in this way Policy doesn't have to update the deadline of all the pending pkt
            gettimeofday(&auxTimeVal, NULL);
            if ((tvNextDeadline.tv_sec < auxTimeVal.tv_sec) ||
                    ((tvNextDeadline.tv_sec == auxTimeVal.tv_sec) && tvNextDeadline.tv_usec < auxTimeVal.tv_usec)) {
                //timer already expired. setting the timer to 0 means that select will return immediately
                tvNextDeadline.tv_sec = 0;
                tvNextDeadline.tv_usec = 0;
            } else {
                if (tvNextDeadline.tv_usec < auxTimeVal.tv_usec) {
                    tvNextDeadline.tv_sec--;
                }
                tvNextDeadline.tv_usec = (tvNextDeadline.tv_usec - auxTimeVal.tv_usec + 1000000 ) % 1000000;
                tvNextDeadline.tv_sec = tvNextDeadline.tv_sec - auxTimeVal.tv_sec;
            }
        } else {
            tvNextDeadline.tv_sec = LLPolicy::NOTIMER;
            tvNextDeadline.tv_usec = LLPolicy::NOTIMER;
        }
        FD_ZERO(&read_fd);
    }

    return 1;
}

int NDNDeviceAdapter::createGPSSocket()
{
    NS_LOG_FUNCTION_NOARGS();

    struct sockaddr_in Local, Serv;
    char data[10000];
    int OptVal;
    int socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketfd == -1) {
        return -1;
    }
    int res = setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, (char *)&OptVal, sizeof(OptVal));
    if (res == -1) {
        return -1;
    }
    memset(&Local, 0, sizeof(Local));
    Local.sin_family = AF_INET;
    Local.sin_addr.s_addr = htonl(INADDR_ANY);
    Local.sin_port = htons(0);
    res = bind(socketfd, (struct sockaddr *) &Local, sizeof(Local));
    if (res == -1)  {
        return -1;
    }
    memset(&Serv, 0, sizeof(Serv));
    Serv.sin_family = AF_INET;
    Serv.sin_addr.s_addr = inet_addr(GPSADDRESS);
    Serv.sin_port = htons(GPSPORT);
    /* connection request */
    res = connect(socketfd, (struct sockaddr *) &Serv, sizeof(Serv));
    if (res == -1)  {
        NS_LOG_ERROR("Connection failed: " << strerror(errno));
        return -1;
    }

    NS_LOG_INFO("Connection to gpsd established");

    //read first line from gpsd (there is no useful data in this line)
    res = read(socketfd, data, 1000);
    if (res == -1) {
        NS_LOG_ERROR("First read on gpsd socket failed: " << strerror(errno));
        return -1;
    }

    return socketfd;
}

int NDNDeviceAdapter::connectToNDNSocket()
{
    NS_LOG_FUNCTION_NOARGS();

    const unsigned char broadcastAddr[ETH_ALEN] = { 0xff , 0xff , 0xff , 0xff , 0xff , 0xff };
    NdnRawSocket *ndnSocket;
    try {
        ndnSocket = new NdnRawSocket(device.getName());
    } catch (NdnSocketException e) {
        NS_LOG_ERROR("Failed to create a NdnRawSocket: " << e.what());
        return -1;
    }

    if (device.getNdnSocket()!=NULL) {
        device.getNdnSocket()->closeSocket();
        delete device.getNdnSocket();
    }
    device.setNdnSocket(ndnSocket);
    device.setName(device.getName());
    device.getNdnSocket()->setDestination(broadcastAddr);  //right now only broadcast communication is allowed
    NS_LOG_DEBUG("NdnSocket id = " << device.getNdnSocket()->getSocket());

    return 1;
}

} /* namespace vndn */
