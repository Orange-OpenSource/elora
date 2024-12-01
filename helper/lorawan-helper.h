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

#ifndef LORAWAN_HELPER_H
#define LORAWAN_HELPER_H

#include "lora-packet-tracker.h"
#include "lora-phy-helper.h"
#include "lorawan-mac-helper.h"

#include "ns3/lora-net-device.h"
#include "ns3/net-device-container.h"
#include "ns3/net-device.h"
#include "ns3/node-container.h"
#include "ns3/trace-helper.h"

#include <ctime>

namespace ns3
{
namespace lorawan
{

/**
 * Helps to create LoraNetDevice objects
 *
 * This class can help create a large set of similar LoraNetDevice objects and
 * configure a large set of their attributes during creation.
 */
class LorawanHelper : public PcapHelperForDevice
{
  public:
    enum TraceLevel
    {
        PKT,
        DEV,
        SF,
        GW,
        NET
    };

    LorawanHelper();
    ~LorawanHelper() override;

    /**
     * Install LoraNetDevices on a list of nodes
     *
     * \param phy the PHY helper to create PHY objects
     * \param mac the MAC helper to create MAC objects
     * \param c the set of nodes on which a lora device must be created
     * \returns a device container which contains all the devices created by this
     * method.
     */
    virtual NetDeviceContainer Install(const LoraPhyHelper& phyHelper,
                                       const LorawanMacHelper& macHelper,
                                       NodeContainer c) const;

    /**
     * Install LoraNetDevice on a single node
     *
     * \param phy the PHY helper to create PHY objects
     * \param mac the MAC helper to create MAC objects
     * \param node the node on which a lora device must be created
     * \returns a device container which contains all the devices created by this
     * method.
     */
    virtual NetDeviceContainer Install(const LoraPhyHelper& phyHelper,
                                       const LorawanMacHelper& macHelper,
                                       Ptr<Node> node) const;

    /**
     * Enable tracking of packets via trace sources.
     *
     * This method automatically connects to trace sources to computes relevant
     * metrics.
     */
    void EnablePacketTracking();

    /**
     * Periodically prints the simulation time to the standard output.
     */
    void EnableSimulationTimePrinting(Time interval);

    /**
     * Periodically prints the status of devices in the network to a file.
     */
    void EnablePeriodicDeviceStatusPrinting(NodeContainer endDevices,
                                            NodeContainer gateways,
                                            std::string filename,
                                            Time interval);

    /**
     * Print a summary of the status of all devices in the network.
     */
    void DoPrintDeviceStatus(NodeContainer endDevices,
                             NodeContainer gateways,
                             std::string filename);

    /**
     * Periodically prints PHY-level performance at every gateway in the container.
     */
    void EnablePeriodicGwsPerformancePrinting(NodeContainer gateways,
                                              std::string filename,
                                              Time interval);

    void DoPrintGwsPerformance(NodeContainer gateways, std::string filename);

    /**
     * Periodically prints global performance.
     */
    void EnablePeriodicGlobalPerformancePrinting(std::string filename, Time interval);

    void DoPrintGlobalPerformance(std::string filename);

    void EnablePeriodicSFStatusPrinting(NodeContainer endDevices,
                                        NodeContainer gateways,
                                        std::string filename,
                                        Time interval);

    void DoPrintSFStatus(NodeContainer endDevices, NodeContainer gateways, std::string filename);

    void EnablePrinting(NodeContainer endDevices,
                        NodeContainer gateways,
                        std::vector<enum TraceLevel> levels,
                        Time samplePeriod);

    LoraPacketTracker& GetPacketTracker();

    LoraPacketTracker* m_packetTracker = nullptr;

    time_t m_oldtime;

  protected:
    static void PcapSniffRxEvent(Ptr<PcapFileWrapper> file, Ptr<const Packet> packet);

    static void PcapSniffTxEvent(Ptr<PcapFileWrapper> file, Ptr<const Packet> packet);

  private:
    /**
     * Actually print the simulation time and re-schedule execution of this
     * function.
     */
    void DoPrintSimulationTime(Time interval);

    void EnablePcapInternal(std::string prefix,
                            Ptr<NetDevice> nd,
                            bool promiscuous,
                            bool explicitFilename) override;

    Time m_lastPhyPerformanceUpdate;
    Time m_lastGlobalPerformanceUpdate;
    Time m_lastDeviceStatusUpdate;
    Time m_lastSFStatusUpdate;
};

} // namespace lorawan

} // namespace ns3
#endif /* LORAWAN_HELPER_H */
