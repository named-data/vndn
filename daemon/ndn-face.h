/*
 * Copyright (c) 2011 University of California, Los Angeles
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Authors: Alexander Afanasyev <alexander.afanasyev@ucla.edu>
 */

#ifndef NDN_FACE_H
#define NDN_FACE_H

#include <algorithm>
#include <ostream>
#include <stdint.h>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/ref.hpp>

#include "corelib/ptr.h"
#include "helper/monitorable.h"
#include "network/request-source-info.h"


namespace vndn
{

class EventMonitor;
class Packet;

/**
 * \ingroup ndn
 * \defgroup ndn-face Faces
 */
/**
 * \ingroup ndn-face
 * \brief Virtual class defining NDN face
 *
 * This class defines basic functionality of NDN face. Face is core
 * component responsible for actual delivery of data packet to and
 * from NDN stack
 *
 * \see NDNLocalFace, NDNNetDeviceFace, NDNIpv4Face, NDNUdpFace
 */
class NDNFace : public Monitorable
{
public:
    /**
     * \brief Default constructor
     */
    NDNFace();
    virtual ~NDNFace();

    /**
     * @brief Check if Interest limit is reached
     *
     * Side effect: if limit is not yet reached, the number of outstanding packets will be increased
     *
     * @returns true if Interest limit is not yet reached
     */
    virtual bool IsBelowLimit();

    /**
     * \brief Send packet on a face
     *
     * This method will be called by lower layers to send data to device or application
     *
     * \param p smart pointer to a packet to send
     *
     * @return false if either limit is reached
     */
    virtual bool Send(const Ptr<const Packet> &p);


    /**
     * \brief Send packet on a Face, plus some metadata
     *
     * If not implemented, it behaves as Send(packet), ignoring metadata
     * \param p smart pointer to a packet to send
     * \param metadata metadata for the face. Ignored in this implementation
     *
     * @return false if either limit is reached
     * */
    virtual bool Send(const Ptr<const Packet> &p, RequestSourceInfo *metadata);

    virtual int getMonitorFd() const;
    virtual void exceptHandler();
    virtual void readHandler(EventMonitor &em);

    /**
     * \brief Receive packet from application or another node and forward it to the NDN stack
     *
     * \todo The only reason for this call is to handle tracing, if requested
     */
    virtual bool Receive(const Ptr<const Packet> &p);

    /**
     * \Brief Assign routing/forwarding metric with face
     *
     * \param metric configured routing metric (cost) of this face
     */
    virtual void SetMetric(uint16_t metric);

    /**
     * \brief Get routing/forwarding metric assigned to the face
     *
     * \returns configured routing/forwarding metric (cost) of this face
     */
    virtual uint16_t GetMetric() const;

    virtual std::ostream &Print(std::ostream &os) const;

    /**
     * @brief Set maximum value for Interest allowance
     *
     * @param bucket maximum value for Interest allowance. If < 0, then limit will be disabled
     */
    void SetBucketMax(double bucket);

    /**
     * @brief Set a normalized value (one second) for Interest allowance bucket leak
     */
    void SetBucketLeak(double leak);

    /**
     * @brief Leak the Interest allowance bucket by (1/interval) * m_bucketMax amount,
     * where interval is time between two consecutive calls of LeakBucket
     */
    void LeakBucket();

    /**
     * \brief Compare two faces. Only two faces on the same node could be compared.
     *
     * Internal index is used for comparison.
     */
    bool operator==(const NDNFace &face) const;

    /**
     * \brief Compare two faces. Only two faces on the same node could be compared.
     *
     * Internal index is used for comparison.
     */
    bool operator<(const NDNFace &face) const;


private:
    NDNFace(const NDNFace &); ///< \brief Disabled copy constructor
    NDNFace &operator=(const NDNFace &); ///< \brief Disabled copy operator

protected:
    int m_app_fd;
    // uint16_t m_metric; ///< \brief Routing/forwarding metric
    double m_bucket;     ///< \brief Value representing current size of the Interest allowance for this face
    double m_bucketMax;  ///< \brief Maximum Interest allowance for this face
    double m_bucketLeak; ///< \brief Normalized amount that should be leaked every second

private:
    boost::posix_time::ptime m_lastLeakTime;
    uint32_t m_metric; ///< \brief metric of the face
};

std::ostream &operator<<(std::ostream &os, const NDNFace &face);

inline bool operator<(const Ptr<NDNFace> &lhs, const Ptr<NDNFace> &rhs)
{
    return *lhs < *rhs;
}

} // namespace vndn

#endif /* NDN_FACE_H */
