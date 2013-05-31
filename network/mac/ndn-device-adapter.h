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

#ifndef NDNDEVICEADAPTER_H_
#define NDNDEVICEADAPTER_H_

#include <sys/un.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/eventfd.h>
#include <semaphore.h>

#include "ndnsock/ndn-raw-socket.h"
#include "ndnsock/ndn-socket-exception.h"
#include "ll-policy.h"
#include "ll-nom-policy.h"
#include "ll-device.h"
#include "link-layer.h"
#include "ll-upper-layer-comm-service.h"
#include "lal-stats.h"

#include "utils/geo/location-service.h"
#include "utils/geo/gpsd-parser.h"
#include "utils/geo/gps-info.h"

// Gpsd address
#define GPSADDRESS "127.0.0.1"

// Gpsd port
#define GPSPORT 2947 //10000//2947

// Gpsd command (to get json data periodically through  a socket)
//#define GPSCOMMAND "?WATCH={\"enable\":true,\"raw\":true}"
#define GPSCOMMAND "?WATCH={\"enable\":true,\"json\":true}"  //?WATCH={"enable":true,"json":true}


namespace vndn
{

/**
 * \brief It's the connection point between NDN and the link adaptation layer that has to adapt NDN to the link layer characteristics
 *
 * It Coordinates the several NDN-Link adaptation layer elements, the communication with NDN (upper layer) , with NDNSocket (lower layer) and the Gps-location service.
 * About the latter, right now it just use the information available from gpsd
 * */
class NDNDeviceAdapter
{

public:
    /**
     * \brief Struct used to give to NDNDeviceAdapter thread all the parameters
     * */
    struct NDNDeviceParams {
        /**incoming trigger fd used to trigger communication from NDN-Link adaptation layer to NDN (see LLUpperLayerCommunicationService)*/
        int incomingEventFd;
        /**outgoing trigger fd used to trigger communication from NDN to NDN-Link adaptation layer (see LLUpperLayerCommunicationService)*/
        int outgoingEventFd;

        /**Number of elements stored in the shared memory used by NDN and NDN-LAL  (see LLUpperLayerCommunicationService)*/
        int numEl;      //number of elements stored in the shared memory

        /**Address of incoming shared memory  (see LLUpperLayerCommunicationService)*/
        struct LLUpperLayerCommunicationService::NDNDevicePktExchange *incomingSharedMemoryPtr;
        /**Address of outgoing shared memory  (see LLUpperLayerCommunicationService)*/
        struct LLUpperLayerCommunicationService::NDNDevicePktExchange *outgoingSharedMemoryPtr;

        /**Interface Network name associated to NDNDeviceAdapter*/
        std::string deviceName;
    };

    struct NDNDeviceMetaData {
        int size;
        //TODO type? info about pkt, about interface
    };

    //param: number of interfaces, sequence of interfaces name
    /**
     * \brief Create a NDNDeviceAdapter specifying the associated network interface
     *
     * \param devicesName Network interface name
     * */
    NDNDeviceAdapter(std::string devname);

    virtual ~NDNDeviceAdapter();

    /**
     * \brief Run NDNDeviceAdatper
     *
     *
     * \param param see NDNDeviceParams struct
     * */
    int start(NDNDeviceParams *param);

protected:
    /**
     * \brief set up the communication with gps daemon
     * */
    int createGPSSocket();

    /**
     * \brief connect to ndnSocket 
     * 
     * It connects to a new socket, that will be stored in the LLDevice of the class. 
     * If the llDevice was already associated with an active ndnSocket, this will be closed before the creation of the new one
     * \param devicesName name of the device (wlan0, wifi2 ...)
     * \return 1 if everything is ok, -1 otherwise
     * */
    int connectToNDNSocket();


    /**
     * LLdevice associated to this NDNDeviceAdapter
     * */
    LLDevice device;

    /**Socket used to send command to gpsd and receive back data about the node location*/
    int gpsdSocket;

    /**
     * \brief LocationService used to store actual node position and to get info about position of other nodes or packets
     * */
    geo::LocationService locationService;

    /**Buffer used for temporary data storage*/
    uint8_t *buffer;

    /**
     * It parses the data got from gpsd (in json format) and translate them to GpsInfo
     * */
    geo::GpsdParser gpsParser;

    /**
     * It allows the communication between NDN and NDN-LAL
     * */
    LLUpperLayerCommunicationService upperLayerComServ;

    timeval nextStatisticUpdate;

};

} /* namespace vndn */
#endif /* LLDAEMON_H_ */
