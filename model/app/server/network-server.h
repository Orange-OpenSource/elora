/*
 * Copyright (c) 2018 University of Padova
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Authors: Davide Magrin <magrinda@dei.unipd.it>
 *          Martina Capuzzo <capuzzom@dei.unipd.it>
 *
 * 17/01/2023
 * Modified by: Alessandro Aimi <alessandro.aimi@orange.com>
 *                              <alessandro.aimi@cnam.fr>
 */

#ifndef NETWORK_SERVER_H
#define NETWORK_SERVER_H

#include "gateway-status.h"
#include "network-controller.h"
#include "network-scheduler.h"
#include "network-status.h"

#include "ns3/application.h"
#include "ns3/class-a-end-device-lorawan-mac.h"
#include "ns3/log.h"
#include "ns3/lora-device-address.h"
#include "ns3/net-device.h"
#include "ns3/node-container.h"
#include "ns3/object.h"
#include "ns3/packet.h"
#include "ns3/point-to-point-net-device.h"

namespace ns3
{
namespace lorawan
{

/**
 * The NetworkServer is an application standing on top of a node equipped with
 * links that connect it with the gateways.
 *
 * This version of the NetworkServer attempts to closely mimic an actual
 * Network Server, by providing as much functionality as possible.
 */
class NetworkServer : public Application
{
  public:
    static TypeId GetTypeId();

    NetworkServer();
    ~NetworkServer() override;

    /**
     * Start the NS application.
     */
    void StartApplication() override;

    /**
     * Stop the NS application.
     */
    void StopApplication() override;

    /**
     * Inform the NetworkServer that these nodes are connected to the network.
     *
     * This method will create a DeviceStatus object for each new node, and add
     * it to the list.
     */
    void AddNodes(NodeContainer nodes);

    /**
     * Inform the NetworkServer that this node is connected to the network.
     * This method will create a DeviceStatus object for the new node (if it
     * doesn't already exist).
     */
    void AddNode(Ptr<Node> node);

    /**
     * Add this gateway to the list of gateways connected to this NS.
     * Each GW is identified by its Address in the NS-GWs network.
     */
    void AddGateway(Ptr<Node> gateway, Ptr<NetDevice> netDevice);

    /**
     * A NetworkControllerComponent to this NetworkServer instance.
     */
    void AddComponent(Ptr<NetworkControllerComponent> component);

    /**
     * Receive a packet from a gateway.
     * \param packet the received packet
     */
    bool Receive(Ptr<NetDevice> device,
                 Ptr<const Packet> packet,
                 uint16_t protocol,
                 const Address& address);

    Ptr<NetworkStatus> GetNetworkStatus();

  protected:
    void DoDispose() override;

    Ptr<NetworkStatus> m_status;
    Ptr<NetworkController> m_controller;
    Ptr<NetworkScheduler> m_scheduler;

    TracedCallback<Ptr<const Packet>> m_receivedPacket;
};

} // namespace lorawan

} // namespace ns3
#endif /* NETWORK_SERVER_H */
