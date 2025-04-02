/*
 * Copyright (c) 2024 INSA Lyon
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Carlos Fernandez Hernandez <carlos.fernandez-hernandez@insa-lyon.fr>
 */

#ifndef THE_THINGS_STACK_HELPER_H
#define THE_THINGS_STACK_HELPER_H

#include "ns3/node-container.h"
#include "ns3/rest-api-helper.h"

namespace ns3
{
namespace lorawan
{

/**
 * This class can be used to install devices and gateways on a real The Things Stack network server
 * using the REST API.
 *
 * \warning Requires libcurl-dev installed.
 */
class TheThingsStackHelper : public RestApiHelper
{
    using str = std::string;

    struct session_t
    {
        // Registration info
        str app = "Ns-3 Application";

        // Session IDs
        str appId;
        std::vector<str> devIds;
        std::vector<str> gwIds;

        // Session keys
        str netKey;
        str appKey;
    };

  public:
    TheThingsStackHelper();

    ~TheThingsStackHelper();

    void CloseConnection(int signal) override;

    int Register(NodeContainer c);

    int Register(Ptr<Node> node);

    void SetApplication(str& name);

  private:
    int DoConnect() override;

    int CreateApplication(const str& name);

    int RegisterPriv(Ptr<Node> node);

    int CreateDevice(Ptr<Node> node);

    int CreateGateway(Ptr<Node> node);

    session_t m_session;
    uint64_t m_run;
};

} // namespace lorawan
} // namespace ns3

#endif /* THE_THINGS_STACK_HELPER_H */
