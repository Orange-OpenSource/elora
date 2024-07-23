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

#include "logical-channel.h"

#include "ns3/log.h"

namespace ns3
{
namespace lorawan
{

NS_LOG_COMPONENT_DEFINE("LogicalChannel");

LogicalChannel::LogicalChannel()
    : m_frequency(0),
      m_replyFrequency(0),
      m_minDataRate(0),
      m_maxDataRate(5),
      m_enabledForUplink(true)
{
    NS_LOG_FUNCTION(this);
}

LogicalChannel::~LogicalChannel()
{
    NS_LOG_FUNCTION(this);
}

LogicalChannel::LogicalChannel(double frequency)
    : m_frequency(frequency),
      m_replyFrequency(frequency),
      m_enabledForUplink(true)
{
    NS_LOG_FUNCTION(this);
}

LogicalChannel::LogicalChannel(double frequency, uint8_t minDataRate, uint8_t maxDataRate)
    : m_frequency(frequency),
      m_replyFrequency(frequency),
      m_minDataRate(minDataRate),
      m_maxDataRate(maxDataRate),
      m_enabledForUplink(true)
{
    NS_LOG_FUNCTION(this);
}

double
LogicalChannel::GetFrequency() const
{
    return m_frequency;
}

void
LogicalChannel::SetReplyFrequency(double replyFrequency)
{
    m_replyFrequency = replyFrequency;
}

double
LogicalChannel::GetReplyFrequency() const
{
    return m_replyFrequency;
}

void
LogicalChannel::SetMinimumDataRate(uint8_t minDataRate)
{
    m_minDataRate = minDataRate;
}

void
LogicalChannel::SetMaximumDataRate(uint8_t maxDataRate)
{
    m_maxDataRate = maxDataRate;
}

uint8_t
LogicalChannel::GetMinimumDataRate() const
{
    return m_minDataRate;
}

uint8_t
LogicalChannel::GetMaximumDataRate() const
{
    return m_maxDataRate;
}

void
LogicalChannel::EnableForUplink()
{
    m_enabledForUplink = true;
}

void
LogicalChannel::DisableForUplink()
{
    m_enabledForUplink = false;
}

bool
LogicalChannel::IsEnabledForUplink() const
{
    return m_enabledForUplink;
}

bool
operator==(const Ptr<LogicalChannel>& first, const Ptr<LogicalChannel>& second)
{
    double thisFreq = first->GetFrequency();
    double otherFreq = second->GetFrequency();

    NS_LOG_DEBUG("Checking equality between logical lora channels: " << thisFreq << " "
                                                                     << otherFreq);

    NS_LOG_DEBUG("Result:" << (thisFreq == otherFreq));
    return (thisFreq == otherFreq);
}

bool
operator!=(const Ptr<LogicalChannel>& first, const Ptr<LogicalChannel>& second)
{
    return !(first == second);
}
} // namespace lorawan
} // namespace ns3
