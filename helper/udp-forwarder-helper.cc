/*
 * Copyright (c) 2022 Orange SA
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
 * Author: Alessandro Aimi <alessandro.aimi@orange.com>
 *                         <alessandro.aimi@cnam.fr>
 */

#include "udp-forwarder-helper.h"

#include "ns3/csma-net-device.h"
#include "ns3/double.h"
#include "ns3/log.h"
#include "ns3/lora-net-device.h"
#include "ns3/random-variable-stream.h"
#include "ns3/simulator.h"
#include "ns3/string.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/udp-forwarder.h"

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
    for (NodeContainer::Iterator i = c.Begin(); i != c.End(); ++i)
    {
        apps.Add(InstallPriv(*i));
    }
    return apps;
}

Ptr<Application>
UdpForwarderHelper::InstallPriv(Ptr<Node> node) const
{
    NS_LOG_FUNCTION(this << node);
    Ptr<UdpForwarder> app = m_factory.Create<UdpForwarder>();
    app->SetNode(node);
    node->AddApplication(app);
    // Link the Forwarder to the NetDevice and GatewayLorawanMac
    for (uint32_t i = 0; i < node->GetNDevices(); i++)
    {
        Ptr<NetDevice> currNetDev = node->GetDevice(i);
        if (auto loraNetDev = DynamicCast<LoraNetDevice>(currNetDev); loraNetDev != nullptr)
        {
            auto mac = DynamicCast<GatewayLorawanMac>(loraNetDev->GetMac());
            NS_ASSERT(bool(mac));
            app->SetGatewayLorawanMac(mac);
            mac->SetReceiveCallback(MakeCallback(&UdpForwarder::ReceiveFromLora, app));
        }
        else if (DynamicCast<CsmaNetDevice>(currNetDev))
        {
            continue;
        }
        else
        {
            NS_LOG_ERROR("Potential error: NetDevice is neither Lora nor Csma");
        }
    }
    return app;
}

} // namespace lorawan
} // namespace ns3
