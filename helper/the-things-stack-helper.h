/*
 * Copyright (c) 2024 INSA Lyon
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Carlos Fernandez Hernandez <carlos.fernandez-hernandez@insa-lyon.fr>
 */

#ifndef THE_THINGS_STACK_HELPER_H
#define THE_THINGS_STACK_HELPER_H

#include "ns3/loragw_hal.h"
#include "ns3/node-container.h"

#include <curl/curl.h>

namespace ns3
{
namespace lorawan
{

/**
 * This class can be used to install devices and gateways on a real
 * The Things Stack network server using the REST API.
 * Requires libcurl-dev installed.
 */
class TheThingsStackHelper
{
    using str = std::string;

    struct session_t
    {
        // Registration info
        str app = "ns-3-application";

        // Session IDs
        str appId;
        int nDevices;
        int nGateway;

        // Session keys
        str netKey;
        str appKey;
    };

  public:
    TheThingsStackHelper();

    ~TheThingsStackHelper();

    int InitConnection(const str address, uint16_t port, const str token);

    void CloseConnection(int signal) const;

    int Register(NodeContainer c) const;

    int Register(Ptr<Node> node) const;

    void SetApplication(str& name);

    void SetNodes(int n, int gw);

  private:
    int DoConnect();

    int CreateApplication(const str& name);

    int RegisterPriv(Ptr<Node> node) const;

    int CreateDevice(Ptr<Node> node) const;

    int CreateGateway(Ptr<Node> node) const;

    int POST(const str& path, const str& body, str& out) const;

    int PUT(const str& path, const str& body, str& out) const;

    int DELETE(const str& path, str& out) const;

    static size_t StreamWriteCallback(char* buffer,
                                      size_t size,
                                      size_t nitems,
                                      std::ostream* stream);

    str m_url;
    str m_token;
    struct curl_slist* m_header = nullptr;

    session_t m_session;
    uint64_t m_run;
};

} // namespace lorawan
} // namespace ns3

#endif /* THE_THINGS_STACK_HELPER_H */
