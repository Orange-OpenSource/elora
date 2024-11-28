/*
 * Copyright (c) 2022 Orange SA
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Alessandro Aimi <alessandro.aimi@orange.com>
 *                         <alessandro.aimi@cnam.fr>
 */

#ifndef UDP_FORWARDER_HELPER_H
#define UDP_FORWARDER_HELPER_H

#include "ns3/application-container.h"
#include "ns3/attribute.h"
#include "ns3/node-container.h"
#include "ns3/object-factory.h"

namespace ns3
{
namespace lorawan
{

/**
 * This class can be used to install UDP Forwarder applications on a set of
 * gateways.
 */
class UdpForwarderHelper
{
  public:
    UdpForwarderHelper();

    ~UdpForwarderHelper();

    void SetAttribute(std::string name, const AttributeValue& value);

    ApplicationContainer Install(NodeContainer c) const;

    ApplicationContainer Install(Ptr<Node> node) const;

  private:
    Ptr<Application> InstallPriv(Ptr<Node> node) const;

    ObjectFactory m_factory;
};

} // namespace lorawan
} // namespace ns3

#endif /* UDP_FORWARDER_HELPER_H */
