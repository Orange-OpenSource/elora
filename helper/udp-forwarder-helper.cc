/*
 * Copyright (c) 2022 Orange SA
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Alessandro Aimi <alessandro.aimi@orange.com>
 *                         <alessandro.aimi@cnam.fr>
 */

#include "udp-forwarder-helper.h"

#include "ns3/double.h"
#include "ns3/log.h"
#include "ns3/lora-net-device.h"
#include "ns3/random-variable-stream.h"
#include "ns3/simulator.h"
#include "ns3/string.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/udp-forwarder.h"
#include "ns3/udp-socket-factory.h"

namespace ns3
{
namespace lorawan
{

NS_LOG_COMPONENT_DEFINE("UdpForwarderHelper");

UdpForwarderHelper::UdpForwarderHelper()
{
    m_factory.SetTypeId("ns3::UdpForwarder");
}

UdpForwarderHelper::~UdpForwarderHelper()
{
}

void
UdpForwarderHelper::SetAttribute(std::string name, const AttributeValue& value)
{
    m_factory.Set(name, value);
}

ApplicationContainer
UdpForwarderHelper::Install(Ptr<Node> node) const
{
    return ApplicationContainer(InstallPriv(node));
}

ApplicationContainer
UdpForwarderHelper::Install(NodeContainer c) const
{
    ApplicationContainer apps;
    for (auto i = c.Begin(); i != c.End(); ++i)
    {
        apps.Add(InstallPriv(*i));
    }
    return apps;
}

Ptr<Application>
UdpForwarderHelper::InstallPriv(Ptr<Node> node) const
{
    NS_LOG_FUNCTION(this << node);
    // Check if node supports UDP sockets
    NS_ASSERT_MSG(node->GetObject<UdpSocketFactory>(), "UDP protocol not installed on input node");
    Ptr<UdpForwarder> app = m_factory.Create<UdpForwarder>();
    app->SetNode(node);
    node->AddApplication(app);
    // Link the Forwarder to the GatewayLorawanMac
    bool foundLoraNetDevice = false;
    for (uint32_t i = 0; i < node->GetNDevices(); i++)
    {
        if (auto loraNetDev = DynamicCast<LoraNetDevice>(node->GetDevice(i)); loraNetDev)
        {
            auto mac = DynamicCast<GatewayLorawanMac>(loraNetDev->GetMac());
            NS_ASSERT_MSG(mac, "Gateway LoRaWAN MAC layer not found in LoraNetDevice");
            app->SetGatewayLorawanMac(mac);
            mac->SetReceiveCallback(MakeCallback(&UdpForwarder::ReceiveFromLora, app));
            foundLoraNetDevice = true;
            break;
        }
    }
    NS_ASSERT_MSG(foundLoraNetDevice, "LoraNetDevice not installed on input node");
    return app;
}

} // namespace lorawan
} // namespace ns3
