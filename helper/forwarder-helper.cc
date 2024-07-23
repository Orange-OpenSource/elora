/*
 * Copyright (c) 2017 University of Padova
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Davide Magrin <magrinda@dei.unipd.it>
 *
 * 17/01/2023
 * Modified by: Alessandro Aimi <alessandro.aimi@orange.com>
 *                              <alessandro.aimi@cnam.fr>
 */

#include "forwarder-helper.h"

#include "ns3/double.h"
#include "ns3/forwarder.h"
#include "ns3/log.h"
#include "ns3/lora-net-device.h"
#include "ns3/random-variable-stream.h"
#include "ns3/simulator.h"
#include "ns3/string.h"
#include "ns3/trace-source-accessor.h"

namespace ns3
{
namespace lorawan
{

NS_LOG_COMPONENT_DEFINE("ForwarderHelper");

ForwarderHelper::ForwarderHelper()
{
    m_factory.SetTypeId("ns3::Forwarder");
}

ForwarderHelper::~ForwarderHelper()
{
}

void
ForwarderHelper::SetAttribute(std::string name, const AttributeValue& value)
{
    m_factory.Set(name, value);
}

ApplicationContainer
ForwarderHelper::Install(Ptr<Node> node) const
{
    return ApplicationContainer(InstallPriv(node));
}

ApplicationContainer
ForwarderHelper::Install(NodeContainer c) const
{
    ApplicationContainer apps;
    for (auto i = c.Begin(); i != c.End(); ++i)
    {
        apps.Add(InstallPriv(*i));
    }

    return apps;
}

Ptr<Application>
ForwarderHelper::InstallPriv(Ptr<Node> node) const
{
    NS_LOG_FUNCTION(this << node);

    Ptr<Forwarder> app = m_factory.Create<Forwarder>();

    app->SetNode(node);
    node->AddApplication(app);

    // Link the Forwarder to the NetDevices
    for (uint32_t i = 0; i < node->GetNDevices(); i++)
    {
        Ptr<NetDevice> currNetDev = node->GetDevice(i);
        if (auto loraNetDev = DynamicCast<LoraNetDevice>(currNetDev); loraNetDev != nullptr)
        {
            auto mac = DynamicCast<GatewayLorawanMac>(loraNetDev->GetMac());
            NS_ASSERT(bool(mac));
            app->SetGatewayLorawanMac(mac);
            mac->SetReceiveCallback(MakeCallback(&Forwarder::ReceiveFromLora, app));
        }
        else if (auto p2pNetDev = DynamicCast<PointToPointNetDevice>(currNetDev);
                 p2pNetDev != nullptr)
        {
            app->SetPointToPointNetDevice(p2pNetDev);
            p2pNetDev->SetReceiveCallback(MakeCallback(&Forwarder::ReceiveFromPointToPoint, app));
        }
        else
        {
            NS_LOG_ERROR("Potential error: NetDevice is neither Lora nor PointToPoint");
        }
    }

    return app;
}
} // namespace lorawan
} // namespace ns3
