/*
 * Copyright (c) 2017 University of Padova
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Davide Magrin <magrinda@dei.unipd.it>
 *         Martina Capuzzo <capuzzom@dei.unipd.it>
 *
 * Modified by: Peggy Anderson <peggy.anderson@usask.ca>
 *
 * 11/01/2023
 * Modified by: Alessandro Aimi <alessandro.aimi@orange.com>
 *                              <alessandro.aimi@cnam.fr>
 */

#ifndef CLASS_A_END_DEVICE_LORAWAN_MAC_H
#define CLASS_A_END_DEVICE_LORAWAN_MAC_H

#include "base-end-device-lorawan-mac.h"
#include "recv-window-manager.h"

// #include "ns3/traced-value.h"

namespace ns3
{
namespace lorawan
{

/**
 * Class representing the MAC layer of a Class A LoRaWAN device.
 */
class ClassAEndDeviceLorawanMac : public BaseEndDeviceLorawanMac
{
    enum RxOutcome
    {
        ACK,
        RECV,
        FAIL,
        NONE
    };

  public:
    static TypeId GetTypeId();

    ClassAEndDeviceLorawanMac();
    ~ClassAEndDeviceLorawanMac() override;

    /**
     * Perform the actions that are required after a packet send.
     *
     * This function handles opening of the first receive window.
     */
    void TxFinished(Ptr<const Packet> packet) override;

    /**
     * Receive a packet.
     *
     * This method is typically registered as a callback in the underlying PHY
     * layer so that it's called when a packet is going up the stack.
     *
     * \param packet the received packet.
     */
    void Receive(Ptr<const Packet> packet) override;

    /**
     * Signal reception failure.
     *
     * This method is typically registered as a callback in the underlying PHY
     * layer so that it's called when a packet is going up the stack.
     *
     * \param packet the failed packet.
     */
    void FailedReception(Ptr<const Packet> packet) override;

    /**
     * Signal no reception during either reception window.
     *
     * This method is typically registered as a callback in the reception window
     * manager that it's called when the second reception window ends.
     */
    void NoReception();

    /////////////////////////
    // Getters and Setters //
    /////////////////////////

    /**
     * Get the first receive window opening delay, starting from transmission end.
     *
     * @return The first receive window opening delay.
     */
    Time GetFirstReceiveWindowDelay();

    /**
     * Get the data rate that will be used in the first receive window.
     *
     * @return The data rate.
     */
    uint8_t GetFirstReceiveWindowDataRate();

    /**
     * Set the data rate to be used in the second receive window.
     *
     * @param dataRate The data rate.
     */
    void SetSecondReceiveWindowDataRate(uint8_t dataRate);

    /**
     * Get the data rate that will be used in the second receive window.
     *
     * @return The data rate.
     */
    uint8_t GetSecondReceiveWindowDataRate() const;

    /**
     * Set the frequency that will be used for the second receive window.
     *
     * @param frequencyHz The Frequency.
     */
    void SetSecondReceiveWindowFrequency(uint32_t frequencyHz);

    /**
     * Get the frequency that is used for the second receive window.
     *
     * @return The frequency, in Hz.
     */
    uint32_t GetSecondReceiveWindowFrequency() const;

  protected:
    void DoInitialize() override;
    void DoDispose() override;

  private:
    /**
     * Add headers and send a packet with the sending function of the physical layer.
     *
     * \param packet the packet to send
     */
    void SendToPhy(Ptr<Packet> packet) override;

    /**
     * Find the minimum waiting time before the next possible transmission based
     * on End Device's transmission/reception process.
     */
    Time GetBusyTransmissionDelay() override;

    /**
     * Decide whether we can retransmit based on reception outcome.
     *
     * \param outcome Outcome of the reception.
     */
    void ManageRetransmissions(RxOutcome outcome);

    /**
     * Compute the time duration of a reception window based on its datarate.
     */
    Time GetReceptionWindowDuration(uint8_t datarate);

    /////////////////////////
    // MAC command methods //
    /////////////////////////

    /**
     * Perform the actions that need to be taken when receiving a RxParamSetupReq
     * command.
     *
     * \param rxParamSetupReq The Parameter Setup Request, which contains:
     *                            - The offset to set.
     *                            - The data rate to use for the second receive window.
     *                            - The frequency to use for the second receive window.
     */
    void OnRxParamSetupReq(Ptr<RxParamSetupReq> rxParamSetupReq) override;

    /**
     * Perform the actions that need to be taken when receiving a RxTimingSetupReq command.
     */
    void OnRxTimingSetupReq(uint8_t del) override;

    /**
     * The duration of a receive window in number of symbols. This should be
     * converted to time based or the reception parameter used.
     *
     * The downlink preamble transmitted by the gateways contains 8 symbols.
     * The receiver requires 5 symbols to detect the preamble and synchronize.
     * Therefore there must be a 5 symbols overlap between the receive window
     * and the transmitted preamble.
     * (Ref: Recommended SX1272/76 Settings for EU868 LoRaWAN Network Operation )
     */
    uint16_t m_recvWinSymb;

    /**
     * The RX1DROffset parameter value
     */
    uint8_t m_rx1DrOffset;

    /**
     * Last channel used for tx
     */
    Ptr<LogicalChannel> m_lastTxCh;

    /**
     * Reception window process manager.
     */
    Ptr<RecvWindowManager> m_rwm;

}; /* ClassAEndDeviceLorawanMac */

} /* namespace lorawan */
} /* namespace ns3 */

#endif /* CLASS_A_END_DEVICE_LORAWAN_MAC_H */
