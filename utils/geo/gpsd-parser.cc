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

#include "gpsd-parser.h"
#include "corelib/log.h"

NS_LOG_COMPONENT_DEFINE ("geo.GpsdParser");

namespace vndn {
namespace geo {


#ifdef FAKE
struct FakeGpsMessage {
    double lat;
    double longitude;
};
#endif

int GpsdParser::parseData(char *data, int len, GpsInfo *info)
{
#ifdef FAKE
    FakeGpsMessage *msg = (FakeGpsMessage *) data;
    info->storeInPrevious();
    info->setPosition(msg->lat, msg->longitude, 0, 0, 0);
    //home: 34.048369,-118.464064
    //Boelther Hall: 34.069263,-118.443929
    //Santa Monica pier: 34.011617,-118.496912
    return 1;
#endif

    char *token ;
    char *start ;
    char *end ;
    char val_sec[64] ;
    char val_usec[64] ;
    char val_lat[64] ;
    char val_log[64] ;
    data[len] = '\0' ;
    token = strstr( data , "\"class\":\"TPV\"" ) ;
    if ( token == NULL )
        return -1;
    //parse time
    token = strstr ( data , "\"time\":" ) ;
    if ( token == NULL )
        return -1;
    start = strstr ( token , ":" ) ;
    end = strstr ( token , "." ) ;
    strncpy ( val_sec, start + 1, end - ( start + 1 ) ) ;
    val_sec[end - ( start + 1 )] = '\0' ;
    token = start = end ;
    end = strstr ( token , "," ) ;
    if ( token == NULL )
        return -1;
    strncpy ( val_usec, start + 1, end - ( start + 1 ) ) ;
    val_usec[end - ( start + 1 )] = '\0' ;
    //parse latitude
    token = strstr ( data , "\"lat\":" ) ;
    if ( token == NULL)
        return -1;
    start = strstr ( token , ":" ) ;
    end = strstr ( token , "," ) ;
    strncpy ( val_lat, start + 1, end - ( start + 1 ) ) ;
    val_lat[end - ( start + 1 )] = '\0' ;
    //parse longitude
    token = strstr ( data , "\"lon\":" ) ;
    if ( token == NULL )
        return -1;
    start = strstr ( token , ":" ) ;
    end = strstr ( token , "," ) ;
    strncpy ( val_log, start + 1, end - ( start + 1 ) ) ;
    val_log[end - ( start + 1 )] = '\0' ;
    //Update data
    /*
     *      sscanf( val_sec , "%lu", &(packet->sec) );
            if ( DEBUG ) printf ( "From GPSd: time (sec) %lu [%x]\n" , packet->sec , ( unsigned int ) packet->sec ) ;
                    sscanf( val_usec , "%lu", &(packet->usec) );
                    if ( DEBUG ) printf ( "From GPSd: time (usec) %lu [%x]\n" , packet->usec , ( unsigned int ) packet->usec ) ;
                    sscanf( val_lat , "%lf", &(packet->lat) );
                    if ( DEBUG ) printf ( "From GPSd: lat %lf [%x]\n" , packet->lat , ( unsigned int ) packet->lat ) ;
                    sscanf( val_log , "%lf", &(packet->log) );
                    if ( DEBUG ) printf ( "From GPSd: log %lf [%x]\n" , packet->log , ( unsigned int ) packet->log) ;

    */

    info->storeInPrevious();

    //testing
    /*  double randomLat = 34 + (rand()%1000)/10000.0 ;
    double randomLong = -118 - (rand()%1000)/10000.0;

    info->setPosition(randomLat,randomLong, 0, 0, 0);*/
    //end test

    info->setPosition(atof(val_lat), atof(val_log), 0, 0, 0);
    NS_LOG_INFO("New position: latitude = " << info->getLatChar() << "; longitude = " << info->getLonChar());
    return 1;
}

} /* namespace geo */
} /* namespace vndn */
