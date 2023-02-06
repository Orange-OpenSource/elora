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

#include "lora-application.h"

#include "ns3/lora-net-device.h"
#include "ns3/uinteger.h"

namespace ns3
{
namespace lorawan
{

NS_LOG_COMPONENT_DEFINE("LoraApplication");

NS_OBJECT_ENSURE_REGISTERED(LoraApplication);

TypeId
LoraApplication::GetTypeId(void)
{
    static TypeId tid =
        TypeId("ns3::LoraApplication")
            .SetParent<Application>()
            .AddConstructor<LoraApplication>()
            .SetGroupName("lorawan")
            .AddAttribute("Interval",
                          "The average time to wait between packets",
                          TimeValue(Seconds(600.0)),
                          MakeTimeAccessor(&LoraApplication::m_avgInterval),
                          MakeTimeChecker())
            .AddAttribute("PacketSize",
                          "Size of packets generated. The minimum packet size is 12 bytes which is "
                          "the size of the header carrying the sequence number and the time stamp.",
                          UintegerValue(18),
                          MakeUintegerAccessor(&LoraApplication::GetPacketSize,
                                               &LoraApplication::SetPacketSize),
                          MakeUintegerChecker<uint32_t>());
    return tid;
}

LoraApplication::LoraApplication()
{
    NS_LOG_FUNCTION(this);
}

LoraApplication::~LoraApplication()
{
    NS_LOG_FUNCTION(this);
}

void
LoraApplication::SetInterval(Time interval)
{
    NS_LOG_FUNCTION(this << interval);
    m_avgInterval = interval;
}

Time
LoraApplication::GetInterval(void) const
{
    NS_LOG_FUNCTION(this);
    return m_avgInterval;
}

void
LoraApplication::SetInitialDelay(Time delay)
{
    NS_LOG_FUNCTION(this << delay);
    m_initialDelay = delay;
}

void
LoraApplication::SetPacketSize(uint8_t size)
{
    NS_LOG_FUNCTION(this << (unsigned)size);
    m_basePktSize = size;
}

uint8_t
LoraApplication::GetPacketSize(void) const
{
    NS_LOG_FUNCTION(this);
    return m_basePktSize;
}

bool
LoraApplication::IsRunning(void)
{
    NS_LOG_FUNCTION(this);
    return m_sendEvent.IsRunning();
}

void
LoraApplication::DoInitialize()
{
    NS_LOG_FUNCTION(this);
    // Make sure we have a MAC layer
    if (bool(m_mac) == 0)
    {
        NS_ASSERT(m_node->GetNDevices() == 1);
        // Assumes there's only one device, force it to be an end device
        auto loraNetDevice = DynamicCast<LoraNetDevice>(m_node->GetDevice(0));
        m_mac = DynamicCast<EndDeviceLorawanMac>(loraNetDevice->GetMac());
        NS_ASSERT(bool(m_mac) != 0);
    }
    Application::DoInitialize();
}

// Protected methods
// StartApp, StopApp and Send will likely be overridden by lora application subclasses
void
LoraApplication::StartApplication(void)
{ // Provide null functionality in case subclass is not interested
    NS_LOG_FUNCTION(this);
}

void
LoraApplication::StopApplication(void)
{ // Provide null functionality in case subclass is not interested
    NS_LOG_FUNCTION_NOARGS();
}

void
LoraApplication::SendPacket(void)
{ // Provide null functionality in case subclass is not interested
    NS_LOG_FUNCTION(this);
}

} // namespace lorawan
} // namespace ns3
