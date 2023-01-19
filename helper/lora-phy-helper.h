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
 * 17/01/2023
 * Modified by: Alessandro Aimi <alessandro.aimi@orange.com>
 *                              <alessandro.aimi@cnam.fr>
 */

#ifndef LORA_PHY_HELPER_H
#define LORA_PHY_HELPER_H

#include "ns3/gateway-lora-phy.h"
#include "ns3/lora-channel.h"
#include "ns3/lora-phy.h"
#include "ns3/lorawan-mac.h"
#include "ns3/lora-net-device.h"
#include "ns3/object-factory.h"
#include "ns3/simple-end-device-lora-phy.h"

namespace ns3
{
namespace lorawan
{

/**
 * Helper to install LoraPhy instances on multiple Nodes.
 */
class LoraPhyHelper
{
  public:
    /**
     * Enum for the type of device: End Device (ED) or Gateway (GW)
     */
    enum DeviceType
    {
        GW,
        ED
    };

    /**
     * Create a phy helper without any parameter set. The user must set
     * them all to be able to call Install later.
     */
    LoraPhyHelper();

    /**
     * Set the LoraChannel to connect the PHYs to.
     *
     * Every PHY created by a call to Install is associated to this channel.
     *
     * \param channel the channel to associate to this helper.
     */
    void SetChannel(Ptr<LoraChannel> channel);

    /**
     * Set the kind of PHY this helper will create.
     *
     * \param dt the device type.
     */
    void SetDeviceType(enum DeviceType dt);

    /**
     * Set an attribute of the underlying PHY object.
     *
     * \param name the name of the attribute to set.
     * \param v the value of the attribute.
     */
    void Set(std::string name, const AttributeValue& v);

    /**
     * Crate a LoraPhy and connect it to a device on a node.
     *
     * \param node the node on which we wish to create a wifi PHY.
     * \param device the device within which this PHY will be created.
     * \return a newly-created PHY object.
     */
    Ptr<LoraPhy> Create(Ptr<Node> node, Ptr<LoraNetDevice> device) const;

  private:
    /**
     * The PHY layer factory object.
     */
    ObjectFactory m_phy;

    /**
     * The channel instance the PHYs will be connected to.
     */
    Ptr<LoraChannel> m_channel;
};

} // namespace lorawan

} // namespace ns3
#endif /* LORA_PHY_HELPER_H */
