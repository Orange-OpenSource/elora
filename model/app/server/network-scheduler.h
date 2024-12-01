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

#ifndef NETWORK_SCHEDULER_H
#define NETWORK_SCHEDULER_H

#include "network-controller.h"
#include "network-status.h"

#include "ns3/core-module.h"
#include "ns3/lora-device-address.h"
#include "ns3/lora-frame-header.h"
#include "ns3/lorawan-mac-header.h"
#include "ns3/object.h"
#include "ns3/packet.h"

namespace ns3
{
namespace lorawan
{

class NetworkStatus;     // Forward declaration
class NetworkController; // Forward declaration

class NetworkScheduler : public Object
{
  public:
    static TypeId GetTypeId();

    NetworkScheduler();
    NetworkScheduler(Ptr<NetworkStatus> status, Ptr<NetworkController> controller);
    ~NetworkScheduler() override;

    /**
     * Method called by NetworkServer to inform the Scheduler of a newly arrived
     * uplink packet. This function schedules the OnReceiveWindowOpportunity
     * events 1 and 2 seconds later.
     */
    void OnReceivedPacket(Ptr<const Packet> packet);

    /**
     * Method that is scheduled after packet arrivals in order to act on
     * receive windows 1 and 2 seconds later receptions.
     */
    void OnReceiveWindowOpportunity(LoraDeviceAddress deviceAddress, int window);

  protected:
    void DoDispose() override;

  private:
    TracedCallback<Ptr<const Packet>> m_receiveWindowOpened;
    Ptr<NetworkStatus> m_status;
    Ptr<NetworkController> m_controller;
};

} // namespace lorawan

} // namespace ns3
#endif /* NETWORK_SCHEDULER_H */
