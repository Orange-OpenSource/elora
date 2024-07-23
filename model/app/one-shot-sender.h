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

#ifndef ONE_SHOT_SENDER_H
#define ONE_SHOT_SENDER_H

#include "lora-application.h"

namespace ns3
{
namespace lorawan
{

class OneShotSender : public LoraApplication
{
  public:
    OneShotSender();
    OneShotSender(Time sendTime);
    ~OneShotSender() override;

    static TypeId GetTypeId();

    /**
     * Send a packet using the LoraNetDevice's Send method.
     */
    void SendPacket() override;

    /**
     * Set the time at which this app will send a packet.
     */
    void SetSendTime(Time sendTime);

    /**
     * Start the application by scheduling the first SendPacket event.
     */
    void StartApplication() override;
};

} // namespace lorawan

} // namespace ns3
#endif /* ONE_SHOT_APPLICATION */
