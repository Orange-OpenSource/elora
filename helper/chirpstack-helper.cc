/*
 * Copyright (c) 2022 Orange SA
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Alessandro Aimi <alessandro.aimi@orange.com>
 *                         <alessandro.aimi@cnam.fr>
 */

#include "chirpstack-helper.h"

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

NS_LOG_COMPONENT_DEFINE("ChirpStackHelper");

const struct coord_s ChirpStackHelper::m_center = {48.866831, 2.356719, 42};

ChirpStackHelper::ChirpStackHelper()
    : m_run(1)
{
    /* Initialize session keys */
    m_session.netKey = "2b7e151628aed2a6abf7158809cf4f3c";
    m_session.appKey = "00000000000000000000000000000000";
}

ChirpStackHelper::~ChirpStackHelper()
{
    CloseConnection(EXIT_SUCCESS);
}

void
ChirpStackHelper::CloseConnection(int signal)
{
    /* Return immediatly if connection was not initialized to begin with */
    if (m_session.tenantId.empty())
    {
        return;
    }

    /* Remove tentant */
    DeleteTenant(m_session.tenantId);

    /* Wipe session data */
    m_session.tenantId.clear();
    m_session.devProfId.clear();
    m_session.appId.clear();

#ifdef NS3_LOG_ENABLE
    std::cout << "\nTear down process terminated after receiving signal " << signal << std::endl;
#endif // NS3_LOG_ENABLE
}

int
ChirpStackHelper::Register(Ptr<Node> node) const
{
    return RegisterPriv(node);
}

int
ChirpStackHelper::Register(NodeContainer c) const
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

int
ChirpStackHelper::CreateHttpIntegration(const str& encoding, const str& endpoint) const
{
    NS_ABORT_MSG_IF(m_session.tenantId.empty(),
                    "Connection was not initialized before registering device");

    str payload = "{"
                  "  \"integration\": {"
                  "    \"encoding\": \"" +
                  encoding + // JSON or PROTOBUF
                  "\","
                  "    \"eventEndpointUrl\": \"" +
                  endpoint + // e.g. http://http-server:8081
                  "\","
                  "    \"headers\": {}"
                  "  }"
                  "}";

    str reply;
    if (POST("/api/applications/" + m_session.appId + "/integrations/http", payload, reply) ==
        EXIT_FAILURE)
    {
        NS_FATAL_ERROR("Unable to register new integration, got reply: " << reply);
    }

    return EXIT_SUCCESS;
}

int
ChirpStackHelper::CreateInfluxDb2Integration(const str& endpoint,
                                             const str& organization,
                                             const str& bucket,
                                             const str& token) const
{
    NS_ABORT_MSG_IF(m_session.tenantId.empty(),
                    "Connection was not initialized before registering device");

    str payload = "{"
                  "  \"integration\": {"
                  "    \"bucket\": \"" +
                  bucket +
                  "\","
                  "    \"db\": \"\"," // unused in InfluxDb v2
                  "    \"endpoint\": \"" +
                  endpoint +
                  "\","
                  "    \"organization\": \"" +
                  organization +
                  "\","
                  "    \"password\": \"\","            // unused in InfluxDb v2
                  "    \"precision\": \"NS\","         // unused in InfluxDb v2
                  "    \"retentionPolicyName\": \"\"," // unused in InfluxDb v2
                  "    \"username\": \"\","            // unused in InfluxDb v2
                  "    \"token\": \"" +
                  token +
                  "\","
                  "    \"version\": \"INFLUXDB_2\""
                  "  }"
                  "}";

    str reply;
    if (POST("/api/applications/" + m_session.appId + "/integrations/influxdb", payload, reply) ==
        EXIT_FAILURE)
    {
        NS_FATAL_ERROR("Unable to register new integration, got reply: " << reply);
    }

    return EXIT_SUCCESS;
}

void
ChirpStackHelper::SetTenant(str& name)
{
    m_session.tenant = name;
}

void
ChirpStackHelper::SetDeviceProfile(str& name)
{
    m_session.devProf = name;
}

void
ChirpStackHelper::SetApplication(str& name)
{
    m_session.app = name;
}

int
ChirpStackHelper::DoConnect()
{
    /* get run identifier */
    m_run = RngSeedManager::GetRun();

    /* Create Ns-3 tenant */
    CreateTenant(m_session.tenant);
    /* Create Ns-3 device profile */
    CreateDeviceProfile(m_session.devProf);
    /* Create Ns-3 application */
    CreateApplication(m_session.app);

    return EXIT_SUCCESS;
}

int
ChirpStackHelper::CreateTenant(const str& name)
{
    /* Clean existing tenants with the same run name */
    std::vector<str> ids;
    ListTenantIds(name + "%20" + std::to_string((unsigned)m_run), ids);
    for (const str& id : ids)
    {
        DeleteTenant(id);
    }

    str payload = "{"
                  "  \"tenant\": {"
                  "    \"canHaveGateways\": true,"
                  "    \"description\": \"\","
                  "    \"id\": \"\","
                  "    \"maxDeviceCount\": 0,"
                  "    \"maxGatewayCount\": 0,"
                  "    \"name\": \"" +
                  name + " " + std::to_string((unsigned)m_run) +
                  "\","
                  "    \"privateGatewaysDown\": false,"
                  "    \"privateGatewaysUp\": false"
                  "  }"
                  "}";

    str reply;
    if (POST("/api/tenants", payload, reply) == EXIT_FAILURE)
    {
        NS_FATAL_ERROR("Unable to register new tenant, got reply: " << reply);
    }

    JSON_Value* json = nullptr;
    json = json_parse_string_with_comments(reply.c_str());
    if (json == nullptr)
    {
        NS_FATAL_ERROR("Invalid JSON in tenant registration reply: " << reply);
    }

    m_session.tenantId = json_object_get_string(json_value_get_object(json), "id");
    json_value_free(json);

    return EXIT_SUCCESS;
}

int
ChirpStackHelper::DeleteTenant(const str& id)
{
    str reply;
    if (DELETE("/api/tenants/" + id, reply) == EXIT_FAILURE)
    {
        NS_FATAL_ERROR("Unable to unregister tenant (id: " << id << "), got reply: " << reply);
    }

    return EXIT_SUCCESS;
}

int
ChirpStackHelper::ListTenantIds(const str& search, std::vector<str>& out)
{
    str reply;
    if (GET("/api/tenants?limit=1000&search=" + search, reply) == EXIT_FAILURE)
    {
        NS_FATAL_ERROR("Unable to list tenants, got reply: " << reply);
    }

    JSON_Value* json = nullptr;
    json = json_parse_string_with_comments(reply.c_str());
    if (json == nullptr)
    {
        NS_FATAL_ERROR("Invalid JSON in list tenant reply: " << reply);
    }

    JSON_Array* array = json_object_get_array(json_value_get_object(json), "result");

    out.clear();
    for (size_t i = 0; i < json_array_get_count(array); ++i)
    {
        out.emplace_back(json_object_get_string(json_array_get_object(array, i), "id"));
    }

    json_value_free(json);

    return EXIT_SUCCESS;
}

int
ChirpStackHelper::CreateDeviceProfile(const str& name)
{
    str payload = "{"
                  "  \"deviceProfile\": {"
                  "    \"abpRx1Delay\": 1,"
                  "    \"abpRx1DrOffset\": 0,"
                  "    \"abpRx2Dr\": 0,"
                  "    \"abpRx2Freq\": 869525000,"
                  "    \"adrAlgorithmId\": \"default\","
                  "    \"autoDetectMeasurements\": true,"
                  "    \"classBPingSlotDr\": 0,"
                  "    \"classBPingSlotFreq\": 0,"
                  "    \"classBPingSlotNbK\": 0,"
                  "    \"classBTimeout\": 0,"
                  "    \"classCTimeout\": 0,"
                  "    \"description\": \"\","
                  "    \"deviceStatusReqInterval\": 0,"
                  "    \"flushQueueOnActivate\": false,"
                  "    \"id\": \"\","
                  "    \"isRelay\": false,"
                  "    \"isRelayEd\": false,"
                  "    \"macVersion\": \"LORAWAN_1_0_4\","
                  "    \"measurements\": {},"
                  "    \"name\": \"" +
                  name +
                  "\","
                  "    \"payloadCodecRuntime\": \"NONE\","
                  "    \"payloadCodecScript\": \"\","
                  "    \"regParamsRevision\": \"RP002_1_0_3\","
                  "    \"region\": \"EU868\","
                  "    \"regionConfigId\": \"\","
                  "    \"relayCadPeriodicity\": \"SEC_1\","
                  "    \"relayDefaultChannelIndex\": 0,"
                  "    \"relayEdActivationMode\": \"DISABLE_RELAY_MODE\","
                  "    \"relayEdBackOff\": 0,"
                  "    \"relayEdRelayOnly\": true,"
                  "    \"relayEdSmartEnableLevel\": 0,"
                  "    \"relayEdUplinkLimitBucketSize\": 0,"
                  "    \"relayEdUplinkLimitReloadRate\": 0,"
                  "    \"relayEnabled\": true,"
                  "    \"relayGlobalUplinkLimitBucketSize\": 0,"
                  "    \"relayGlobalUplinkLimitReloadRate\": 0,"
                  "    \"relayJoinReqLimitBucketSize\": 0,"
                  "    \"relayJoinReqLimitReloadRate\": 0,"
                  "    \"relayNotifyLimitBucketSize\": 0,"
                  "    \"relayNotifyLimitReloadRate\": 0,"
                  "    \"relayOverallLimitBucketSize\": 0,"
                  "    \"relayOverallLimitReloadRate\": 0,"
                  "    \"relaySecondChannelAckOffset\": \"KHZ_0\","
                  "    \"relaySecondChannelDr\": 0,"
                  "    \"relaySecondChannelFreq\": 0,"
                  "    \"supportsClassB\": false,"
                  "    \"supportsClassC\": false,"
                  "    \"supportsOtaa\": false,"
                  "    \"tags\": {},"
                  "    \"tenantId\": \"" +
                  m_session.tenantId +
                  "\","
                  "    \"uplinkInterval\": 86400"
                  "  }"
                  "}";

    str reply;
    if (POST("/api/device-profiles", payload, reply) == EXIT_FAILURE)
    {
        NS_FATAL_ERROR("Unable to register new device profile, got reply: " << reply);
    }

    JSON_Value* json = nullptr;
    json = json_parse_string_with_comments(reply.c_str());
    if (json == nullptr)
    {
        NS_FATAL_ERROR("Invalid JSON in device profile registration reply :" << reply);
    }

    m_session.devProfId = json_object_get_string(json_value_get_object(json), "id");
    json_value_free(json);

    return EXIT_SUCCESS;
}

int
ChirpStackHelper::CreateApplication(const str& name)
{
    str payload = "{"
                  "  \"application\": {"
                  "    \"description\": \"\","
                  "    \"id\": \"\","
                  "    \"name\": \"" +
                  name +
                  "\","
                  "    \"tenantId\": \"" +
                  m_session.tenantId +
                  "\""
                  "  }"
                  "}";

    str reply;
    if (POST("/api/applications", payload, reply) == EXIT_FAILURE)
    {
        NS_FATAL_ERROR("Unable to register new application, got reply: " << reply);
    }

    JSON_Value* json = nullptr;
    json = json_parse_string_with_comments(reply.c_str());
    if (json == nullptr)
    {
        NS_FATAL_ERROR("Invalid JSON in application registration reply: " << reply);
    }

    m_session.appId = json_object_get_string(json_value_get_object(json), "id");
    json_value_free(json);

    return EXIT_SUCCESS;
}

int
ChirpStackHelper::RegisterPriv(Ptr<Node> node) const
{
    NS_LOG_FUNCTION(this << node);
    NS_ABORT_MSG_IF(m_session.tenantId.empty(),
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
ChirpStackHelper::CreateDevice(Ptr<Node> node) const
{
    char eui[17];
    uint64_t id = (m_run << 48) + node->GetId();
    snprintf(eui, 17, "%016lx", id);

    str payload = "{"
                  "  \"device\": {"
                  "    \"applicationId\": \"" +
                  m_session.appId +
                  "\","
                  "    \"description\": \"\","
                  "    \"devEui\": \"" +
                  str(eui) +
                  "\","
                  "    \"deviceProfileId\": \"" +
                  m_session.devProfId +
                  "\","
                  "    \"isDisabled\": false,"
                  "    \"name\": \"Device " +
                  std::to_string((unsigned)id) +
                  "\","
                  "    \"skipFcntCheck\": true,"
                  "    \"tags\": {},"
                  "    \"variables\": {}"
                  "  }"
                  "}";

    str reply;
    if (POST("/api/devices", payload, reply) == EXIT_FAILURE)
    {
        NS_FATAL_ERROR("Unable to register device " << str(eui) << ", reply: " << reply);
    }

    char devAddr[9];
    auto netdev = DynamicCast<LoraNetDevice>(node->GetDevice(0));
    auto mac = DynamicCast<BaseEndDeviceLorawanMac>(netdev->GetMac());
    snprintf(devAddr, 9, "%08x", mac->GetDeviceAddress().Get());

    payload = "{"
              "  \"deviceActivation\": {"
              "    \"aFCntDown\": 0,"
              "    \"appSKey\": \"" +
              m_session.appKey +
              "\","
              "    \"devAddr\": \"" +
              str(devAddr) +
              "\","
              "    \"fCntUp\": 0,"
              "    \"fNwkSIntKey\": \"" +
              m_session.netKey +
              "\","
              "    \"nFCntDown\": 0,"
              "    \"nwkSEncKey\": \"" +
              m_session.netKey +
              "\","
              "    \"sNwkSIntKey\": \"" +
              m_session.netKey +
              "\""
              "  }"
              "}";

    if (POST("/api/devices/" + str(eui) + "/activate", payload, reply) == EXIT_FAILURE)
    {
        NS_FATAL_ERROR("Unable to activate device " << str(eui) << ", reply: " << reply);
    }

    return EXIT_SUCCESS;
}

int
ChirpStackHelper::CreateGateway(Ptr<Node> node) const
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
                  "  \"gateway\": {"
                  "    \"description\": \"\","
                  "    \"gatewayId\": \"" +
                  str(eui) +
                  "\","
                  "    \"location\": {"
                  "      \"accuracy\": 0," +
                  str(coordbuf) +
                  "      \"source\": \"UNKNOWN\""
                  "    },"
                  "    \"metadata\": {}," +
                  "    \"name\": \"Gateway " + std::to_string((unsigned)id) +
                  "\","
                  "    \"statsInterval\": 30,"
                  "    \"properties\": {},"
                  "    \"tags\": {},"
                  "    \"tenantId\": \"" +
                  m_session.tenantId +
                  "\""
                  "  }"
                  "}";

    str reply;
    if (POST("/api/gateways", payload, reply) == EXIT_FAILURE)
    {
        NS_FATAL_ERROR("Unable to register gateway " << str(eui) << ", reply: " << reply);
    }

    return EXIT_SUCCESS;
}

} // namespace lorawan
} // namespace ns3
