/*
 * Copyright (c) 2017 University of Padova
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
 * Author: Davide Magrin <magrinda@dei.unipd.it>
 *
 * 23/12/2022
 * Modified by: Alessandro Aimi <alessandro.aimi@orange.com>
 *                              <alessandro.aimi@cnam.fr>
 */

#include "ns3/gateway-lora-phy.h"
#include "ns3/node.h"
#include "ns3/lora-tag.h"

namespace ns3
{
namespace lorawan
{

NS_LOG_COMPONENT_DEFINE("GatewayLoraPhy");

NS_OBJECT_ENSURE_REGISTERED(GatewayLoraPhy);

/**************************************
 *    ReceptionPath implementation    *
 **************************************/
GatewayLoraPhy::ReceptionPath::ReceptionPath()
    : m_available(1),
      m_event(0),
      m_endReceiveEventId(EventId())
{
}

GatewayLoraPhy::ReceptionPath::~ReceptionPath(void)
{
}

bool
GatewayLoraPhy::ReceptionPath::IsAvailable(void)
{
    return m_available;
}

void
GatewayLoraPhy::ReceptionPath::Free(void)
{
    m_available = true;
    m_event = 0;
    m_endReceiveEventId = EventId();
}

void
GatewayLoraPhy::ReceptionPath::LockOnEvent(Ptr<LoraInterferenceHelper::Event> event)
{
    m_available = false;
    m_event = event;
}

Ptr<LoraInterferenceHelper::Event>
GatewayLoraPhy::ReceptionPath::GetEvent(void)
{
    return m_event;
}

EventId
GatewayLoraPhy::ReceptionPath::GetEndReceive(void)
{
    return m_endReceiveEventId;
}

void
GatewayLoraPhy::ReceptionPath::SetEndReceive(EventId endReceiveEventId)
{
    m_endReceiveEventId = endReceiveEventId;
}

/***********************************************************************
 *                 Implementation of Gateway methods                   *
 ***********************************************************************/

TypeId
GatewayLoraPhy::GetTypeId(void)
{
    static TypeId tid =
        TypeId("ns3::GatewayLoraPhy")
            .SetParent<LoraPhy>()
            .SetGroupName("lorawan")
            .AddTraceSource(
                "NoReceptionBecauseTransmitting",
                "Trace source indicating a packet "
                "could not be correctly received because"
                "the GW is in transmission mode",
                MakeTraceSourceAccessor(&GatewayLoraPhy::m_noReceptionBecauseTransmitting),
                "ns3::Packet::TracedCallback")
            .AddTraceSource("LostPacketBecauseNoMoreReceivers",
                            "Trace source indicating a packet "
                            "could not be correctly received because"
                            "there are no more demodulators available",
                            MakeTraceSourceAccessor(&GatewayLoraPhy::m_noMoreDemodulators),
                            "ns3::Packet::TracedCallback")
            .AddTraceSource("OccupiedReceptionPaths",
                            "Number of currently occupied reception paths",
                            MakeTraceSourceAccessor(&GatewayLoraPhy::m_occupiedReceptionPaths),
                            "ns3::TracedValueCallback::Int");
    return tid;
}

GatewayLoraPhy::GatewayLoraPhy()
    : m_isTransmitting(false)
{
    NS_LOG_FUNCTION_NOARGS();
}

GatewayLoraPhy::~GatewayLoraPhy()
{
    NS_LOG_FUNCTION_NOARGS();
}

void
GatewayLoraPhy::Send(Ptr<Packet> packet,
                     LoraTxParameters txParams,
                     double frequency,
                     double txPowerDbm)
{
    NS_LOG_FUNCTION(this << packet << txParams << frequency << txPowerDbm);

    // Get the time a packet with these parameters will take to be transmitted
    Time duration = GetOnAirTime(packet, txParams);
    NS_LOG_DEBUG("Duration of packet: " << duration << ", SF" << unsigned(txParams.sf));
    // Interrupt all receive operations
    for (auto& path : m_receptionPaths)
        if (!path->IsAvailable()) // Reception path is occupied
        {
            // Fire the trace source for reception interrupted by transmission
            m_noReceptionBecauseTransmitting(path->GetEvent()->GetPacket(),
                                             (m_device) ? m_device->GetNode()->GetId() : 0);
            // Cancel the scheduled EndReceive call
            Simulator::Cancel(path->GetEndReceive());
            // Free it and resets all parameters
            path->Free();
        }
    // Send the downlink packet in the channel
    m_channel->Send(this, packet, txPowerDbm, txParams, duration, frequency, true);
    Simulator::Schedule(duration, &GatewayLoraPhy::TxFinished, this);
    m_isTransmitting = true;
    // Fire the trace source
    m_startSending(packet, (m_device) ? m_device->GetNode()->GetId() : 0);
    // Fire the sniffer trace source
    if (!m_phySniffTxTrace.IsEmpty())
        Simulator::Schedule(duration, &GatewayLoraPhy::m_phySniffTxTrace, this, packet);
}

void
GatewayLoraPhy::StartReceive(Ptr<Packet> packet,
                             double rxPowerDbm,
                             uint8_t sf,
                             Time duration,
                             double frequency)
{
    NS_LOG_FUNCTION(this << packet << rxPowerDbm << duration << frequency);
    if (m_isTransmitting)
    {
        // If we get to this point, there are no demodulators we can use
        NS_LOG_INFO("Dropping packet reception of packet with sf = "
                    << unsigned(sf) << " because we are in TX mode");
        // Fire the trace sources
        m_noReceptionBecauseTransmitting(packet, (m_device) ? m_device->GetNode()->GetId() : 0);
        return;
    }
    // Add the event to the LoraInterferenceHelper
    auto event = m_interference.Add(duration, rxPowerDbm, sf, packet, frequency);
    // Cycle over the receive paths to check availability to receive the packet
    for (auto& path : m_receptionPaths)
        if (path->IsAvailable()) // If the receive path is available we have a candidate
        {
            // See whether the reception power is above or below the sensitivity
            // for that spreading factor
            double sensitivity = GatewayLoraPhy::sensitivity[unsigned(sf) - 7];
            if (rxPowerDbm < sensitivity) // Packet arrived below sensitivity
            {
                NS_LOG_INFO("Dropping packet reception of packet with sf = "
                            << unsigned(sf) << " because under the sensitivity of " << sensitivity
                            << " dBm");
                // Fire the trace sources
                m_underSensitivity(packet, (m_device) ? m_device->GetNode()->GetId() : 0);
            }
            else // We have sufficient sensitivity to start receiving
            {
                NS_LOG_INFO("Scheduling reception of a packet, occupying one demodulator");
                // Block this resource
                path->LockOnEvent(event);
                m_occupiedReceptionPaths++;
                // Schedule the end of the reception of the packet
                path->SetEndReceive(
                    Simulator::Schedule(duration, &LoraPhy::EndReceive, this, packet, event));
                // Fire the trace source
                m_phyRxBeginTrace(packet);
            }
            return;
        }
    // If we get to this point, there are no demodulators we can use
    NS_LOG_INFO("Dropping packet reception of packet with sf = "
                << unsigned(sf) << " and frequency " << frequency
                << "Hz because no suitable demodulator was found");
    // Fire the trace source
    m_noMoreDemodulators(packet, (m_device) ? m_device->GetNode()->GetId() : 0);
}

void
GatewayLoraPhy::EndReceive(Ptr<Packet> packet, Ptr<LoraInterferenceHelper::Event> event)
{
    NS_LOG_FUNCTION(this << packet << *event);
    // Call the trace source
    m_phyRxEndTrace(packet);
    // Call the LoraInterferenceHelper to determine whether there was
    // destructive interference. If the packet is correctly received, this
    // method returns a 0.
    uint8_t packetDestroyed = m_interference.IsDestroyedByInterference(event);
    // Check whether the packet was destroyed
    if (packetDestroyed)
    {
        NS_LOG_DEBUG("packetDestroyed by interference on SF " << unsigned(packetDestroyed));
        // Update the packet's LoraTag
        LoraTag tag;
        packet->RemovePacketTag(tag);
        tag.SetDestroyedBy(packetDestroyed);
        tag.SetReceptionTime(Simulator::Now());
        packet->AddPacketTag(tag);
        // Fire the trace source
        m_interferedPacket(packet, (m_device) ? m_device->GetNode()->GetId() : 0);
    }
    else // Reception was correct
    {
        NS_LOG_INFO("Packet with SF " << unsigned(event->GetSpreadingFactor())
                                      << " received correctly");
        // Set the receive power and frequency of this packet in the LoraTag: this
        // information can be useful for upper layers trying to control link
        // quality and to fill the packet sniffing header.
        LoraTag tag;
        packet->RemovePacketTag(tag);
        tag.SetReceivePower(event->GetRxPowerdBm());
        tag.SetFrequency(event->GetFrequency());
        tag.SetSnr(RxPowerToSNR(event->GetRxPowerdBm()));
        tag.SetReceptionTime(Simulator::Now());
        packet->AddPacketTag(tag);
        // Forward the packet to the upper layer
        if (!m_rxOkCallback.IsNull())
            m_rxOkCallback(packet);
        // Fire the trace source
        m_successfullyReceivedPacket(packet, (m_device) ? m_device->GetNode()->GetId() : 0);
        // Fire the sniffer trace source
        if (!m_phySniffRxTrace.IsEmpty())
            m_phySniffRxTrace(packet);
    }
    // Search for the demodulator that was locked on this event to free it.
    for (auto& path : m_receptionPaths)
        if (path->GetEvent() == event)
        {
            path->Free();
            m_occupiedReceptionPaths--;
            return;
        }
}

void
GatewayLoraPhy::CreateReceptionPaths(uint8_t number)
{
    NS_LOG_FUNCTION(this << (unsigned)number);
    m_receptionPaths.clear();
    for (uint8_t i = 0; i < number; ++i)
        m_receptionPaths.push_back(Create<GatewayLoraPhy::ReceptionPath>());
}

bool
GatewayLoraPhy::IsTransmitting(void)
{
    NS_LOG_FUNCTION_NOARGS();
    return m_isTransmitting;
}

void
GatewayLoraPhy::TxFinished(void)
{
    NS_LOG_FUNCTION_NOARGS();
    m_isTransmitting = false;
}

// Uplink sensitivity (Source: SX1301 datasheet)
// {SF7, SF8, SF9, SF10, SF11, SF12}
// These sensitivities are for a bandwidth of 125000 Hz
const double GatewayLoraPhy::sensitivity[6] = {-126.5, -129, -131.5, -134, -136.5, -139.5};

} // namespace lorawan
} // namespace ns3
