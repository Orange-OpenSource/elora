/*
 * Copyright (c) 2017 University of Padova
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Davide Magrin <magrinda@dei.unipd.it>
 *
 * 23/12/2022
 * Modified by: Alessandro Aimi <alessandro.aimi@orange.com>
 *                              <alessandro.aimi@cnam.fr>
 */

#ifndef PERIODIC_SENDER_HELPER_H
#define PERIODIC_SENDER_HELPER_H

#include "ns3/address.h"
#include "ns3/application-container.h"
#include "ns3/attribute.h"
#include "ns3/net-device.h"
#include "ns3/node-container.h"
#include "ns3/object-factory.h"
#include "ns3/periodic-sender.h"

#include <stdint.h>
#include <string>

namespace ns3
{
namespace lorawan
{

/**
 * This class can be used to install PeriodicSender applications on a wide
 * range of nodes.
 */
class PeriodicSenderHelper
{
  public:
    PeriodicSenderHelper();

    ~PeriodicSenderHelper();

    void SetAttribute(std::string name, const AttributeValue& value);

    ApplicationContainer Install(NodeContainer c) const;

    ApplicationContainer Install(Ptr<Node> node) const;

    /**
     * Set the period to be used by the applications created by this helper.
     *
     * A value of Seconds (0) results in randomly generated periods according to
     * the model contained in the TR 45.820 document.
     *
     * \param period The period to set
     */
    void SetPeriod(Time period);

    // Extract different constant period for each device from a distribution
    void SetPeriodGenerator(Ptr<RandomVariableStream> rv);

    void SetPacketSize(uint8_t size);

    // Extract different constant packet size for each device from a distribution
    void SetPacketSizeGenerator(Ptr<RandomVariableStream> rv);

  private:
    Ptr<Application> InstallPriv(Ptr<Node> node) const;

    ObjectFactory m_factory;

    Ptr<UniformRandomVariable> m_initialDelay;

    Ptr<UniformRandomVariable> m_intervalProb;

    Time m_period; //!< The period with which the application will be set to send
                   // messages

    uint8_t m_pktSize; // the packet size.

    Ptr<RandomVariableStream> m_intervalGenerator; //!< the sending interval assignment variable
    Ptr<RandomVariableStream> m_sizeGenerator;     //!< the packet size assignment variable
};

} // namespace lorawan

} // namespace ns3
#endif /* PERIODIC_SENDER_HELPER_H */
