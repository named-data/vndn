V-NDN: NDN for Vehicular Networks
=================================

V-NDN is a C++ implementation of NDN for vehicular networks.

V-NDN mainly focuses on vehicle-to-vehicle ad-hoc (IBSS) networks, where it can replace IP as a layer 3 protocol. But it can also use other wireless technologies, such as 802.11 infrastructure or WiMAX, on top of UDP/IP.

Building from git
-----------------

Software required to build V-NDN from git:

* a recent C++ compiler (both g++ and clang++ should work)
* autoconf 2.68 or later
* automake 1.11.x (later versions should work too)
* libtool (any recent version should work)
* boost 1.48 or later (with threading support enabled)
* libevent 2.0.x
* libyajl 2.0 or later

For example, Ubuntu 12.10 satisfies the above requirements and is regularly tested by us.

To build with GCC (default):
```
./bootstrap && make
```

To build with Clang:
```
./bootstrap CC=clang CXX=clang++ && make
```

Usage
-----

*NOTE: all programs must be run as root due to raw sockets usage.*

* **ndnd**: the main NDN daemon, must be running on every node. The list of network faces must be specified on the command line; available face types and their (required) arguments are:
	* `adhoc <interface-name>`
	* `hub <local-ip-address>`
	* `net <local-ip-address> <ndn-hub-ip-address>`

	For example, running `./ndnd adhoc wlan0 net 192.168.0.42 192.168.0.1` starts ndnd with 2 active faces: one ad-hoc face on the wlan0 wireless interface (on which you must have already configured an IBSS network), and one face over IP bound to the local address 192.168.0.42 and using the NDN hub at 192.168.0.1 (in this example the corresponding hub can be started on another machine with `./ndnd hub 192.168.0.1`).

* **trafficConsumer**: application that periodically issues interests for traffic information. Requires 2 arguments:
	* number of seconds to wait before retransmitting an unsatisfied interest (-1 disables all retransmissions)
	* number of seconds to wait before issuing a new interest after the previous one has been satisfied

* **trafficProducer**: application that produces traffic information. No arguments required.

* **photoConsumer**: application that periodically issues interests for road pictures. Requires 2 arguments:
	* number of seconds to wait before retransmitting an unsatisfied interest (-1 disables all retransmissions)
	* number of seconds to wait before issuing a new interest after the previous one has been satisfied

* **photoProducer**: application that takes pictures of the road. Requires 3 arguments:
	* unique ID of the car where the application is running
	* gpsd port number (usually 2947)
	* device used to take the pictures (e.g. /dev/video0)

Code documentation
------------------

The documentation can be generated with:
```
doxygen Doxyfile
```
Please note that this is still a work in progress, some parts of the documentation may be missing or incomplete.

References
----------

G. Grassi, D. Pesavento, L. Wang, G. Pau, R. Vuyyuru, R. Wakikawa, L. Zhang, **Vehicular Inter-Networking via Named Data**, in *ACM SIGMOBILE Mobile Computing and Communications Review*. [pending publication]

License
-------

V-NDN is released under the [GNU General Public License version 2](http://opensource.org/licenses/GPL-2.0). See the LICENSE file for more information.
