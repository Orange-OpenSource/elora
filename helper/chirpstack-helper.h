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
#include "ns3/rest-api-helper.h"

namespace ns3
{
namespace lorawan
{

/**
 * This class can be used to install devices and gateways on a real
 * ChirpStack network server using the REST API.
 * Requires libcurl-dev installed.
 */
class ChirpStackHelper : public RestApiHelper
{
    using str = std::string;

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
    ChirpStackHelper();

    ~ChirpStackHelper();

    void CloseConnection(int signal) override;

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
    int DoConnect() override;

    int CreateTenant(const str& name);

    int DeleteTenant(const str& id);

    int ListTenantIds(const str& search, std::vector<str>& out);

    int CreateDeviceProfile(const str& name);

    int CreateApplication(const str& name);

    int RegisterPriv(Ptr<Node> node) const;

    int CreateDevice(Ptr<Node> node) const;

    int CreateGateway(Ptr<Node> node) const;

    session_t m_session;
    uint64_t m_run;

    static const struct coord_s m_center;
};

} // namespace lorawan
} // namespace ns3

#endif /* CHIRPSTACK_HELPER_H */
