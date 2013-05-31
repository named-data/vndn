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

#include <iostream>
#include <string>
#include <syslog.h>

#include "corelib/singleton.h"
#include "helper/event-monitor.h"
#include "app-connector.h"
#include "ndn-l3-protocol.h"
#include "ndn-fib.h"
#include "ndn-flooding-strategy.h"
#include "ndn-adhoc-net-device-face.h"
#include "ndn-hub-over-ip-device-face.h"
#include "ndn-net-device-face.h"
#include "ndn-management.h"

using namespace vndn;
using std::cerr;
using std::cout;
using std::endl;
using std::string;


static void usage()
{
    cout << "Usage: ./ndnd <type-of-face> <interface-name or ip-address>\n"
         << "Available interface types: hub (local ip), adhoc (device name), net (local ip and hub ip)\n"
         << "Example: ./ndnd adhoc wlan0 hub 10.0.0.1\n";
}

/**
 * params:
 * type of face - name or ip address
 * Types of faces:
 *      hub: ndn-hub-face - local ip address
 *      adhoc: ad-hoc-face (interface name is required)
 *      net: net-device-face - local ip address  + ?hub ip?
 */
// TODO net device: needs hub ip

int main(int argc, char **argv)
{
    if (argc < 2) {
        cerr << "Error: invalid number of arguments." << endl;
        usage();
        return -1;
    }

    ::openlog("ndnd", LOG_ODELAY | LOG_PID, LOG_LOCAL6);

    Ptr<AppConnector> appConn = Create<AppConnector>();

    NDNL3Protocol *protocol = Singleton<NDNL3Protocol>::Get();
    protocol->SetForwardingStrategy(Create<NDNFloodingStrategy>());
    Ptr<NDNFib> fib = Create<NDNFib>();
    protocol->SetFib(fib);

    NDNManagementInterface manager;

    EventMonitor em;
    em.add(appConn);
    em.addTimer(&NDNL3Protocol::Reap, &em, &NDNL3Protocol::REAP_INTERVAL);

    for (int i = 1; i < argc; i++) {
        Ptr<NDNFace> face;
        string arg(argv[i]);
        if (arg.compare("hub") == 0) {
            i++; // consume one more argument (local ip)
            string ip(argv[i]);
            cout << "Creating hub face with IP address" << ip << endl;
            try {
                face = Create<NDNHubOverIPDeviceFace>(ip);
            } catch (const char *e) {
                cerr << "Failed to create NDNHubOverIPDeviceFace: " << e << endl;
                continue;
            }
        } else if (arg.compare("adhoc") == 0) {
            i++; // consume one more argument (device name)
            string dev(argv[i]);
            cout << "Creating adhoc face on interface " << dev << endl;
            try {
                face = Create<NDNAdhocNetDeviceFace>(dev);
            } catch (NDNLinkLayerCommunication e) {
                cerr << "Failed to create NDNAdhocNetDeviceFace: " << e.error() << endl;
                continue;
            }
        } else if (arg.compare("net") == 0) {
            i++; // consume one more argument (local ip)
            string ip(argv[i]);
            i++; // consume one more argument (hub ip)
            string hub(argv[i]);
            cout << "Creating net face with IP address " << ip << " and hub address " << hub << endl;
            try {
                face = Create<NDNNetDeviceFace>(ip, hub);
            } catch (const char *e) {
                cerr << "Failed to create NDNNetDeviceFace: " << e << endl;
                continue;
            }
        } else {
            cerr << "Error: unknown argument '" << arg << "'" << endl;
            usage();
            return -1;
        }
        protocol->AddFace(face);
        em.add(face);
    }

    // Start monitoring
    em.monitor();

    return 0;
}
