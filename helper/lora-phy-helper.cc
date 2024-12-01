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

#include "lora-phy-helper.h"

namespace ns3
{
namespace lorawan
{

NS_LOG_COMPONENT_DEFINE("LoraPhyHelper");

LoraPhyHelper::LoraPhyHelper()
{
    m_interferenceHelper.SetTypeId("ns3::LoraInterferenceHelper");
}

LoraPhyHelper::~LoraPhyHelper()
{
    m_channel = nullptr;
}

Ptr<LoraPhy>
LoraPhyHelper::Install(Ptr<LoraNetDevice> device) const
{
    auto phy = m_phy.Create<LoraPhy>();
    auto interference = m_interferenceHelper.Create<LoraInterferenceHelper>();
    phy->SetInterferenceHelper(interference);
    phy->SetChannel(m_channel);
    device->SetPhy(phy);
    return phy;
}

void
LoraPhyHelper::SetChannel(Ptr<LoraChannel> channel)
{
    m_channel = channel;
}

} // namespace lorawan
} // namespace ns3
