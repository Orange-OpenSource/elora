/*
 * Copyright (c) 2024 INSA Lyon
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Carlos Fernandez Hernandez <carlos.fernandez-hernandez@insa-lyon.fr>
 */

#include "the-things-stack-helper.h"

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

NS_LOG_COMPONENT_DEFINE("TheThingsStackHelper");

TheThingsStackHelper::TheThingsStackHelper()
    : m_run(1)
{
    /* Initialize session keys */
    m_session.netKey = "2b7e151628aed2a6abf7158809cf4f3c";
    m_session.appKey = "ffffffffffffffffffffffffffffffff";
}

TheThingsStackHelper::~TheThingsStackHelper()
{
    CloseConnection(EXIT_SUCCESS);
}

void
TheThingsStackHelper::CloseConnection(int signal)
{
    /* Return immediatly if connection was not initialized to begin with */
    if (m_session.appId.empty())
    {
        return;
    }

    str reply;

    /* Delete all devices */
    for (const auto& devId : m_session.devIds)
    {
        if (DELETE("/api/v3/ns/applications/" + m_session.appId + "/devices/" + devId, reply) ==
            EXIT_FAILURE)
        {
            NS_LOG_ERROR("Unable to unregister device from NS, got reply: " << reply);
        }
        if (DELETE("/api/v3/as/applications/" + m_session.appId + "/devices/" + devId, reply) ==
            EXIT_FAILURE)
        {
            NS_LOG_ERROR("Unable to unregister device from AS, got reply: " << reply);
        }
        if (DELETE("/api/v3/applications/" + m_session.appId + "/devices/" + devId, reply) ==
            EXIT_FAILURE)
        {
            NS_LOG_ERROR("Unable to unregister device IS, got reply: " << reply);
        }
    }

    /* Purge all gateways */
    for (const auto& gwId : m_session.gwIds)
    {
        if (DELETE("/api/v3/gateways/" + gwId + "/purge", reply) == EXIT_FAILURE)
        {
            NS_LOG_ERROR("Unable to purge gateway, got reply: " << reply);
        }
    }

    /* Purge application */
    if (DELETE("/api/v3/applications/" + m_session.appId + "/purge", reply) == EXIT_FAILURE)
    {
        NS_LOG_ERROR("Unable to purge application, got reply: " << reply);
    }

    /* Wipe session data */
    m_session.devIds.clear();
    m_session.gwIds.clear();
    m_session.appId.clear();

#ifdef NS3_LOG_ENABLE
    std::cout << "\nTear down process terminated after receiving signal " << signal << std::endl;
#endif // NS3_LOG_ENABLE
}

int
TheThingsStackHelper::Register(Ptr<Node> node)
{
    return RegisterPriv(node);
}

int
TheThingsStackHelper::Register(NodeContainer c)
{
    for (auto i = c.Begin(); i != c.End(); ++i)
    {
        if (RegisterPriv(*i) == EXIT_FAILURE)
        {
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

void
TheThingsStackHelper::SetApplication(str& name)
{
    m_session.app = name;
}

int
TheThingsStackHelper::DoConnect()
{
    /* get run identifier */
    m_run = RngSeedManager::GetRun();

    /* Create Ns-3 application */
    CreateApplication(m_session.app);

    return EXIT_SUCCESS;
}

int
TheThingsStackHelper::CreateApplication(const str& name)
{
    char buf[21];
    snprintf(buf, 21, "%020ld", m_run);
    str appId(buf);

    str payload = "{"
                  "  \"application\": {"
                  "    \"ids\": {"
                  "      \"application_id\" : \"" +
                  appId +
                  "\""
                  "    },"
                  "    \"name\": \"" +
                  name + " " + std::to_string((unsigned)m_run) +
                  "\","
                  "    \"description\": \"Application for my test devices \","
                  "    \"network_server_address\": \"localhost\","
                  "    \"application_server_address\": \"localhost\","
                  "    \"join_server_address\": \"localhost\""
                  "  }"
                  "}";

    str reply;
    if (POST("/api/v3/users/admin/applications", payload, reply) == EXIT_FAILURE)
    {
        NS_FATAL_ERROR("Unable to register new application, got reply: " << reply);
    }

    JSON_Value* json = nullptr;
    json = json_parse_string_with_comments(reply.c_str());
    if (json == nullptr)
    {
        NS_FATAL_ERROR("Invalid JSON in application registration reply: " << reply);
    }

    // Validate expected response format
    m_session.appId =
        json_object_get_string(json_object_get_object(json_value_get_object(json), "ids"),
                               "application_id");
    json_value_free(json);

    return EXIT_SUCCESS;
}

int
TheThingsStackHelper::RegisterPriv(Ptr<Node> node)
{
    NS_LOG_FUNCTION(this << node);
    NS_ABORT_MSG_IF(m_session.appId.empty(),
                    "Connection was not initialized before registering device");

    Ptr<LoraNetDevice> netdev;
    // We assume nodes can have at max 1 LoraNetDevice
    for (int i = 0; i < (int)node->GetNDevices(); ++i)
    {
        if (netdev = DynamicCast<LoraNetDevice>(node->GetDevice(i)); bool(netdev))
        {
            if (bool(DynamicCast<BaseEndDeviceLorawanMac>(netdev->GetMac())))
            {
                CreateDevice(node);
            }
            else if (bool(DynamicCast<GatewayLorawanMac>(netdev->GetMac())))
            {
                CreateGateway(node);
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
TheThingsStackHelper::CreateDevice(Ptr<Node> node)
{
    char eui[17];
    uint64_t id = (m_run << 48) + node->GetId();
    snprintf(eui, 17, "%016lx", id);

    str payload = "{"
                  "  \"end_device\": {"
                  "    \"ids\": {"
                  "      \"device_id\": \"" +
                  str(eui) +
                  "\","
                  "      \"dev_eui\": \"" +
                  str(eui) +
                  "\""
                  "    },"
                  "    \"name\": \"Device " +
                  std::to_string((unsigned)id) +
                  "\","
                  "    \"join_server_address\": \"localhost\","
                  "    \"network_server_address\": \"localhost\","
                  "    \"application_server_address\": \"localhost\""
                  "  },"
                  "  \"field_mask\": {"
                  "    \"paths\": ["
                  "      \"join_server_address\","
                  "      \"network_server_address\","
                  "      \"application_server_address\","
                  "      \"ids.dev_eui\""
                  "    ]"
                  "  }"
                  "}";

    str reply;
    if (POST("/api/v3/applications/" + m_session.appId + "/devices", payload, reply) ==
        EXIT_FAILURE)
    {
        NS_FATAL_ERROR("Unable to register device in IS, reply: " << reply);
    }

    JSON_Value* json = nullptr;
    json = json_parse_string_with_comments(reply.c_str());
    if (json == nullptr)
    {
        NS_FATAL_ERROR("Invalid JSON in application registration reply: " << reply);
    }

    // Validate expected response format
    m_session.devIds.emplace_back(
        json_object_get_string(json_object_get_object(json_value_get_object(json), "ids"),
                               "device_id"));
    json_value_free(json);

    char devAddr[9];
    auto netdev = DynamicCast<LoraNetDevice>(node->GetDevice(0));
    auto mac = DynamicCast<BaseEndDeviceLorawanMac>(netdev->GetMac());
    snprintf(devAddr, 9, "%08x", mac->GetDeviceAddress().Get());

    payload = "{"
              "  \"end_device\": {"
              "    \"supports_join\": false,"
              "    \"lorawan_version\": \"1.0.4\","
              "    \"ids\": {"
              "      \"device_id\": \"" +
              str(eui) +
              "\","
              "      \"dev_eui\": \"" +
              str(eui) +
              "\""
              "    },"
              "    \"session\": {"
              "      \"keys\": {"
              "        \"f_nwk_s_int_key\":{"
              "          \"key\": \"" +
              m_session.netKey +
              "\""
              "        }"
              "      },"
              "      \"dev_addr\":\"" +
              str(devAddr) +
              "\""
              "    },"
              "    \"mac_settings\": {"
              "      \"resets_f_cnt\": true,"
              "      \"factory_preset_frequencies\": ["
              "        \"868100000\","
              "        \"868300000\","
              "        \"868500000\""
              "      ],"
              "      \"desired_rx1_delay\": \"RX_DELAY_1\","
              "      \"status_count_periodicity\": 0,"
              "      \"status_time_periodicity\": \"0s\","
              "      \"adr\": {"
              "        \"disabled\": {}"
              "      }"
              "    },"
              "    \"resets_f_cnt\": true,"
              "    \"lorawan_phy_version\": \"RP002_V1_0_3\"," // RP002_V1_0_3
              "    \"frequency_plan_id\": \"EU_863_870\""
              "  },"
              "  \"field_mask\": {"
              "    \"paths\": ["
              "      \"supports_join\","
              "      \"lorawan_version\","
              "      \"ids.device_id\","
              "      \"ids.dev_eui\","
              "      \"session.keys.f_nwk_s_int_key.key\","
              "      \"session.dev_addr\","
              "      \"mac_settings.resets_f_cnt\","
              "      \"mac_settings.factory_preset_frequencies\","
              "      \"mac_settings.desired_rx1_delay\","
              "      \"mac_settings.adr\","
              "      \"mac_settings.status_count_periodicity\","
              "      \"mac_settings.status_time_periodicity\","
              "      \"lorawan_phy_version\","
              "      \"frequency_plan_id\""
              "    ]"
              "  }"
              "}";

    if (PUT("/api/v3/ns/applications/" + m_session.appId + "/devices/" + str(eui),
            payload,
            reply) == EXIT_FAILURE)
    {
        NS_FATAL_ERROR("Unable to activate device in NS, reply: " << reply);
    }

    payload = "{"
              "  \"end_device\": {"
              "    \"ids\": {"
              "      \"device_id\": \"device-" +
              std::to_string((unsigned)id) +
              "\","
              "      \"dev_eui\": \"" +
              str(eui) +
              "\""
              "    },"
              "    \"session\": {"
              "      \"keys\": {"
              "        \"app_s_key\": {"
              "          \"key\": \"" +
              m_session.appKey +
              "\""
              "        }"
              "      },"
              "      \"dev_addr\": \"" +
              str(devAddr) +
              "\""
              "    },"
              "    \"skip_payload_crypto\": true"
              "  },"
              "  \"field_mask\": {"
              "    \"paths\": ["
              "      \"ids.device_id\","
              "      \"ids.dev_eui\","
              "      \"session.keys.app_s_key.key\","
              "      \"session.dev_addr\","
              "      \"skip_payload_crypto\""
              "    ]"
              "  }"
              "}";

    if (PUT("/api/v3/as/applications/" + m_session.appId + "/devices/" + str(eui),
            payload,
            reply) == EXIT_FAILURE)
    {
        NS_FATAL_ERROR("Unable to activate device in AS, reply: " << reply);
    }

    return EXIT_SUCCESS;
}

int
TheThingsStackHelper::CreateGateway(Ptr<Node> node)
{
    char eui[17];
    uint64_t id = (m_run << 48) + node->GetId();
    snprintf(eui, 17, "%016lx", id);

    str payload = "{"
                  "  \"gateway\": {"
                  "    \"ids\": {"
                  "      \"eui\": \"" +
                  str(eui) +
                  "\","
                  "      \"gateway_id\":\"" +
                  str(eui) +
                  "\""
                  "    },"
                  "    \"name\": \"Gateway " +
                  std::to_string((unsigned)id) +
                  "\","
                  "    \"frequency_plan_ids\": [\"EU_863_870\"],"
                  "    \"require_authenticated_connection\": false,"
                  "    \"status_public\": false,"
                  "    \"location_public\": false,"
                  "    \"gateway_server_address\": \"localhost\","
                  "    \"enforce_duty_cycle\": true"
                  "  }"
                  "}";

    str reply;
    if (POST("/api/v3/users/admin/gateways", payload, reply) == EXIT_FAILURE)
    {
        NS_FATAL_ERROR("Unable to register gateway, reply: " << reply);
    }

    JSON_Value* json = nullptr;
    json = json_parse_string_with_comments(reply.c_str());
    if (json == nullptr)
    {
        NS_FATAL_ERROR("Invalid JSON in application registration reply: " << reply);
    }

    // Validate expected response format
    m_session.gwIds.emplace_back(
        json_object_get_string(json_object_get_object(json_value_get_object(json), "ids"),
                               "gateway_id"));
    json_value_free(json);

    return EXIT_SUCCESS;
}

} // namespace lorawan
} // namespace ns3
