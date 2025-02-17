/*
 * Copyright (c) 2024 INSA Lyon
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Carlos Fernandez Hernandez <carlos.fernandez-hernandez@insa-lyon.fr>
 */

#include "TTN-helper.h"

#include "ns3/base-end-device-lorawan-mac.h"
#include "ns3/gateway-lorawan-mac.h"
#include "ns3/log.h"
#include "ns3/lora-net-device.h"
#include "ns3/parson.h"
#include "ns3/rng-seed-manager.h"

namespace ns3
{
namespace lorawan
{

NS_LOG_COMPONENT_DEFINE("TTNHelper");

const struct coord_s TTNHelper::m_center = {48.866831, 2.356719, 42};

TTNHelper::TTNHelper()
    : m_run(1)
{
    m_url = "http://localhost:8885";

    /* Initialize token */
    m_token = "NNSXS.CI4KYAD4WEVQUQHTBI77DPQQ2WN2IROIKWRCLDA."
              "TEXTNWIJMJ4Z5LSGJVBBKJ72QAL3S3GAQDHFTTOR4GUDZI2TCHWQ";
    /* Initialize session keys */
    m_session.netKey = "2b7e151628aed2a6abf7158809cf4f3c";
    m_session.appKey = "2b7e151628aed2a6abf7158809cf4f3c";
}

TTNHelper::~TTNHelper()
{
    CloseConnection(EXIT_SUCCESS);
}

int
TTNHelper::InitConnection(const str address, uint16_t port, const str token)
{
    NS_LOG_FUNCTION(this << address << (unsigned)port);

    /* Setup base URL string with IP and port */
    m_url = address + ":" + std::to_string(port);
    NS_LOG_INFO("TTN REST API URL set to: " << m_url);

    /* set API token */
    m_token = token;

    /* Initialize HTTP header fields */
    curl_slist_free_all(m_header); /* free the header list if previously set */
    NS_ASSERT_MSG(!m_token.empty(), "API token was not set.");
    m_header = curl_slist_append(m_header, ("Authorization: Bearer " + m_token).c_str());
    m_header = curl_slist_append(m_header, "Accept: application/json");
    m_header = curl_slist_append(m_header, "Content-Type: application/json");
    NS_LOG_INFO("Curl Header " << m_header);
    /* get run identifier */
    m_run = RngSeedManager::GetRun();

    return DoConnect();
}

void
TTNHelper::CloseConnection(int signal) const
{
    str reply;
    // uint64_t id;
    /* Delete all ED */
    NS_LOG_DEBUG("Starting Delete Devices");

    for (int i = 0; i < m_session.nDevices; i++)
    {
        int n_ed = i + m_session.nGateway + 1;
        NS_LOG_INFO("Deleting device-" << std::to_string((unsigned)n_ed));
        if (DELETE("/api/v3/ns/applications/" + str(m_session.appId) + "/devices/device-" +
                       std::to_string((unsigned)n_ed),
                   reply) == EXIT_FAILURE)
        {
            NS_LOG_ERROR("Unable to unregister device from NS, got reply: " << reply);
        }
        if (DELETE("/api/v3/as/applications/" + str(m_session.appId) + "/devices/device-" +
                       std::to_string((unsigned)n_ed),
                   reply) == EXIT_FAILURE)
        {
            NS_LOG_ERROR("Unable to unregister device from AS, got reply: " << reply);
        }
        if (DELETE("/api/v3/applications/" + str(m_session.appId) + "/devices/device-" +
                       std::to_string((unsigned)n_ed),
                   reply) == EXIT_FAILURE)
        {
            NS_LOG_ERROR("Unable to unregister device from Stack, got reply: " << reply);
        }
    }

    for (int j = 0; j < m_session.nGateway; j++)
    {
        int n_gw = j + 1;
        if (DELETE("/api/v3/gateways/gw-" + std::to_string((unsigned)n_gw), reply) == EXIT_FAILURE)
        {
            NS_LOG_ERROR(
                "Unable to delete gw-" + std::to_string((unsigned)n_gw) + ", got reply: " << reply);
        }
        if (DELETE("/api/v3/gateways/gw-" + std::to_string((unsigned)n_gw) + "/purge", reply) ==
            EXIT_FAILURE)
        {
            NS_LOG_ERROR(
                "Unable to purge gw-" + std::to_string((unsigned)n_gw) + ", got reply: " << reply);
        }
    }

    /* Terminate curl */
    curl_global_cleanup();

    curl_slist_free_all(m_header); /* free the header list */

#ifdef NS3_LOG_ENABLE
    std::cout << "\nTear down process terminated after receiving signal " << signal << std::endl;
#endif // NS3_LOG_ENABLE
}

int
TTNHelper::Register(Ptr<Node> node) const
{
    return RegisterPriv(node);
}

int
TTNHelper::Register(NodeContainer c) const
{
    for (NodeContainer::Iterator i = c.Begin(); i != c.End(); ++i)
    {
        if (RegisterPriv(*i) == EXIT_FAILURE)
        {
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

void
TTNHelper::SetNodes(int n, int gw)
{
    m_session.nDevices = n;
    m_session.nGateway = gw;
}

void
TTNHelper::SetApp(str& name)
{
    m_session.tenant = name;
}

void
TTNHelper::SetDeviceProfile(str& name)
{
    m_session.devProf = name;
}

void
TTNHelper::SetApplication(str& name)
{
    m_session.app = name;
}

int
TTNHelper::DoConnect()
{
    /* Init curl */
    curl_global_init(CURL_GLOBAL_NOTHING);
    /* Create Ns-3 application */
    m_session.appId = m_session.app + std::to_string((unsigned)m_run) + "-test";
    NewApplication(m_session.app);

    return EXIT_SUCCESS;
}

// std::to_string((unsigned)m_run)

int
TTNHelper::NewApplication(const str& name)
{
    m_session.appId = name + std::to_string((unsigned)m_run) + "-test";
    NS_LOG_DEBUG("Name:" << m_session.appId);
    str payload =
        "{"
        "  \"application\": {                                                             "
        "   \"ids\": {                                                                    "
        "\"application_id\" : \"" +
        m_session.appId +
        "\"   "
        "},                                                                               "
        "   \"name\": \"eloratest\",                                                      "
        "   \"description\": \"Application for my test devices \",                        "
        "    \"network_server_address\": \"localhost\",                        "
        "    \"application_server_address\": \"localhost\",                        "
        "    \"join_server_address\": \"localhost\"                                      "
        "  }"
        "}";
    str reply;
    if (POST("/api/v3/users/admin/applications", payload, reply) == EXIT_FAILURE)
    {
        NS_FATAL_ERROR("Unable to register new application, got reply: " << reply);
    }
    NS_LOG_DEBUG("Came Back");

    JSON_Value* json = nullptr;
    json = json_parse_string_with_comments(reply.c_str());
    if (json == nullptr)
    {
        NS_FATAL_ERROR("Invalid JSON in device profile registration reply: " << reply);
    }

    NS_LOG_DEBUG("appId:" << m_session.appId);

    json_value_free(json);

    return EXIT_SUCCESS;
}

int
TTNHelper::RegisterPriv(Ptr<Node> node) const
{
    NS_LOG_FUNCTION(this << node);

    Ptr<LoraNetDevice> netdev;
    // We assume nodes can have at max 1 LoraNetDevice
    for (int i = 0; i < (int)node->GetNDevices(); ++i)
    {
        if (netdev = DynamicCast<LoraNetDevice>(node->GetDevice(i)); bool(netdev))
        {
            if (bool(DynamicCast<BaseEndDeviceLorawanMac>(netdev->GetMac())))
            {
                NewDevice(node);
            }
            else if (bool(DynamicCast<GatewayLorawanMac>(netdev->GetMac())))
            {
                NewGateway(node);
            }
            else
            {
                NS_FATAL_ERROR("No LorawanMac installed (node id: " << (unsigned)node->GetId()
                                                                    << ")");
            }
            return EXIT_SUCCESS;
        }
    }

    NS_LOG_DEBUG("No LoraNetDevice installed (node id: " << (unsigned)node->GetId() << ")");
    return EXIT_FAILURE;
}

int
TTNHelper::NewDevice(Ptr<Node> node) const
{
    NS_LOG_DEBUG("ED(session prof ID: " << str(m_session.devProfId) << ")");
    char eui[17];
    uint64_t id = (m_run << 48) + node->GetId();

    snprintf(eui, 17, "%016lx", id);
    NS_LOG_DEBUG("ED(session prof ID: " << str(m_session.appId) << ")");

    str payload = "{"
                  "\"end_device\": {"
                  "\"ids\": {"
                  "\"device_id\": \"device-" +
                  std::to_string((unsigned)id) +
                  "\","
                  "\"dev_eui\": \"" +
                  str(eui) +
                  "\""
                  "},"
                  "\"join_server_address\": \"localhost\","
                  "\"network_server_address\": \"localhost\","
                  "\"application_server_address\": \"localhost\""
                  "},"
                  "\"field_mask\": {"
                  "\"paths\": ["
                  "\"join_server_address\","
                  "\"network_server_address\","
                  "\"application_server_address\","
                  "\"ids.dev_eui\" ]"
                  "}"
                  "}";

    str reply;
    if (POST("/api/v3/applications/" + str(m_session.appId) + "/devices", payload, reply) ==
        EXIT_FAILURE)
    {
        NS_FATAL_ERROR("Unable to register device " << str(eui) << ", reply: " << reply);
    }

    char devAddr[9];
    auto netdev = DynamicCast<LoraNetDevice>(node->GetDevice(0));
    auto mac = DynamicCast<BaseEndDeviceLorawanMac>(netdev->GetMac());
    snprintf(devAddr, 9, "%08x", mac->GetDeviceAddress().Get());
    NS_LOG_DEBUG("DeviceAddr" << str(devAddr) << ")");
    NS_LOG_DEBUG("m_session.netKey" << str(m_session.netKey) << ")");
    NS_LOG_DEBUG("m_session.appKey" << str(m_session.appKey) << ")");

    payload = "{"
              "\"end_device\": {"
              "\"supports_join\": false,"
              "\"lorawan_version\": \"1.0.4\","
              "\"ids\": {"
              "\"device_id\": \"device-" +
              std::to_string((unsigned)id) +
              "\","
              "\"dev_eui\": \"" +
              str(eui) +
              "\""
              "},"
              "\"session\": {"
              " \"keys\": {"
              "\"f_nwk_s_int_key\":{"
              "\"key\": \"" +
              m_session.netKey +
              "\""
              "}"
              "},"
              "\"dev_addr\":\"" +
              str(devAddr) +
              "\""
              "},"
              "\"mac_settings\": {"
              "\"resets_f_cnt\": true,"
              "\"factory_preset_frequencies\": ["
              "\"868100000\","
              "\"868300000\","
              "\"868500000\","
              "\"867100000\","
              "\"867300000\","
              "\"867500000\","
              "\"867700000\","
              "\"867900000\""
              "],"
              "\"desired_rx1_delay\": \"RX_DELAY_1\","
              "\"status_count_periodicity\": 0,"
              "\"status_time_periodicity\": \"0s\","
              "\"adr\": {\"disabled\": {}}"

              "},"

              "\"resets_f_cnt\": true,"
              "\"lorawan_phy_version\": \"RP002_V1_0_3\"," // RP002_V1_0_3
              "\"frequency_plan_id\": \"EU_863_870\""
              "},"
              "\"field_mask\": {"
              " \"paths\": ["
              "\"supports_join\","
              " \"lorawan_version\","
              "\"ids.device_id\","
              "\"ids.dev_eui\","
              "\"session.keys.f_nwk_s_int_key.key\","
              "\"session.dev_addr\","
              "\"mac_settings.resets_f_cnt\","
              "\"mac_settings.factory_preset_frequencies\","
              "\"mac_settings.desired_rx1_delay\","
              "\"mac_settings.adr\","
              "\"mac_settings.status_count_periodicity\","
              "\"mac_settings.status_time_periodicity\","
              "\"lorawan_phy_version\","
              "\"frequency_plan_id\"]"
              "}"
              "}";

    if (PUT("/api/v3/ns/applications/" + str(m_session.appId) + "/devices/device-" +
                std::to_string((unsigned)id),
            payload,
            reply) == EXIT_FAILURE)
    {
        NS_FATAL_ERROR("Unable to activate device device-" << std::to_string((unsigned)id)
                                                           << ", reply: " << reply);
    }

    payload = "{"
              "\"end_device\": {"
              "\"ids\": {"
              " \"device_id\": \"device-" +
              std::to_string((unsigned)id) +
              "\","
              "\"dev_eui\": \"" +
              str(eui) +
              "\""
              "},"
              "\"session\": {"
              "\"keys\": {"
              "\"app_s_key\": {"
              "\"key\": \"" +
              m_session.appKey +
              "\""
              "}"
              "},"
              "\"dev_addr\": \"" +
              str(devAddr) +
              "\""
              " },"
              "\"skip_payload_crypto\": true"
              "},"
              "\"field_mask\": {"
              "\"paths\": ["
              "\"ids.device_id\","
              "\"ids.dev_eui\","
              "\"session.keys.app_s_key.key\","
              "\"session.dev_addr\","
              "\"skip_payload_crypto\""
              "]"
              "}"
              "}";

    if (PUT("/api/v3/as/applications/" + str(m_session.appId) + "/devices/device-" +
                std::to_string((unsigned)id),
            payload,
            reply) == EXIT_FAILURE)
    {
        NS_FATAL_ERROR("Unable to activate device device-" << std::to_string((unsigned)id)
                                                           << ", reply: " << reply);
    }

    return EXIT_SUCCESS;
}

int
TTNHelper::NewGateway(Ptr<Node> node) const
{
    char eui[17];
    uint64_t id = (m_run << 48) + node->GetId();
    snprintf(eui, 17, "%016lx", id);

    /* get reference coordinates */
    coord_s coord;
    double r_earth = 6371000.0;
    Vector position = node->GetObject<MobilityModel>()->GetPosition();
    coord.alt = m_center.alt + (short)position.z;
    coord.lat = m_center.lat + (position.y / r_earth) * (180.0 / M_PI);
    coord.lon =
        m_center.lon + (position.x / r_earth) * (180.0 / M_PI) / cos(m_center.lat * M_PI / 180.0);
    char coordbuf[100];
    snprintf(coordbuf,
             100,
             "\"altitude\":%i,\"latitude\":%.5f,\"longitude\":%.5f,",
             coord.alt,
             coord.lat,
             coord.lon);

    str payload = "{"
                  "  \"gateway\": {                                                                "
                  "         \"ids\": {                                                             "
                  "         \"eui\": \"" +
                  str(eui) +
                  "\",                                     "
                  "         \"gateway_id\":\"gw-" +
                  std::to_string((unsigned)id) +
                  "\"          "
                  "                  },                                                            "
                  "    \"name\": \"gw-" +
                  std::to_string((unsigned)id) +
                  "\",          "
                  "    \"frequency_plan_ids\": [\"EU_863_870\"],                                   "
                  "    \"require_authenticated_connection\": false,                                "
                  "    \"status_public\": false,                                                   "
                  "    \"location_public\": false,                                                 "
                  "    \"gateway_server_address\": \"localhost\",        "
                  "    \"enforce_duty_cycle\": true                                                "
                  " }"
                  "}";

    str reply;
    if (POST("/api/v3/users/admin/gateways", payload, reply) == EXIT_FAILURE)
    {
        NS_FATAL_ERROR("Unable to register gateway " << str(eui) << ", reply: " << reply);
    }

    return EXIT_SUCCESS;
}

int
TTNHelper::PUT(const str& path, const str& body, str& out) const
{
    CURL* curl;
    CURLcode res;
    std::stringstream ss;

    /* get a curl handle */
    curl = curl_easy_init();
    if (curl)
    {
        /* Set the URL that is about to receive our POST. */
        curl_easy_setopt(curl, CURLOPT_URL, (m_url + path).c_str());

        /* Specify the HEADER content */
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, m_header);

        /* DELETE the given path */
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");

        /* Set reply stringstream */

        if (!body.empty())
        {
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)body.size());
        }
        /* Set reply stringstream */
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, (void*)StreamWriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&ss);

        NS_LOG_INFO("Sending PUT request to " << m_url << path << ", with body: " << body);
        /* Perform the request, res will get the return code */
        res = curl_easy_perform(curl);

        /* always cleanup */
        curl_easy_cleanup(curl);
    }
    else
    {
        NS_LOG_ERROR("curl_easy_init() failed\n");
        return EXIT_FAILURE;
    }

    out = ss.str();
    NS_LOG_INFO("Received PUT reply: " << out);

    /* Check for errors */
    if (res != CURLE_OK)
    {
        NS_LOG_ERROR("curl_easy_perform() failed: " << curl_easy_strerror(res) << "\n");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int
TTNHelper::POST(const str& path, const str& body, str& out) const
{
    CURL* curl;
    CURLcode res;
    std::stringstream ss;

    /* get a curl handle */
    curl = curl_easy_init();
    if (curl)
    {
        /* Set the URL that is about to receive our POST. */
        curl_easy_setopt(curl, CURLOPT_URL, (m_url + path).c_str());

        /* Specify the HEADER content */
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, m_header);

        /* Add body, if present */
        if (!body.empty())
        {
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)body.size());
        }

        /* Set reply stringstream */
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, (void*)StreamWriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&ss);

        NS_LOG_INFO("Sending POST request to " << m_url << path << ", with body: " << body);
        /* Perform the request, res will get the return code */
        NS_LOG_INFO("Request:" << body.c_str());

        res = curl_easy_perform(curl);
        NS_LOG_INFO("Result request:" << res);

        /* always cleanup */
        curl_easy_cleanup(curl);
    }
    else
    {
        NS_LOG_ERROR("curl_easy_init() failed\n");
        return EXIT_FAILURE;
    }

    out = ss.str();
    NS_LOG_INFO("Received POST reply: " << out);

    /* Check for errors */
    if (res != CURLE_OK)
    {
        NS_LOG_ERROR("curl_easy_perform() failed: " << curl_easy_strerror(res) << "\n");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int
TTNHelper::DELETE(const str& path, str& out) const
{
    CURL* curl;
    CURLcode res;
    std::stringstream ss;

    /* get a curl handle */
    curl = curl_easy_init();
    if (curl)
    {
        /* Set the URL that is about to receive our POST. */
        curl_easy_setopt(curl, CURLOPT_URL, (m_url + path).c_str());

        /* Specify the HEADER content */
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, m_header);

        /* DELETE the given path */
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");

        /* Set reply stringstream */
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, (void*)StreamWriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&ss);

        NS_LOG_INFO("Sending DELETE request to " << m_url << path);
        /* Perform the request, res will get the return code */
        res = curl_easy_perform(curl);

        /* always cleanup */
        curl_easy_cleanup(curl);
    }
    else
    {
        NS_LOG_ERROR("curl_easy_init() failed\n");
        return EXIT_FAILURE;
    }

    out = ss.str();
    NS_LOG_INFO("Received DELETE reply: " << out);

    /* Check for errors */
    if (res != CURLE_OK)
    {
        NS_LOG_ERROR("curl_easy_perform() failed: " << curl_easy_strerror(res) << "\n");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

size_t
TTNHelper::StreamWriteCallback(char* buffer, size_t size, size_t nitems, std::ostream* stream)
{
    size_t realwrote = size * nitems;
    stream->write(buffer, static_cast<std::streamsize>(realwrote));
    if (!(*stream))
    {
        realwrote = 0;
    }

    return realwrote;
}

} // namespace lorawan
} // namespace ns3
