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

#ifndef GATEWAY_LORAWAN_MAC_H
#define GATEWAY_LORAWAN_MAC_H

#include "lorawan-mac.h"

#include "ns3/lora-tag.h"

namespace ns3
{
namespace lorawan
{

class GatewayLorawanMac : public LorawanMac
{
  public:
    static TypeId GetTypeId();

    GatewayLorawanMac();
    ~GatewayLorawanMac() override;

    // Implementation of the LorawanMac interface
    void Send(Ptr<Packet> packet) override;

    // Implementation of the LorawanMac interface
    void Receive(Ptr<const Packet> packet) override;

    // Implementation of the LorawanMac interface
    void FailedReception(Ptr<const Packet> packet) override;

    // Implementation of the LorawanMac interface
    void TxFinished(Ptr<const Packet> packet) override;

    // Check whether the physiscal layer is currently transmitting
    bool IsTransmitting();

    /**
     * Return the next time at which we will be able to transmit.
     *
     * \return The next transmission time.
     */
    Time GetWaitingTime(double frequency);

  private:
  protected:
};

} // namespace lorawan

} // namespace ns3
#endif /* GATEWAY_LORAWAN_MAC_H */
