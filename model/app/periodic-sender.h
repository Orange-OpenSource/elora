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

#ifndef PERIODIC_SENDER_H
#define PERIODIC_SENDER_H

#include "lora-application.h"

namespace ns3
{
namespace lorawan
{

class PeriodicSender : public LoraApplication
{
  public:
    PeriodicSender();
    ~PeriodicSender() override;

    static TypeId GetTypeId();

  private:
    /**
     * Start the application by scheduling the first SendPacket event
     */
    void StartApplication() override;

    /**
     * Send a packet using the LoraNetDevice's Send method
     */
    void SendPacket() override;
};

} // namespace lorawan

} // namespace ns3
#endif /* SENDER_APPLICATION */
