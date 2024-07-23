/*
 * Copyright (c) 2022 Orange SA
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Alessandro Aimi <alessandro.aimi@orange.com>
 *                         <alessandro.aimi@cnam.fr>
 */

#ifndef LORA_APPLICATION_H
#define LORA_APPLICATION_H

#include "ns3/application.h"
#include "ns3/base-end-device-lorawan-mac.h"

namespace ns3
{
namespace lorawan
{

class LoraApplication : public Application
{
  public:
    /**
     * \brief Get the type ID.
     * \return the object TypeId
     */
    static TypeId GetTypeId();
    LoraApplication();
    ~LoraApplication() override;

    /**
     * Set the sending interval
     * \param interval the interval between two packet sendings
     */
    void SetInterval(Time interval);

    /**
     * Get the sending inteval
     * \returns the interval between two packet sends
     */
    Time GetInterval() const;

    /**
     * Set the initial delay of this application
     */
    void SetInitialDelay(Time delay);

    /**
     * Set packet size
     */
    void SetPacketSize(uint8_t size);

    /**
     * Get packet size
     */
    uint8_t GetPacketSize() const;

    /**
     * True if the application is currently running
     */
    bool IsRunning();

  protected:
    void DoInitialize() override;
    void DoDispose() override;

    /**
     * Start the application by scheduling the first SendPacket event
     */
    void StartApplication() override;

    /**
     * Stop the application
     */
    void StopApplication() override;

    /**
     * Send a packet using the LoraNetDevice's Send method
     */
    virtual void SendPacket();

    /**
     * The average interval between to consecutive send events
     */
    Time m_avgInterval;

    /**
     * The initial delay of this application
     */
    Time m_initialDelay;

    /**
     * The sending event scheduled as next
     */
    EventId m_sendEvent;

    /**
     * The packet size.
     */
    uint8_t m_basePktSize;

    /**
     * The MAC layer of this node
     */
    Ptr<BaseEndDeviceLorawanMac> m_mac;
};

} // namespace lorawan

} // namespace ns3
#endif /* LORA_APPLICATION_H */
