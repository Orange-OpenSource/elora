/*
 * Copyright (c) 2022 Orange SA
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Alessandro Aimi <alessandro.aimi@orange.com>
 *                         <alessandro.aimi@cnam.fr>
 */

#ifndef POISSON_SENDER_H
#define POISSON_SENDER_H

#include "lora-application.h"

namespace ns3
{
namespace lorawan
{

class PoissonSender : public LoraApplication
{
  public:
    PoissonSender();
    ~PoissonSender() override;

    static TypeId GetTypeId();

  protected:
    void DoInitialize() override;
    void DoDispose() override;

  private:
    /**
     * Start the application by scheduling the first SendPacket event
     */
    void StartApplication() override;

    /**
     * Send a packet using the LoraNetDevice's Send method
     */
    void SendPacket() override;

    Ptr<ExponentialRandomVariable> m_interval; //!< Random variable modeling packet inter-send time
};

} // namespace lorawan

} // namespace ns3
#endif /* POISSON_SENDER_H */
