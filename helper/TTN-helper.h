/*
 * Copyright (c) 2022 Orange SA
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * 
 * Based on the implementation for Chirpstack created by Author: Alessandro Aimi <alessandro.aimi@orange.com>
 *                                                              <alessandro.aimi@cnam.fr>
 * Author of this implementation: Carlos Fernandez Hernandez
 *                                <carlos.fernandez-hernandez@insa-lyon.fr>
 * Received support from the Project ANR-21-CE25-0002-01    
 */

#ifndef TTN_HELPER_H
#define TTN_HELPER_H

#include "ns3/loragw_hal.h"
#include "ns3/node-container.h"

#include <curl/curl.h>

namespace ns3
{
namespace lorawan
{

/**
 * This class can be used to install devices and gateways on a real
 * TTN network server using the REST API.
 * Requires libcurl-dev installed.
 */
class TTNHelper
{
    using str = std::string;
    using query_t = std::vector<std::pair<str, str>>;

    struct session_t
    {
        // Registration info
        str tenant = "Ns-3 Simulator";
        str devProf = "Ns-3 Device Profile";
        str app = "ns-3-application";

        // Session IDs
        str tenantId;
        str devProfId;
        str appId;
        int nDevices;
        int nGateway;

        // Session keys
        str netKey;
        str appKey;
    };

  public:
    TTNHelper();

    ~TTNHelper();

    int InitConnection(const str address, uint16_t port, const str token);

    void CloseConnection(int signal) const;

    int Register(NodeContainer c) const;

    int Register(Ptr<Node> node) const;

    void SetApp(str& name);

    void SetDeviceProfile(str& name);

    void SetApplication(str& name);
    void SetNodes(int n,int gw);

  private:
    int DoConnect();


    int NewApplication(const str& name);

    int RegisterPriv(Ptr<Node> node) const;

    int NewDevice(Ptr<Node> node) const;

    int NewGateway(Ptr<Node> node) const;

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

    static const struct coord_s m_center;
};

} // namespace lorawan

} // namespace ns3
#endif /* TTN_HELPER_H */
