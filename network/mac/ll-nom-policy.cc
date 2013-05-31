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

#include "ll-nom-policy.h"
#include "corelib/log.h"

NS_LOG_COMPONENT_DEFINE ("LLNomPolicy");

namespace vndn
{

using namespace geo;

/*
 * Be aware that:
 * if we want to implement a mechanism that notifies the NDN Daemon every time a retransmission counter has expired, we can't delete a pkt
 * inside getPktForRetransmission, but we have to wait the next deadline, when we will delete the pkt and notify the NDN daemon (without sending out the pkt)
 */

// TODO: Would the policy store the entire pkt (with LL header) or only the payload? the header (the position) should be recreated at every retransmission


LLNomPolicy::LLNomPolicy()
{
    srand(time(NULL));
    ackManager = new LALAckManagerByDistance();
}

LLNomPolicy::~LLNomPolicy()
{
    delete ackManager;
}


int LLNomPolicy::addOutgoingPkt(uint8_t pkt[], int *len, LLMetadata80211AdHoc *metadata, const LocationService & locationService)
{
    NS_LOG_DEBUG("LLNomPolicy::addOutgoingPkt pkt len: " << *len);
    std::string name;
    Ptr<const NameComponents> components;
    uint32_t nonce;
#ifdef LAL_STATISTICS
    bool packetSentIsAContent;
#endif
    const Ptr<Packet> p =  Packet::InitFromBuffer((const uint8_t *)pkt, *len);
    try {
        NDNHeaderHelper::Type type = NDNHeaderHelper::GetNDNHeaderType (p);
        switch (type) {
        case NDNHeaderHelper::INTEREST: {
#ifdef LAL_STATISTICS
            statistics.increaseSendingOutInterestRequest();
            packetSentIsAContent=false;
#endif
            Ptr<InterestHeader> header = GetHeader<InterestHeader> (*p);
            components = header->GetName();
            name.append(interestKey);
            NS_LOG_DEBUG("NAME " << * header->GetName());;
            nonce = header->GetNonce();
            NS_LOG_DEBUG("NONCE: " << nonce);
            std::list<std::string> headerCmp = header->GetName()->GetComponents();
            std::list<std::string>::iterator i;
            for (i = headerCmp.begin(); i != headerCmp.end(); i++) {
                name.append(*i);
                name.append("/");
            }
            NS_LOG_INFO("Received a pkt from NDND: type: INTEREST, name: "<< * header->GetName()<<" lenght: "<< *len<< " nonce: "<< nonce);
            break;
        }
        case NDNHeaderHelper::CONTENT_OBJECT: {
#ifdef LAL_STATISTICS
            statistics.increaseSendingOutContentRequest();
            packetSentIsAContent=true;
#endif
            Ptr<ContentObjectHeader> header = GetHeader<ContentObjectHeader> (*p);
            components = header->GetName();
            name.append(dataKey);
            std::string ndnName;
            nonce = -1; //content object doesn't have a nonce
            std::list<std::string> headerCmp = header->GetName()->GetComponents();
            NS_LOG_DEBUG("NAME " << header->GetName());
            std::list<std::string>::iterator i;
            for (i = headerCmp.begin(); i != headerCmp.end(); ++i) {
                ndnName.append(*i);
                ndnName.append("/");
            }
            name.append(ndnName);
            NS_LOG_INFO("Received a pkt from NDND: type: CONTENT, name: "<< * header->GetName()<<" lenght: "<< *len);
            
            /**we need to check if there is a pending interest for this content (the content could arrive from an other interface) */
            std::list<const PacketStorage::linkLayerPktElement *> matchingElements;
            storage.searchByPrefixMatch(header->GetName(), matchingElements);
            std::list<const PacketStorage::linkLayerPktElement *>::iterator matchIt = matchingElements.begin();
            if (matchIt==matchingElements.end()) {
                /**there is no pending interest for that content*/
            } else {
                while(matchIt!=matchingElements.end()){
                    const PacketStorage::linkLayerPktElement *el = *matchIt;
                    NS_LOG_INFO("The content "<< * header->GetName() <<" satisfies the pending interest: " << el->name);
                    if (storage.deletePktByKey(el->name) == -1) {
                        NS_LOG_WARN("WARNING delete from storage failed. The pkt will be discarded anyway");
                    }
                    matchIt++;
                }
            }
            break;
        }
        default: {
            NS_LOG_WARN("Received a packet from NDND with an unknow pkt type");
            return DISCARD;
        }
        }
    } catch (NDNUnknownHeaderException) {
        NS_LOG_WARN("Received a packet from NDND with an unknow pkt type: NDNUnknownHeaderException");
        return DISCARD; //unknow pkt, it has to be discarded
    } catch (InterestHeaderException) {
        NS_LOG_WARN("Receiving a packet from NDND caused NDNInterestHeaderException");
        return DISCARD; //unknow pkt, it has to be discarded
    } catch (CcnbParser::CcnbDecodingException) {
        NS_LOG_WARN("Receiving a packet from NDND caused CcnbDecodingException");
        return DISCARD;
    }
    //Add link layer header at the pkt
    uint8_t data[(*len) + sizeof(llHeader)];
    llHeader llhdr;

    *len = setLLHeader(&llhdr, data, pkt, *len, locationService);
    memcpy(pkt, data, *len);

    std::pair <unsigned int, unsigned int> time;
    //calculateNextTimer(0,maxRetransmissionNumber,&(time.first), &(time.second));

    //TODO pkt geo info has to be stored separately from data
    //header will be added, but only in the data that goes back to LLDaemon. The packetStorage stores only data
    int res;
    if (metadata == NULL) { //pkt generated locally.
        NS_LOG_INFO("the packet received from NDND is generated locally, there is no metadata attached");
        GeoStorage geoS (locationService.getLatitude(), locationService.getLongitude());
        calculateFirstTransmission(NULL, locationService, &(time.first), &(time.second));
        AckInfo *ackInfo = ackManager->createAckInfo(locationService, NULL, llhdr);
        res = storage.insertPkt(data, *len, maxRetransmissionNumber, name,components, nonce, time, geoS, ackInfo);

    } else { //pkt has been forwarded, so we're keeping the position information about the previous hop
        if ((DEFAULT_COORDINATE_DOUBLE == metadata->getPreviousHopInfo().getLat()) || (DEFAULT_COORDINATE_DOUBLE == metadata->getPreviousHopInfo().getLongitude())) {
            NS_LOG_INFO("the packet received from NDND has been forwarded, but there is no valid gps info attached");
            //the source node hadn't valid gps coordinates. The packet should be considered as a locally generated packet (expect for TOS)
            GeoStorage geoS (locationService.getLatitude(), locationService.getLongitude());
            calculateFirstTransmission(NULL, locationService, &(time.first), &(time.second));
            AckInfo *ackInfo = ackManager->createAckInfo(locationService, NULL, llhdr);
            res = storage.insertPkt(data, *len, maxRetransmissionNumber, name, components,nonce, time, geoS, ackInfo);
        } else {
            NS_LOG_INFO("the packet received from NDND has been forwarded from latitude: "<<metadata->getPreviousHopInfoAddr()->getLat()<<", longitude: "<<metadata->getPreviousHopInfoAddr()->getLongitude());
            calculateFirstTransmission(metadata->getPreviousHopInfoAddr(), locationService, &(time.first), &(time.second));
            AckInfo *ackInfo = ackManager->createAckInfo(locationService, metadata->getPreviousHopInfoAddr(), llhdr);
            res = storage.insertPkt(data, *len, maxRetransmissionNumber, name,components, nonce, time, metadata->getPreviousHopInfo(), ackInfo);
        }
        delete metadata;

    }
    if (res == -1) {
        NS_LOG_ERROR("Insertion in the storage failed");
        return -1;
    } else {
#ifdef LAL_STATISTICS
        if (packetSentIsAContent) {
            statistics.increaseContentSent();
        } else {
            statistics.increaseInterestSent();
        }
#endif
    }
    return DISCARD; //GOTONETWORK;
}

int LLNomPolicy::pktFromNetwork(uint8_t pkt[], int *len, void *&dataWithoutLLHeader, const LocationService & locationService)
{
    NdnSocket::ndnSocketMetaData *ndnSocketInfo = (NdnSocket::ndnSocketMetaData *) pkt;
    NS_LOG_DEBUG("received pkt from : " << PRINTABLE_MAC_ADDRESS(ndnSocketInfo->sourceMacAddress));
    //TODO give ndnSocketInfo to a hypothetic neighbouring services (if it exist)
    llHeader *llhdr = (llHeader *) &pkt[sizeof(NdnSocket::ndnSocketMetaData)];
    *len = *len - sizeof(NdnSocket::ndnSocketMetaData);
    *len = (*len) - sizeof(llHeader);
    if (*len <= 0) {
        return DISCARD;
    }
    GeoStorage sourceGeoS (atof(llhdr->lat), atof(llhdr->longitude) );
    //TODO get the TOS
#ifdef TEST_GPS_MOVING
    if (!isReachable(sourceGeoS, locationService)) //node to far, pkt not received
        return DISCARD;
#endif

    dataWithoutLLHeader = &(pkt[sizeof(llHeader) + sizeof(NdnSocket::ndnSocketMetaData)]);
    std::string name;
    uint32_t nonce;
    int returnCommand;
    try {
        const Ptr<Packet> p = Packet::InitFromBuffer((const uint8_t *)dataWithoutLLHeader, *len);
        NDNHeaderHelper::Type type = NDNHeaderHelper::GetNDNHeaderType (p);

        const PacketStorage::linkLayerPktElement *el = NULL;
        switch (type) {
        case NDNHeaderHelper::INTEREST: {
#ifdef LAL_STATISTICS
            statistics.increaseReceivedInterest();
#endif
            Ptr<InterestHeader> header = GetHeader<InterestHeader> (*p);
            nonce = header->GetNonce();
            name.append(interestKey);
            std::list<std::string> headerCmp = header->GetName()->GetComponents();
            std::list<std::string>::iterator i;
            for (i = headerCmp.begin(); i != headerCmp.end(); ++i) {
                name.append(*i);
                name.append("/");
            }
            NS_LOG_INFO("Received data from network. Position: lat:" << llhdr->lat << ", long: " << llhdr->longitude <<
                        ", previous hop MAC address: "<< PRINTABLE_MAC_ADDRESS(ndnSocketInfo->sourceMacAddress) <<
                        ", type: INTEREST, name: "<< * header->GetName() <<", length: "<< *len<<", nonce: "<<header->GetNonce());
            
            if (storage.getPktByKey(name, el) == -1) {
                NS_LOG_DEBUG("LLNomPolicy-pktFromNetwork: pkt " << name << " NOT found in storage");
                returnCommand = GOUPLAYER;
#ifdef LAL_STATISTICS
                statistics.increaseReceivedInterestGoingUp();
#endif
            } else {
                if (el->nonce != nonce) { //same interest but different nonce. it means that the source of the packets are different
                    NS_LOG_INFO("Interest received from network with name "<< * header->GetName()<<
                                "match a pending interest, but they have a different nonce: "<< header->GetNonce() <<
                                " (received), "<<el->nonce<<" (pending interest)");
                    returnCommand = GOUPLAYER;
#ifdef LAL_STATISTICS
                    statistics.increaseReceivedInterestGoingUp();
#endif
                } else {
                    std::pair<bool, int> ackResponse = ackManager->receivedRetransmission(llhdr, el, locationService);
                    if (ackResponse.first && ackResponse.second == 0) {
                        NS_LOG_INFO("Interest received from network with name "<< * header->GetName()<<
                                    " acked a pending interest (nonce "<<el->nonce <<"). Statistics: retransmission: "<<el->retransmission<<
                                    ", number of ack received: "<<el->ackInfo->getNumberOfAck());
#ifdef LAL_STATISTICS
                        statistics.increaseAckedPacket();
#endif
                        if (storage.deletePktByKey(name) == -1) {
                            NS_LOG_WARN("WARNING LLNomPolicy-pktFromNetwork: delete from storage failed. The pkt will be discarded anyway");
                        }
                        returnCommand = DISCARD;
                    } else {
                        NS_LOG_INFO("Interest received from network with name "<< * header->GetName()<<
                                    " is a partial ack for a pending interest. Statistics: retransmission: "<<el->retransmission<< ", number of ack received: "<<el->ackInfo->getNumberOfAck()<<
                                    "is the packet a push progress? "<<ackResponse.first);
#ifdef LAL_STATISTICS
                        statistics.increaseReceivedInterestGoingUp();
#endif
                        returnCommand = GOUPLAYER;
                    }
                }
            }
            break;
        }
        case NDNHeaderHelper::CONTENT_OBJECT: {
#ifdef LAL_STATISTICS
            statistics.increaseReceivedContent();
#endif
            Ptr<ContentObjectHeader> header = GetHeader<ContentObjectHeader> (*p);
            nonce = -1;
            name.append(dataKey);
            std::list<std::string> headerCmp = header->GetName()->GetComponents();
            std::list<std::string>::iterator i;
            for (i = headerCmp.begin(); i != headerCmp.end(); ++i) {
                name.append(*i);
                name.append("/");
            }
            NS_LOG_INFO("Received data from network. Position: lat:" << llhdr->lat << ", long: " << llhdr->longitude <<
                        "previous hop MAC address: "<< PRINTABLE_MAC_ADDRESS(ndnSocketInfo->sourceMacAddress) <<
                        "type: CONTENT, name: "<< * header->GetName() <<", length: "<< *len);
            
            if (storage.getPktByKey(name, el) == -1) {
                NS_LOG_DEBUG("LLNomPolicy-pktFromNetwork: pkt " << name << " NOT found in storage");
                returnCommand = GOUPLAYER;
#ifdef LAL_STATISTICS
                statistics.increaseReceivedContentGoingUp();
#endif
            } else {
                NS_LOG_DEBUG("LLNomPolicy-pktFromNetwork: pkt " << name << " found in storage");
                std::pair<bool, int> ackResponse = ackManager->receivedRetransmission(llhdr, el, locationService);
                if (ackResponse.first && ackResponse.second == 0) {
                    NS_LOG_INFO("Content received from network with name "<< * header->GetName()<<
                                " acked a pending content. Statistics: retransmission: "<<el->retransmission<<
                                ", number of ack received: "<<el->ackInfo->getNumberOfAck());
#ifdef LAL_STATISTICS
                    statistics.increaseAckedPacket();
#endif
                    if (storage.deletePktByKey(name) == -1) {
                        NS_LOG_WARN("WARNING LLNomPolicy-pktFromNetwork: delete from storage failed. The pkt will be discarded anyway");
                    }
                    returnCommand = DISCARD;
                } else {
                    NS_LOG_INFO("Content received from network with name "<< * header->GetName()<<
                                " is a partial ack for a pending content. Statistics: retransmission: "<<el->retransmission<<
                                ", number of ack received: "<<el->ackInfo->getNumberOfAck()<<
                                "is the packet a push progress? "<<ackResponse.first);
#ifdef LAL_STATISTICS
                    statistics.increaseReceivedContentGoingUp();
#endif
                    returnCommand = GOUPLAYER;
                }
            }
            //checking if there is a pending interest for the same data
            std::list<const PacketStorage::linkLayerPktElement *> matchingElements;
            storage.searchByPrefixMatch(header->GetName(), matchingElements);
            std::list<const PacketStorage::linkLayerPktElement *>::iterator matchIt = matchingElements.begin();
            if (matchIt==matchingElements.end()) {
                /**there is no pending interest for that content*/
            } else {
                const PacketStorage::linkLayerPktElement *el = *matchIt;
                while(matchIt!=matchingElements.end()){
                    NS_LOG_INFO("Content received from network with name "<< * header->GetName()<<
                                " acked the pending interest: " << el->name<<". Statistics: retransmission: "<<el->retransmission<<
                                ", number of ack received: "<<el->ackInfo->getNumberOfAck());
#ifdef LAL_STATISTICS
                    statistics.increaseInterestAckedByContent();
#endif
                    if (storage.deletePktByKey(el->name) == -1) {
                        NS_LOG_WARN("WARNING delete from storage failed. The pkt will be discarded anyway");
                    }
                    matchIt++;
                }
            }
            break;
        }
        default: {
            NS_LOG_WARN("LLNomPolicy: pktFromNetwork, unknow pkt type");
            return DISCARD;
        }
        }
    } catch (NDNUnknownHeaderException) {
        NS_LOG_WARN("LLNomPolicy: pktFromNetwork, unknow pkt type");
        return DISCARD; //unknow pkt, it has to be discarded
    } catch (InterestHeaderException) {
        NS_LOG_WARN("LLNomPolicy-pktFromNetwork, NDNInterestHeaderException");
        return DISCARD; //unknow pkt, it has to be discarded
    } catch (CcnbParser::CcnbDecodingException) {
        NS_LOG_WARN("caught NDNDecodingException");
        return DISCARD;
    }
    if (returnCommand == GOUPLAYER) {
        LLMetadata80211AdHoc *metaData = new LLMetadata80211AdHoc(sourceGeoS);
        metaData->setTos(ntohl(llhdr->tos));
        int res = communicationService->writeMessageToNDN(dataWithoutLLHeader, *len, metaData);
        if (res == -1) {
            NS_LOG_ERROR("ERROR LLNomPolicy send pkt to NDN layer failed " << strerror(errno));
            return -1;
            //TODO exception + thread management ?
        } else {
            NS_LOG_INFO("Sending the received pkt up to NDND");
        }
        returnCommand = DISCARD;
    }
    return returnCommand;
}

int LLNomPolicy::getNextDeadline(long int *sec, long int *usec)
{
    NS_LOG_DEBUG("LLNomPolicy::getNextDeadline");
    const PacketStorage::linkLayerPktElement *el;
    if (storage.getFirstDeadline(el) == -1) {
        NS_LOG_DEBUG("LLNomPolicy::getNextDeadline, storage is empty");
        //the storage is empty
        return NOTIMER;
    }
    *sec = el->timer.first;
    *usec = el->timer.second;
    NS_LOG_DEBUG("LLNomPolicy::getNextDeadline: " << *sec << "sec, " << *usec << " usec");
    return 1;
}

int LLNomPolicy::getPktForRetransmission(uint8_t ptrData[], const LocationService &locationService)
{
    const PacketStorage::linkLayerPktElement *el = NULL;
    if (storage.getFirstDeadline(el) == -1) {
        //the storage is empty
        return -1;
    }
    NS_LOG_DEBUG("size of pkt being retransmitted: " << el->size);
    int newSize = el->size;//setLLHeader(&llhdr, ptrData, el->data, el->size); //TODO restore this when the storage will not store the header
    //TODO the header haa to be updated (gps information)
    memcpy(ptrData, el->data, newSize);
    struct llHeader *hdr = (llHeader *)ptrData;
    updateLLHeaderCoordinates(hdr, locationService);

#ifdef LAL_STATISTICS
    if (el->retransmission != 1) {
        statistics.increaseNumberOfRetransmission();
    }
#endif

    if (el->retransmission >= (el->retransmissionLimit)) {
        NS_LOG_INFO("Last retransmission for the packet: type+name: " << el->name<< ", size: "<< el->size<<
                    ", the pkt is being transmitted for the "<<el->retransmission<<" times, number of received ack: "<<
                    el->ackInfo->getNumberOfAck());
        //check section BE AWARE at the beginning of the file if you have to change this function
        if (storage.deleteFirstPkt(el->name) == -1) {
            NS_LOG_WARN("WARNING LLNomPolicy-getPktForRetransmission: delete from storage failed. The pkt will be discarded anyway");
        }
    } else {
        std::pair <unsigned int, unsigned int> newTime;
        calculateNextTimer(el->retransmission, el->retransmissionLimit, &(newTime.first), &(newTime.second));
        if (storage.increaseRetransmissionCounterAndSetNewTimer(el->name, newTime ) == -1) {
            NS_LOG_WARN("WARNING LLNomPolicy-getPktForRetransmission: increaseRetransmissionCounterAndSetNewTimer failed");
            //deleting the packet to avoid infinite loop (retransmission number never incremented)
            if (storage.deleteFirstPkt(el->name) == -1) {
                NS_LOG_WARN("WARNING LLNomPolicy-getPktForRetransmission: delete from storage failed. The pkt will be discarded anyway");
            }
            return -1;
        }
        NS_LOG_INFO("Sending a packet out: type+name: " << el->name<< ", size: "<< el->size<<
                    ", the pkt is being transmitted for the "<<el->retransmission<<" times, number of received ack: "<<
                    el->ackInfo->getNumberOfAck());
    }
    return newSize;
}


void LLNomPolicy::calculateNextTimer(int numberOfRetransmission, int maxNumberOfRetransmission, unsigned int *sec, unsigned int *usec)
{
    //TODO this is just a temporary nextTimer calculation
    struct timeval tt;
    gettimeofday(&tt, NULL);
    *usec = (usecFirstRetransmission + tt.tv_usec) % 1000000;
    *sec =  secFirstRetransmission + tt.tv_sec + (usecFirstRetransmission + tt.tv_usec) / 1000000;
}


void LLNomPolicy::calculateFirstTransmission(GeoStorage *geoStorage, const LocationService &locationService, unsigned int *sec, unsigned int *usec)
{
    struct timeval tt;
    gettimeofday(&tt, NULL);
    int Tgap = 0;
    int Tca = rand() % maxTca;
    if (geoStorage != NULL) { //the pkt that needs to be transmitted is not generated locally
        double distance = locationService.getDistance(geoStorage->getLat(), geoStorage->getLongitude());
        /*Tgap is microsecond, so it has no sense to use a double, we can just get the integer part*/
        Tgap = (int) Tdist * (Dmax - std::min((double)Dmax * 1.0, distance)) / Dmax;
    }
    NS_LOG_INFO("Timer for first transmission: Tgap: "<< Tgap << ", Tca" << Tca);
    *usec = (tt.tv_usec + Tca + (Tgap)) % 1000000;
    *sec =  tt.tv_sec + (tt.tv_usec + Tca + (Tgap)) / 1000000;
}

int LLNomPolicy::getNDNHeader(const Ptr<const Packet> &p, Ptr<Header> header)
{
    try {
        NDNHeaderHelper::Type type = NDNHeaderHelper::GetNDNHeaderType (p);
        switch (type) {
        case NDNHeaderHelper::INTEREST: {
            header = GetHeader<InterestHeader> (*p);
            return  NDNHeaderHelper::INTEREST;
        }
        case NDNHeaderHelper::CONTENT_OBJECT: {
            header = GetHeader<ContentObjectHeader> (*p);
            return  NDNHeaderHelper::CONTENT_OBJECT;
        }
        }
    } catch (NDNUnknownHeaderException) {
        NS_LOG_WARN("LLNomPolicy: unknow pkt type");
        return -1; //unknow pkt, it has to be discarded
    } catch (InterestHeaderException) {
        NS_LOG_WARN("LLNomPolicy, InterestHeaderException");
        return -1; //unknow pkt, it has to be discarded
    } catch (CcnbParser::CcnbDecodingException) {
        NS_LOG_WARN("caught NDNDecodingException");
        return -1;
    }
    return -1;
}

void LLNomPolicy::setLLupperLayerCommunication(LLUpperLayerCommunicationService *upperLayerComServ)
{
    communicationService = upperLayerComServ;
}

int LLNomPolicy::setLLHeader(llHeader *llhdr, uint8_t *buffer, const uint8_t *data, int len, const LocationService &locationService)
{
    llhdr->tos = htonl(75); //TODO just temporary. NDND or application should determinate this value
    memcpy(llhdr->lat, locationService.getLatChar(), GPS_STRING_SIZE);
    memcpy(llhdr->longitude, locationService.getLonChar(), GPS_STRING_SIZE);
    memcpy(buffer, llhdr, sizeof(llHeader));
    memcpy(&(buffer[sizeof(llHeader)]), data, len); //now the LLDaemon can free the pkt area when it will send the pkt
    return len + sizeof(llHeader);
}

void LLNomPolicy::updateLLHeaderCoordinates(llHeader *llhdr, const LocationService &locationService)
{
    memcpy(llhdr->lat, locationService.getLatChar(), GPS_STRING_SIZE);
    memcpy(llhdr->longitude, locationService.getLonChar(), GPS_STRING_SIZE);
}

#ifdef TEST_GPS_MOVING
bool LLNomPolicy::isReachable(GeoStorage &position, const LocationService &locationService)
{
    if (locationService.getDistance(position.getLat(), position.getLongitude()) > 120) {
        return false;
    }
    return true;
}
#endif

void LLNomPolicy::printStatistic() 
{
#ifdef LAL_STATISTICS
    statistics.sendToLog();
#endif
}

}
/* namespace vndn */
