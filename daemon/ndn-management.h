/*
 * Copyright (c) 2013 Francesco Berni <kurojishi@gmail.com>
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

#include <pthread.h>
#include <boost/scoped_ptr.hpp>
#include <boost/interprocess/ipc/message_queue.hpp>
#include <boost/property_tree/ptree.hpp>


using boost::property_tree::ptree;
using namespace boost::interprocess;

namespace vndn
{

/**
 * \ingroup ndn
 * \brief Class that implement the prefix registration it's designed for easy future management and runtime configuration needs
 *
 * - This class when instanced spawn a thread and wait for json messages using boost message queue
 * - The ServiceType field in the json message identify the type of service that need to be served, just add an if in the InternalThreadEntry loop with the new Service. Other fields are service specific
 *
 * \see http://www.boost.org/doc/libs/1_48_0/doc/html/interprocess/synchronization_mechanisms.html#interprocess.synchronization_mechanisms.message_queue  for more information on Boost.Interprocess library
 */
class NDNManagementInterface
{
public:
    /**
     * \brief Public contructor spawn a posix thread and create the message_queue
     * */
    NDNManagementInterface();
    /**
     * \brief Public deconstructor kill the thread and clean the message_queue create for future nice death of the daemon
     */
    virtual ~NDNManagementInterface();


private:
    /**
     * \brief utility method used for starting the pthread nicely
     * */
    void InternalThreadEntry();

    /**
     * \brief Prefix registration, register a prefix/face couple withing the daemon
     * json message structure for PrefixRegistration:
     * - Action: identify if it's needed to delete or register a prefix, maintained the CCNx names for actions
     * - Name: is the prefix name that we want to modify
     * - FaceID: is the face that need to serve Interest for the Name prefix (generally the LocalFace of the producer)
     * \param boost property tree result of the parsing of the json message
     * */
    void prefixRegistration(ptree forwarding_entry);

    /**
     * \brief Function run in the thread, infinite loop that waits for messages
     * */
    static void * InternalThreadEntryFunc(void * This);

    /**
     * \brief Returns true if the thread was successfully started, false if there was an error starting the thread
     * */
    bool StartInternalThread();

    pthread_t _thread;
    boost::scoped_ptr<message_queue> management_queue;
};

}
