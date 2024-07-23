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

#ifndef LOGICAL_CHANNEL_H
#define LOGICAL_CHANNEL_H

#include "sub-band.h"

#include "ns3/object.h"

namespace ns3
{
namespace lorawan
{

class SubBand;

/**
 * This class represents a logical LoRaWAN channel.
 *
 * A logical channel is characterized by a central frequency and a range of data
 * rates that can be sent on it.
 *
 * Furthermore, a LogicalChannel can be marked as enabled or disabled for
 * uplink transmission.
 */
class LogicalChannel : public SimpleRefCount<LogicalChannel>
{
  public:
    LogicalChannel();
    virtual ~LogicalChannel();

    LogicalChannel(double frequency);

    /**
     * Constructor providing initialization of frequency and data rate limits.
     *
     * \param frequency This channel's frequency.
     * \param minDataRate This channel's minimum data rate.
     * \param maxDataRate This channel's maximum data rate.
     */
    LogicalChannel(double frequency, uint8_t minDataRate, uint8_t maxDataRate);

    /**
     * Get the frequency (Hz).
     *
     * \return The center frequency of this channel.
     */
    double GetFrequency() const;

    /**
     * Set the reply frequency (Hz).
     *
     * \param replyFrequency The center frequency this channel should receive replies on.
     */
    void SetReplyFrequency(double replyFrequency);

    /**
     * Get the reply frequency (Hz).
     *
     * \return The center frequency of replies of this channel.
     */
    double GetReplyFrequency() const;

    /**
     * Set the minimum Data Rate that is allowed on this channel.
     */
    void SetMinimumDataRate(uint8_t minDataRate);

    /**
     * Set the maximum Data Rate that is allowed on this channel.
     */
    void SetMaximumDataRate(uint8_t maxDataRate);

    /**
     * Get the minimum Data Rate that is allowed on this channel.
     */
    uint8_t GetMinimumDataRate() const;

    /**
     * Get the maximum Data Rate that is allowed on this channel.
     */
    uint8_t GetMaximumDataRate() const;

    /**
     * Set this channel as enabled for uplink.
     */
    void EnableForUplink();

    /**
     * Set this channel as disabled for uplink.
     */
    void DisableForUplink();

    /**
     * Test whether this channel is marked as enabled for uplink.
     */
    bool IsEnabledForUplink() const;

  private:
    /**
     * The central frequency for transmission of this channel, in Hz.
     */
    double m_frequency;

    /**
     * The central frequency on which we receive replies when using this channel, in Hz.
     */
    double m_replyFrequency;

    /**
     * The minimum Data Rate that is allowed on this channel.
     */
    uint8_t m_minDataRate;

    /**
     * The maximum Data Rate that is allowed on this channel.
     */
    uint8_t m_maxDataRate;

    /**
     * Whether this channel can be used for uplink or not.
     */
    bool m_enabledForUplink;
};

/**
 * Overload of the == operator to compare different instances of the same LogicalChannel
 */
bool operator==(const Ptr<LogicalChannel>& first, const Ptr<LogicalChannel>& second);

/**
 * Overload the != operator to compare different instances of the same LogicalChannel
 */
bool operator!=(const Ptr<LogicalChannel>& first, const Ptr<LogicalChannel>& second);

} // namespace lorawan

} // namespace ns3
#endif /* LOGICAL_CHANNEL_H */
