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

#ifndef GATEWAY_LORA_PHY_H
#define GATEWAY_LORA_PHY_H

#include "lora-phy.h"

#include "ns3/traced-value.h"

namespace ns3
{
namespace lorawan
{

/**
 * Class modeling a Lora SX1301 chip.
 *
 * This class models the behaviour of the chip employed in Lora gateways. These
 * chips are characterized by the presence of 8 receive paths, or parallel
 * receivers, which can be employed to listen to different channels
 * simultaneously. This characteristic of the chip is modeled using the
 * ReceivePath class, which describes a single parallel receiver. GatewayLoraPhy
 * essentially holds and manages a collection of these objects.
 */
class GatewayLoraPhy : public LoraPhy
{
    /**
     * This class represents a configurable reception path.
     *
     * Differently from EndDeviceLoraPhys, these do not need to be configured to
     * listen for a certain SF. ReceptionPaths be either locked on an event or
     * free.
     */
    class ReceptionPath : public SimpleRefCount<GatewayLoraPhy::ReceptionPath>
    {
      public:
        /**
         * Constructor.
         */
        ReceptionPath();

        ~ReceptionPath();

        /**
         * Query whether this reception path is available to lock on a signal.
         *
         * \return True if its current state is free, false if it's currently locked.
         */
        bool IsAvailable() const;

        /**
         * Set this reception path as available.
         *
         * This function sets the m_available variable as true, and deletes the
         * LoraInterferenceHelper Event this ReceivePath was previously locked on.
         */
        void Free();

        /**
         * Set this reception path as not available and lock it on the
         * provided event.
         *
         * \param event The LoraInterferenceHelper Event to lock on.
         */
        void LockOnEvent(Ptr<LoraInterferenceHelper::Event> event);

        /**
         * Get the event this reception path is currently on.
         *
         * \returns 0 if no event is currently being received, a pointer to
         * the event otherwise.
         */
        Ptr<LoraInterferenceHelper::Event> GetEvent();

        /**
         * Get the EventId of the EndReceive call associated to this ReceptionPath's
         * packet.
         */
        EventId GetEndReceive();

        /**
         * Set the EventId of the EndReceive call associated to this ReceptionPath's
         * packet.
         */
        void SetEndReceive(EventId endReceiveEventId);

      private:
        /**
         * Whether this reception path is available to lock on a signal or not.
         */
        bool m_available;

        /**
         * The event this reception path is currently locked on.
         */
        Ptr<LoraInterferenceHelper::Event> m_event;

        /**
         * The EventId associated of the call to EndReceive that is scheduled to
         * happen when the packet this ReceivePath is locked on finishes reception.
         */
        EventId m_endReceiveEventId;
    };

  public:
    static TypeId GetTypeId();

    GatewayLoraPhy();
    ~GatewayLoraPhy() override;

    void StartReceive(Ptr<Packet> packet,
                      double rxPowerDbm,
                      uint8_t sf,
                      Time duration,
                      double frequency) override;

    void Send(Ptr<Packet> packet,
              LoraPhyTxParameters txParams,
              double frequency,
              double txPowerDbm) override;

    /**
     * Used to check the gateway transmission state by the outside
     */
    bool IsTransmitting() override;

    /**
     * Set a certain number of reception paths.
     */
    void SetReceptionPaths(uint8_t number);

  protected:
    void DoDispose() override;

    void EndReceive(Ptr<Packet> packet, Ptr<LoraInterferenceHelper::Event> event) override;

    /**
     * Used to schedule a change in the gateway transmission state
     */
    virtual void TxFinished(Ptr<Packet> packet);

    /**
     * A vector containing the various parallel receivers that are managed by this
     * Gateway.
     */
    std::vector<Ptr<ReceptionPath>> m_receptionPaths;

    bool m_isTransmitting; //!< Flag indicating whether a transmission is going on

    /**
     * A vector containing the sensitivities required to correctly decode
     * different spreading factors.
     */
    static const double sensitivity[6];

    /**
     * The number of occupied reception paths.
     */
    TracedValue<int> m_occupiedReceptionPaths;

    /**
     * Trace source that is fired when a packet cannot be received because all
     * available ReceivePath instances are busy.
     *
     * \see class CallBackTraceSource
     */
    TracedCallback<Ptr<const Packet>, uint32_t> m_noMoreDemodulators;

    /**
     * Trace source that is fired when a packet cannot be received because
     * the Gateway is in transmission state.
     *
     * \see class CallBackTraceSource
     */
    TracedCallback<Ptr<const Packet>, uint32_t> m_noReceptionBecauseTransmitting;
};

} // namespace lorawan

} // namespace ns3
#endif /* GATEWAY_LORA_PHY_H */
