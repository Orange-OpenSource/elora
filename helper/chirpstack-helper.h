/*
 * Copyright (c) 2022 Orange SA
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Alessandro Aimi <alessandro.aimi@orange.com>
 *                         <alessandro.aimi@cnam.fr>
 */

#ifndef CHIRPSTACK_HELPER_H
#define CHIRPSTACK_HELPER_H

#include "ns3/loragw_hal.h"
#include "ns3/node-container.h"

#include <curl/curl.h>

namespace ns3
{
namespace lorawan
{

/**
 * This class can be used to install devices and gateways on a real
 * chirpstack network server using the REST API.
 * Requires libcurl-dev installed.
 */
class ChirpstackHelper
{
    using str = std::string;
    using query_t = std::vector<std::pair<str, str>>;

    struct session_t
    {
        // Registration info
        str tenant = "Ns-3 Simulator";
        str devProf = "Ns-3 Device Profile";
        str app = "Ns-3 Application";

        // Session IDs
        str tenantId;
        str devProfId;
        str appId;

        // Session keys
        str netKey;
        str appKey;
    };

  public:
    ChirpstackHelper();

    int InitConnection(const str address, uint16_t port, const str token);

    void CloseConnection(int signal);

    int Register(NodeContainer c) const;

    int Register(Ptr<Node> node) const;

    int CreateHttpIntegration(const str& encoding, const str& endpoint) const;

    int CreateInfluxDb2Integration(const str& endpoint,
                                   const str& organization,
                                   const str& bucket,
                                   const str& token) const;

    void SetTenant(str& name);

    void SetDeviceProfile(str& name);

    void SetApplication(str& name);

  private:
    int DoConnect();

    int CreateTenant(const str& name);

    int CreateDeviceProfile(const str& name);

    int CreateApplication(const str& name);

    int RegisterPriv(Ptr<Node> node) const;

    int CreateDevice(Ptr<Node> node) const;

    int CreateGateway(Ptr<Node> node) const;

    int POST(const str& path, const str& body, str& out) const;

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

    static const struct coord_s m_center;
};

} // namespace lorawan

} // namespace ns3
#endif /* CHIRPSTACK_HELPER_H */
