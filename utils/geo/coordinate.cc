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

#include "coordinate.h"
#include "corelib/log.h"

#include <cmath>
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <boost/foreach.hpp>
#include <boost/tokenizer.hpp>

#define DEG2RAD (M_PI / 180.0)
#define RAD2DEG (180.0 / M_PI)

NS_LOG_COMPONENT_DEFINE("geo.Coordinate");


namespace vndn {
namespace geo {

class Ellipsoid
{
public:
    Ellipsoid() {}

    Ellipsoid(int Id, std::string name, double radius, double ecc)
    {
        id = Id;
        ellipsoidName = name;
        EquatorialRadius = radius;
        eccentricitySquared = ecc;
    }

    int id;
    std::string ellipsoidName;
    double EquatorialRadius;
    double eccentricitySquared;
};

class Cons
{
public:
    //  id, Ellipsoid name, Equatorial Radius, square of eccentricity
    //  static const Ellipsoid ellipsoida =Ellipsoid( -1, "Placeholder", 0, 0);
    static Ellipsoid ellipsoid[30];
};

class Conv
{
public:

    /**
     * \brief Convert lat/long coordinate into UTM coordinate (the result will be store in the last 3 parameters)
     */
    static void LLtoUTM(int ReferenceEllipsoid, double Lat, double Long,
                        double & UTMNorthing, double & UTMEasting, std::string & UTMZone)
    {
        //converts lat/long to UTM coords.  Equations from USGS Bulletin 1532
        //East Longitudes are positive, West longitudes are negative.
        //North latitudes are positive, South latitudes are negative
        //Lat and Long are in decimal degrees
        //Written by Chuck Gantz- chuck.gantz@globalstar.com
        Ellipsoid ellipsoid[30];
        ellipsoid[0] = Ellipsoid( -1, "Placeholder", 0, 0);
        ellipsoid[1] = Ellipsoid( 1, "Airy", 6377563, 0.00667054);
        ellipsoid[2] =Ellipsoid( 2, "Australian National", 6378160, 0.006694542);
        ellipsoid[3] =Ellipsoid( 3, "Bessel 1841", 6377397, 0.006674372);
        ellipsoid[4] =Ellipsoid( 4, "Bessel 1841 (Nambia) ", 6377484, 0.006674372);
        ellipsoid[5] =Ellipsoid( 5, "Clarke 1866", 6378206, 0.006768658);
        ellipsoid[6] =                    Ellipsoid( 6, "Clarke 1880", 6378249, 0.006803511);
        ellipsoid[7] =Ellipsoid( 7, "Everest", 6377276, 0.006637847);
        ellipsoid[8] =Ellipsoid( 8, "Fischer 1960 (Mercury) ", 6378166, 0.006693422);
        ellipsoid[9] =Ellipsoid( 9, "Fischer 1968", 6378150, 0.006693422);
        ellipsoid[10] = Ellipsoid( 10, "GRS 1967", 6378160, 0.006694605);
        ellipsoid[11] = Ellipsoid( 11, "GRS 1980", 6378137, 0.00669438);
        ellipsoid[12] = Ellipsoid( 12, "Helmert 1906", 6378200, 0.006693422);
        ellipsoid[13] =Ellipsoid( 13, "Hough", 6378270, 0.00672267);
        ellipsoid[14] = Ellipsoid( 14, "International", 6378388, 0.00672267);
        ellipsoid[15] =Ellipsoid( 15, "Krassovsky", 6378245, 0.006693422);
        ellipsoid[16] =Ellipsoid( 16, "Modified Airy", 6377340, 0.00667054);
        ellipsoid[17] =Ellipsoid( 17, "Modified Everest", 6377304, 0.006637847);
        ellipsoid[18] = Ellipsoid( 18, "Modified Fischer 1960", 6378155, 0.006693422);
        ellipsoid[19] =Ellipsoid( 19, "South American 1969", 6378160, 0.006694542);
        ellipsoid[20] =Ellipsoid( 20, "WGS 60", 6378165, 0.006693422);
        ellipsoid[21] =Ellipsoid( 21, "WGS 66", 6378145, 0.006694542);
        ellipsoid[22] =Ellipsoid( 22, "WGS-72", 6378135, 0.006694318);
        ellipsoid[23] =Ellipsoid( 23, "WGS-84", 6378137, 0.00669438);

        double a = ellipsoid[ReferenceEllipsoid].EquatorialRadius;
        double eccSquared = ellipsoid[ReferenceEllipsoid].eccentricitySquared;
        double k0 = 0.9996;

        double LongOrigin;
        double eccPrimeSquared;
        double N, T, C, A, M;

        //Make sure the longitude is between -180.00 .. 179.9
        double LongTemp = (Long + 180) - (int)((Long + 180) / 360) * 360 - 180; // -180.00 .. 179.9;

        double LatRad = Lat * DEG2RAD;
        double LongRad = LongTemp * DEG2RAD;
        double LongOriginRad;
        int ZoneNumber;
        ZoneNumber = (int)((LongTemp + 180) / 6) + 1;

        if (Lat >= 56.0 && Lat < 64.0 && LongTemp >= 3.0 && LongTemp < 12.0)
            ZoneNumber = 32;

        // Special zones for Svalbard
        if (Lat >= 72.0 && Lat < 84.0){
            if (LongTemp >= 0.0 && LongTemp < 9.0) ZoneNumber = 31;
            else if (LongTemp >= 9.0 && LongTemp < 21.0) ZoneNumber = 33;
            else if (LongTemp >= 21.0 && LongTemp < 33.0) ZoneNumber = 35;
            else if (LongTemp >= 33.0 && LongTemp < 42.0) ZoneNumber = 37;
        }
        LongOrigin = (ZoneNumber - 1) * 6 - 180 + 3;  //+3 puts origin in middle of zone
        LongOriginRad = LongOrigin * DEG2RAD;

        //compute the UTM Zone from the latitude and longitude
        //sprintf(UTMZone, "%d%c", ZoneNumber, UTMLetterDesignator(Lat));
        char UTMZoneChar[100];
        sprintf(UTMZoneChar, "%d", ZoneNumber);
        UTMZone = std::string(UTMZoneChar);

        //UTMZone = ZoneNumber.ToString();
        eccPrimeSquared = (eccSquared) / (1 - eccSquared);

        N = a / sqrt(1 - eccSquared * sin(LatRad) * sin(LatRad));
        T = tan(LatRad) * tan(LatRad);
        C = eccPrimeSquared * cos(LatRad) * cos(LatRad);
        A = cos(LatRad) * (LongRad - LongOriginRad);

        M = a * ((1 - eccSquared / 4 - 3 * eccSquared * eccSquared / 64 - 5 * eccSquared * eccSquared * eccSquared / 256) * LatRad
                 - (3 * eccSquared / 8 + 3 * eccSquared * eccSquared / 32 + 45 * eccSquared * eccSquared * eccSquared / 1024) * sin(2 * LatRad)
                 + (15 * eccSquared * eccSquared / 256 + 45 * eccSquared * eccSquared * eccSquared / 1024) * sin(4 * LatRad)
                 - (35 * eccSquared * eccSquared * eccSquared / 3072) * sin(6 * LatRad));

        UTMEasting = (double)(k0 * N * (A + (1 - T + C) * A * A * A / 6
                                        + (5 - 18 * T + T * T + 72 * C - 58 * eccPrimeSquared) * A * A * A * A * A / 120)
                              + 500000.0);

        UTMNorthing = (double)(k0 * (M + N * tan(LatRad) * (A * A / 2 + (5 - T + 9 * C + 4 * C * C) * A * A * A * A / 24
                                                            + (61 - 58 * T + T * T + 600 * C - 330 * eccPrimeSquared) * A * A * A * A * A * A / 720)));
        if (Lat < 0)
            UTMNorthing += 10000000.0; //10000000 meter offset for southern hemisphere
    }

    /**
     *\brief Convert UTM coordinates into lat/long coordinates (the result will be stored in the last 2 parameters)
     */
    static void UTMtoLL(int ReferenceEllipsoid, double UTMNorthing, double UTMEasting, std::string UTMZone,
                        double & Lat,  double & Long )
    {
        //converts UTM coords to lat/long.  Equations from USGS Bulletin 1532
        //East Longitudes are positive, West longitudes are negative.
        //North latitudes are positive, South latitudes are negative
        //Lat and Long are in decimal degrees.
        //Written by Chuck Gantz- chuck.gantz@globalstar.com
        double k0 = 0.9996;
        double a = Cons::ellipsoid[ReferenceEllipsoid].EquatorialRadius;
        double eccSquared = Cons::ellipsoid[ReferenceEllipsoid].eccentricitySquared;
        double eccPrimeSquared;
        double e1 = (1- sqrt(1-eccSquared))/(1+sqrt(1-eccSquared));
        double N1, T1, C1, R1, D, M;
        double LongOrigin;
        double mu, phi1Rad;
        double x, y;
        int ZoneNumber;
        x = UTMEasting - 500000.0; //remove 500,000 meter offset for longitude
        y = UTMNorthing;

        //ZoneNumber = System.Convert.ToUInt16(UTMZone);
        ZoneNumber = atoi(UTMZone.c_str());

        LongOrigin = (ZoneNumber - 1)*6 - 180 + 3;  //+3 puts origin in middle of zone

        eccPrimeSquared = (eccSquared)/(1-eccSquared);
        M = y / k0;
        mu = M/(a*(1-eccSquared/4-3*eccSquared*eccSquared/64-5*eccSquared*eccSquared*eccSquared/256));

        phi1Rad = mu	+ ((3*e1/2-27*e1*e1*e1/32)*sin(2*mu) ) // fp
                + ((21*e1*e1/16-55*e1*e1*e1*e1/32)*sin(4*mu))
                +((151*e1*e1*e1/96)*sin(6*mu))
                + ((1094*e1*e1*e1*e1/512)* sin(8*mu));
        N1 = a/sqrt(1-eccSquared*sin(phi1Rad)*sin(phi1Rad));
        T1 = tan(phi1Rad)*tan(phi1Rad);
        C1 = eccPrimeSquared*cos(phi1Rad)*cos(phi1Rad);
        R1 = a*(1-eccSquared)/pow(1-eccSquared*sin(phi1Rad)*sin(phi1Rad), 1.5);
        D = x/(N1*k0);

        Lat = phi1Rad - (N1*tan(phi1Rad)/R1)*(D*D/2-(5+3*T1+10*C1-4*C1*C1-9*eccPrimeSquared)*D*D*D*D/24
                                              +(61+90*T1+298*C1+45*T1*T1-252*eccPrimeSquared-3*C1*C1)*D*D*D*D*D*D/720);
        Lat = Lat * RAD2DEG;

        Long = (D-(1+2*T1+C1)*D*D*D/6+(5-2*C1+28*T1-3*C1*C1+8*eccPrimeSquared+24*T1*T1)
                *D*D*D*D*D/120)/cos(phi1Rad);
        Long = LongOrigin + Long * RAD2DEG;
    }

    static void myUTM2LL(double UTMEasting, double UTMNorthing, std::string UTMZone, double &lat, double &lon)
    {
        double x = UTMEasting - 500000;
        double y = UTMNorthing;
        double k0 = 0.9996;
        double e2 = Cons::ellipsoid[23].eccentricitySquared;
        double a = Cons::ellipsoid[23].EquatorialRadius;
        /*
        lat = latitude of point
        long = longitude of point
        long0 = central meridian of zone
        k0  = scale along long0 = 0.9996
        e = SQRT(1-b2/a2) = .08 approximately. This is the eccentricity of the earth's elliptical cross-section.
        e'2 = (ea/b)2 = e2/(1-e2) = .007 approximately. The quantity e' only occurs in even powers so it need only be calculated as e'2.
        n = (a-b)/(a+b)
        rho = a(1-e2)/(1-e2sin2(lat))3/2. This is the radius of curvature of the earth in the meridian plane.
        nu = a/(1-e2sin2(lat))1/2. This is the radius of curvature of the earth perpendicular to the meridian plane. It is also the distance from the point in question to the polar axis, measured perpendicular to the earth's surface.
        p = (long-long0)
        sin1" = sine of one second of arc = pi/(180*60*60) = 4.8481368 x 10-6.
        */
        int ZoneNumber = atoi(UTMZone.c_str());
        double long0 = (ZoneNumber - 1) * 6 - 180 + 3;

        double M = y / k0;
        double mu = M / (a*(1 - e2/4 - 3*e2*e2/64 - 5*e2*e2*e2/256));
        double e1 = (1 - sqrt(1-e2)) / (1 + sqrt(1-e2));

        double J1 = (3*e1/2 - 27 * pow(e1,3 )/32);
        double J2 = 21 * pow(e1,2)/16 - 55* pow(e1,4)/32;
        double J3 = 151* pow(e1,3)/96;
        double J4 = 1097 * pow(e1,4)/512;

        double fp = mu + J1* sin(2*mu) + J2 * sin(4*mu) + J3* sin(6*mu) + J4 * sin(8*mu);

        double ei2 = e2 / (1 - e2);
        double C1 = ei2 * pow(cos(fp),2);
        double T1 = pow(tan(fp),2);
        double R1 = a*(1-e2)/ pow((1-e2*pow(sin(fp),2)),1.5);
        double N1 = a / sqrt(1 - e2 * pow(sin(fp), 2));
        double D = x / (N1 * k0);

        double Q1 = N1 * tan(fp)/R1;
        double Q2 = pow(D,2)/2;
        double Q3 = (5 + 3*T1 + 10*C1 - 4* pow(C1,2) - 9 * ei2)* pow(D,4) /24;
        double Q4 = (61 + 90*T1 + 298*C1 + 45*pow(T1,2) - 3* pow(C1,2) -252*ei2)* pow(D,6)/720;

        lat = fp - Q1*(Q2 - Q3 + Q4);
        lat = lat * RAD2DEG;

        double Q5 = D;
        double Q6 = (1 + 2*T1 +C1)*pow(D,3)/6;
        double Q7 = (5 - 2*C1 + 28*T1 -3*pow(C1,2) + 8* ei2 + 24* pow(T1,2)) * pow(D,5)/120;

        lon = (Q5 - Q6 + Q7)/cos(fp);
        lon = lon * RAD2DEG;
        lon = lon + long0;
    }
};


Coordinate::Coordinate()
{
    hash=0;
}

Coordinate::Coordinate(double lat, double lon)
{
    latitude=lat;
    longitude=lon;
    //double UTMNorthing, UTMEasting;
    //std::string UTMzone;
    //check latitude and longitude
    if (lat < -90 || lat > 90){
        NS_LOG_WARN("Coordinate.CreateFromLatLong: Latitude wrong format...");
        std::string err = "Coordinate.CreateFromLatLong: Latitude wrong format";
        throw CoordinateException(err);
    }
    if (lon < -180 || lon > 180){
        NS_LOG_WARN("Coordinate.CreateFromLatLong: Longitude wrong format...");
        std::string err = "Coordinate.CreateFromLatLong: Longitude wrong format";
        throw CoordinateException(err);
    }
    Conv::LLtoUTM(23, lat, lon, UTMNorthing, UTMEasting, UTMzone);
    //Coordinate(lat, lon, UTMNorthing, UTMEasting, UTMzone);
    hash = (int)((latitude + 90) * 100000) ^ (int)((longitude + 180) * 100000); 
}
    
Coordinate::Coordinate(double latitudeP, double longitudeP, double UTMNorthingP, double UTMEastingP, std::string UTMzoneP)
{
    latitude = latitudeP;
    longitude = longitudeP;
    UTMNorthing = UTMNorthingP;
    UTMEasting = UTMEastingP;
    UTMzone = UTMzoneP;
    hash = (int)((latitude + 90) * 100000) ^ (int)((longitude + 180) * 100000); 
}    
        
Coordinate::~Coordinate()
{       
}

void Coordinate::updateUTM()
{
    Conv::LLtoUTM(23, latitude, longitude, UTMNorthing, UTMEasting, UTMzone);
}


double Coordinate::asinSafe(double x)
{
    return asin(std::max(-1.0, std::min(x, 1.0)));
}

//great circle formulae
double Coordinate::greatCircleFormula(double gpsLat, double gpsLongitude)
{
    return 2 * asinSafe(sqrt(pow((sin((gpsLat - latitude) / 2)), 2) +
                             cos(gpsLat) * cos(latitude) * pow((sin((gpsLongitude - longitude) / 2)), 2)));
}


double Coordinate::haversineFormula(double gpsLat, double gpsLongitude) const
{
    double dlong = (gpsLongitude - longitude) * DEG2RAD;
    double dlat = (gpsLat - latitude) * DEG2RAD;
    double a = pow(sin(dlat / 2.0), 2) + cos(latitude * DEG2RAD) * cos(gpsLat * DEG2RAD) * pow(sin(dlong / 2.0), 2);
    double c = 2 * atan2(sqrt(a), sqrt(1 - a));
    return 1000 * 6367 * c;
}

double Coordinate::getDistance(double gpsLat, double gpsLongitude) const
{
    return haversineFormula(gpsLat, gpsLongitude);
}


double Coordinate::getDistance(Coordinate gps) const
{
    return getDistance(gps.getLatitude(), gps.getLongitude());
}

double Coordinate::twoPointsDistance(const Coordinate coord1) const
{
    return getDistance(coord1);
    /*double xSquare = pow((coord1.UTMNorthing - UTMNorthing), 2);
    //std::cout<<"UTMNorthing "<<UTMNorthing<<"||"<<coord1.UTMNorthing<<" result "<<xSquare<<"\n";
    double ySquare = pow((coord1.UTMEasting - UTMEasting), 2);
    //std::cout<<"UTMEasting "<<UTMEasting<<"||"<<coord1.UTMEasting<<" result "<<ySquare<<"\n";
    return sqrt((xSquare + ySquare));*/
}


double Coordinate::twoPointsDistance(double coord1_X, double coord1_Y) const
{
    return getDistance(coord1_X, coord1_Y);
}

double Coordinate::pointToSegmentDist(Coordinate start, Coordinate end) const
{
    if ((UTMzone.compare(start.UTMzone) ==0) && (UTMzone.compare(end.UTMzone)==0)){
        return pointToSegmentDistUTM(start, end);
    }
    else{
        return pointToSegmentDistENU(start, end);
    }
}
    
//TODO check who is using whis function. they shoul use  the func above
double Coordinate::pointToSegmentDistUTM(Coordinate lineStart, Coordinate lineEnd) const
{
    // distance between myself and a segment
    // point == current Position in UTM
    // lineStart and lineEnd == the two points that define a line
    if(lineStart.isEqual(lineEnd)){
        return 0;
    }
    
    double dist = linePointDist(lineStart.UTMEasting,lineStart.UTMNorthing, lineEnd.UTMEasting, lineEnd.UTMNorthing, UTMEasting,UTMNorthing);
    return dist;
    
    /*
    this is the formuala used by the original code. Sometimes it shows lack of precision
    
    double check, mag, U, x, y;
    mag=pow(lineStart.UTMNorthing - lineEnd.UTMNorthing, 2) + pow(lineStart.UTMEasting - lineEnd.UTMEasting, 2);
    U=(((UTMNorthing - lineStart.UTMNorthing) * (lineEnd.UTMNorthing - lineStart.UTMNorthing)) + 
    ((UTMEasting - lineStart.UTMEasting) * (lineEnd.UTMEasting - lineStart.UTMEasting))) / mag;
    if(U < 0.0 || U > 1.0){
        return std::min(twoPointsDistance(lineStart), twoPointsDistance(lineEnd));
    }
    else{
        x=lineStart.UTMNorthing + U * (lineEnd.UTMNorthing - lineStart.UTMNorthing);
        y=lineStart.UTMEasting + U * (lineEnd.UTMEasting - lineStart.UTMEasting);
        check=twoPointsDistanceUTM(x, y);
    }
    return check;   
     */
}
    
double Coordinate::linePointDist(double xA, double yA, double xB, double yB, double xC, double yC) const
{
    if (dot(xA, yA, xB, yB, xC, yC) > 0) {
        return distance(xB, yB, xC, yC);
    } else if (dot(xB, yB, xA, yA, xC, yC) > 0) {
        return distance(xA, yA, xC, yC);
    } else {
        double dist = cross(xA, yA, xB, yB, xC, yC) / distance(xA, yA, xB, yB);
        if (dist < 0) {
            dist = -dist;
        }
        return dist;
    }
}

double Coordinate::distance(double xA, double yA, double xB, double yB) const
{
    return sqrt(pow(xA - xB, 2) + pow(yA - yB, 2));
}
    
double Coordinate::dot(double xA, double yA, double xB, double yB, double xC, double yC) const
{
    double xAB = xB - xA;
    double yAB = yB - yA;
    double xBC = xC - xB;
    double yBC = yC - yB;
    double dot = xAB * xBC + yAB * yBC;
    return dot;
}

double Coordinate::cross(double xA, double yA, double xB, double yB, double xC, double yC) const
{
    double xAB = xB - xA;
    double yAB = yB - yA;
    double xAC = xC - xA;
    double yAC = yC - yA;
    double cross = xAB * yAC - yAB * xAC;
    return cross;
}

double Coordinate::pointToSegmentDistENU(Coordinate lineStart, Coordinate lineEnd) const
{
    // distance between myself and a segment
    // point == current Position in ENU
    // lineStart and lineEnd == the two points that define a line
 /*   if(lineStart.isEqual(lineEnd)){
        return 0;
    }
    double check;
    double enuXpoint, enuXstart, enuXend, enuYpoint, enuYstart, enuYend, enuZpoint, enuZstart, enuZend;
    //TODO: set origin
    ConvertCoordinate.WS84ToEnuPrecise(point.latitude, point.longitude, out enuXpoint, out enuYpoint, out enuZpoint);
    ConvertCoordinate.WS84ToEnuPrecise(lineStart.latitude, lineStart.longitude, out enuXstart, out enuYstart, out enuZstart);
    ConvertCoordinate.WS84ToEnuPrecise(lineEnd.latitude, lineEnd.longitude, out enuXend, out enuYend, out enuZend);
    
    double mag = Math.Pow(enuXstart - enuXend, 2) +
    Math.Pow(enuYstart - enuYend, 2);
    
    double U = (((enuXpoint - enuXstart) * (enuXend - enuXstart)) +
                ((enuYpoint - enuYstart) * (enuYend - enuYstart))) / mag;
    
    //TODO: porting comment(of previous author): the formula is not clear
    if (U < 0.0 || U > 1.0)
        return Math.Min(Coordinate.TwoPointsDistanceENU(point, lineStart), Coordinate.TwoPointsDistanceENU(point, lineEnd));
    else
    {
        double x1 = enuXstart + U * (enuXend - enuXstart);
        double y1 = enuYstart + U * (enuYend - enuYstart);
        
        check = Coordinate.TwoPointsDistance(enuXpoint, enuYpoint, x1, y1);
    }
    return check;*/
    return 0;
}
    
 
double Coordinate::twoPointsDistanceUTM(double northing, double easting) const
{
    //double xSquare = pow((coord1_X - latitude), 2);
    double xSquare = pow((northing - UTMNorthing), 2);
    //std::cout<<"UTMNorthing "<<UTMNorthing<<"||"<<coord1_X<<" result "<<xSquare<<"\n";
    double ySquare = pow((easting - UTMEasting), 2);
    //std::cout<<"UTMEasting "<<UTMEasting<<"||"<<coord1_Y<<" result "<<ySquare<<"\n";
    return sqrt((xSquare + ySquare));
}

std::string Coordinate::toString()
{
    std::stringstream out;
    //out << "(";
    out << std::setprecision(15) << latitude;
    out << ",";
    out << std::setprecision(15) << longitude;
    //out << ")";
    return out.str();
}

double Coordinate::getDistance(std::string coordinateFormat)
{
    double lat = 0.0, lng = 0.0;
    boost::char_separator<char> sep(",");
    boost::tokenizer< boost::char_separator<char> > tok(coordinateFormat, sep);
    boost::tokenizer< boost::char_separator<char> >::iterator beg = tok.begin();

    // TODO: handle malformed strings

    if(beg!=tok.end()){
        std::stringstream ss(*beg); //turn the string into a stream
        ss >> lat;
    }
    beg++;
    if(beg!=tok.end()){
        std::stringstream ss(*beg); //turn the string into a stream
        ss >> lng;
    }

    NS_LOG_DEBUG("Parsing " << coordinateFormat << " -> lat: "<< lat << ", long: " << lng);
    return getDistance(lat, lng);
}

}
}
