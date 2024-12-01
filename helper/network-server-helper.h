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

#ifndef NETWORK_SERVER_HELPER_H
#define NETWORK_SERVER_HELPER_H

#include "ns3/address.h"
#include "ns3/application-container.h"
#include "ns3/attribute.h"
#include "ns3/net-device.h"
#include "ns3/network-server.h"
#include "ns3/node-container.h"
#include "ns3/object-factory.h"
#include "ns3/point-to-point-helper.h"

#include <stdint.h>
#include <string>

namespace ns3
{
namespace lorawan
{

/**
 * This class can install Network Server applications on multiple nodes at once.
 */
class NetworkServerHelper
{
  public:
    NetworkServerHelper();

    ~NetworkServerHelper();

    void SetAttribute(std::string name, const AttributeValue& value);

    ApplicationContainer Install(NodeContainer c);

    ApplicationContainer Install(Ptr<Node> node);

    /**
     * Set which end devices will be managed by this NS.
     */
    void SetEndDevices(NodeContainer endDevices);

    /**
     * Enable (true) or disable (false) the ADR component in the Network
     * Server created by this helper.
     */
    void EnableAdr(bool enableAdr);

    /**
     * Set the ADR implementation to use in the Network Server created
     * by this helper.
     */
    void SetAdr(std::string type);

  private:
    void InstallComponents(Ptr<NetworkServer> netServer);
    Ptr<Application> InstallPriv(Ptr<Node> node);

    ObjectFactory m_factory;

    NodeContainer m_endDevices; //!< Set of endDevices to connect to this NS

    bool m_adrEnabled;

    ObjectFactory m_adrSupportFactory;
};

} // namespace lorawan

} // namespace ns3
#endif /* NETWORK_SERVER_HELPER_H */
