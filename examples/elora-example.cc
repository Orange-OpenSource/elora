/*
 * This program produces real-time traffic to an external chirpstack server.
 * Key elements are preceded by a comment with lots of dashes ( ///////////// )
 */

// ns3 imports
#include "ns3/core-module.h"
#include "ns3/csma-helper.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/mobility-helper.h"
#include "ns3/okumura-hata-propagation-loss-model.h"
#include "ns3/propagation-delay-model.h"
#include "ns3/tap-bridge-helper.h"

// lorawan imports
#include "ns3/chirpstack-helper.h"
#include "ns3/hex-grid-position-allocator.h"
#include "ns3/lorawan-helper.h"
#include "ns3/periodic-sender-helper.h"
#include "ns3/range-position-allocator.h"
#include "ns3/udp-forwarder-helper.h"
#include "ns3/urban-traffic-helper.h"

// cpp imports
#include <unordered_map>

using namespace ns3;
using namespace lorawan;

NS_LOG_COMPONENT_DEFINE("EloraExample");
#include "utilities.cc"

/* Global declaration of connection helper for signal handling */
ChirpstackHelper csHelper;

int
main(int argc, char* argv[])
{
    /***************************
     *  Simulation parameters  *
     ***************************/

    std::string tenant = "ELoRa";
    std::string apiAddr = "127.0.0.1";
    uint16_t apiPort = 8090;
    std::string token =
        "eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9."
        "eyJhdWQiOiJjaGlycHN0YWNrIiwiaXNzIjoiY2hpcnBzdGFjayIsInN1YiI6IjZlMjQ4NjljLWQxMjItNDZkOS04Nz"
        "E0LTM5Yzc4Nzg4OTRhZCIsInR5cCI6ImtleSJ9.IB20o6Jrcwj5qZ9mPEuthzzqMyc3YNSl8by_ZXrjqhw";
    uint16_t destPort = 1700;

    double periods = 24; // H * D
    int gatewayRings = 1;
    double range = 2540.25; // Max range for downlink (!) coverage probability > 0.98 (with okumura)
    int nDevices = 1;
    std::string sir = "CROCE";
    bool initializeSF = true;
    bool testDev = false;
    bool file = false; // Warning: will produce a file for each gateway
    bool log = false;

    /* Expose parameters to command line */
    {
        CommandLine cmd(__FILE__);
        cmd.AddValue("tenant", "Chirpstack tenant name of this simulation", tenant);
        cmd.AddValue("apiAddr", "Chirpstack REST API endpoint IP address", apiAddr);
        cmd.AddValue("apiPort", "Chirpstack REST API endpoint IP address", apiPort);
        cmd.AddValue("token", "Chirpstack API token (to be generated in Chirpstack UI)", token);
        cmd.AddValue("destPort", "Port used by the Chirpstack Gateway Bridge", destPort);
        cmd.AddValue("periods", "Number of periods to simulate (1 period = 1 hour)", periods);
        cmd.AddValue("rings", "Number of gateway rings in hexagonal topology", gatewayRings);
        cmd.AddValue("range", "Radius of the device allocation disk around a gateway)", range);
        cmd.AddValue("devices", "Number of end devices to include in the simulation", nDevices);
        cmd.AddValue("sir", "Signal to Interference Ratio matrix used for interference", sir);
        cmd.AddValue("initSF", "Whether to initialize the SFs", initializeSF);
        cmd.AddValue("adr", "ns3::BaseEndDeviceLorawanMac::ADRBit");
        cmd.AddValue("test", "Use test devices (5s period, 5B payload)", testDev);
        cmd.AddValue("file", "Whether to enable .pcap tracing on gateways", file);
        cmd.AddValue("log", "Whether to enable logs", log);
        cmd.Parse(argc, argv);
    }

    /* Apply global configurations */
    ///////////////// Real-time operation, necessary to interact with the outside world.
    GlobalValue::Bind("SimulatorImplementationType", StringValue("ns3::RealtimeSimulatorImpl"));
    GlobalValue::Bind("ChecksumEnabled", BooleanValue(true));
    Config::SetDefault("ns3::BaseEndDeviceLorawanMac::ADRBackoff", BooleanValue(true));
    Config::SetDefault("ns3::BaseEndDeviceLorawanMac::EnableCryptography", BooleanValue(true));
    ///////////////// Needed to manage the variance introduced by real world interaction
    Config::SetDefault("ns3::ClassAEndDeviceLorawanMac::RecvWinSymb", UintegerValue(16));

    /* Logging options */
    if (log)
    {
        //!> Requirement: build ns3 with debug option
        LogComponentEnable("UdpForwarder", LOG_LEVEL_DEBUG);
        LogComponentEnable("ClassAEndDeviceLorawanMac", LOG_LEVEL_INFO);
        LogComponentEnable("BaseEndDeviceLorawanMac", LOG_LEVEL_INFO);
        // LogComponentEnable ("LoraFrameHeader", LOG_LEVEL_INFO);
        /* Monitor state changes of devices */
        LogComponentEnable("EloraExample", LOG_LEVEL_ALL);
        /* Formatting */
        LogComponentEnableAll(LOG_PREFIX_FUNC);
        LogComponentEnableAll(LOG_PREFIX_NODE);
        LogComponentEnableAll(LOG_PREFIX_TIME);
    }

    /*******************
     *  Radio Channel  *
     *******************/

    Ptr<OkumuraHataPropagationLossModel> loss;
    Ptr<NakagamiPropagationLossModel> rayleigh;
    Ptr<LoraChannel> channel;
    {
        // Delay obtained from distance and speed of light in vacuum (constant)
        Ptr<PropagationDelayModel> delay = CreateObject<ConstantSpeedPropagationDelayModel>();

        // This one is empirical and it encompasses average loss due to distance, shadowing (i.e.
        // obstacles), weather, height
        loss = CreateObject<OkumuraHataPropagationLossModel>();
        loss->SetAttribute("Frequency", DoubleValue(868100000.0));
        loss->SetAttribute("Environment", EnumValue(UrbanEnvironment));
        loss->SetAttribute("CitySize", EnumValue(LargeCity));

        // Here we can add variance to the propagation model with multipath Rayleigh fading
        rayleigh = CreateObject<NakagamiPropagationLossModel>();
        rayleigh->SetAttribute("m0", DoubleValue(1.0));
        rayleigh->SetAttribute("m1", DoubleValue(1.0));
        rayleigh->SetAttribute("m2", DoubleValue(1.0));

        channel = CreateObject<LoraChannel>(loss, delay);
    }

    /*************************
     *  Position & mobility  *
     *************************/

    MobilityHelper mobilityEd;
    MobilityHelper mobilityGw;
    Ptr<RangePositionAllocator> rangeAllocator;
    {
        // Gateway mobility
        mobilityGw.SetMobilityModel("ns3::ConstantPositionMobilityModel");
        // In hex tiling, distance = range * cos (pi/6) * 2 to have no holes
        double gatewayDistance = range * std::cos(M_PI / 6) * 2;
        auto hexAllocator = CreateObject<HexGridPositionAllocator>();
        hexAllocator->SetAttribute("Z", DoubleValue(30.0));
        hexAllocator->SetAttribute("distance", DoubleValue(gatewayDistance));
        mobilityGw.SetPositionAllocator(hexAllocator);

        // End Device mobility
        mobilityEd.SetMobilityModel("ns3::ConstantPositionMobilityModel");
        // We define rho to generalize the allocation disk for any number of gateway rings
        double rho = range + 2.0 * gatewayDistance * (gatewayRings - 1);
        rangeAllocator = CreateObject<RangePositionAllocator>();
        rangeAllocator->SetAttribute("rho", DoubleValue(rho));
        rangeAllocator->SetAttribute("ZRV",
                                     StringValue("ns3::UniformRandomVariable[Min=1|Max=10]"));
        rangeAllocator->SetAttribute("range", DoubleValue(range));
        mobilityEd.SetPositionAllocator(rangeAllocator);
    }

    /******************
     *  Create Nodes  *
     ******************/

    Ptr<Node> exitnode;
    NodeContainer gateways;
    NodeContainer endDevices;
    {
        exitnode = CreateObject<Node>();

        int nGateways = 3 * gatewayRings * gatewayRings - 3 * gatewayRings + 1;
        gateways.Create(nGateways);
        mobilityGw.Install(gateways);
        rangeAllocator->SetNodes(gateways);

        endDevices.Create(nDevices);
        mobilityEd.Install(endDevices);
    }

    /************************
     *  Create Net Devices  *
     ************************/

    /* Csma between gateways and tap-bridge (represented by exitnode) */
    {
        NodeContainer csmaNodes(NodeContainer(exitnode), gateways);

        // Connect the bridge to the gateways with csma
        CsmaHelper csma;
        csma.SetChannelAttribute("DataRate", DataRateValue(DataRate(5000000)));
        csma.SetChannelAttribute("Delay", TimeValue(MilliSeconds(2)));
        csma.SetDeviceAttribute("Mtu", UintegerValue(1500));
        auto csmaNetDevs = csma.Install(csmaNodes);

        // Install and initialize internet stack on gateways and bridge nodes
        InternetStackHelper internet;
        internet.Install(csmaNodes);

        Ipv4AddressHelper addresses;
        addresses.SetBase("10.1.2.0", "255.255.255.0");
        addresses.Assign(csmaNetDevs);

        Ipv4GlobalRoutingHelper::PopulateRoutingTables();
    }

    ///////////////// Attach a Tap-bridge to outside the simulation to the server csma device
    TapBridgeHelper tapBridge;
    tapBridge.SetAttribute("Mode", StringValue("ConfigureLocal"));
    tapBridge.SetAttribute("DeviceName", StringValue("ns3-tap"));
    tapBridge.Install(exitnode, exitnode->GetDevice(0));

    /* Radio side (between end devicees and gateways) */
    LorawanHelper helper;
    NetDeviceContainer gwNetDev;
    {
        // Physiscal layer settings
        LoraPhyHelper phyHelper;
        phyHelper.SetInterference("IsolationMatrix", EnumValue(sirMap.at(sir)));
        phyHelper.SetChannel(channel);

        // Create a LoraDeviceAddressGenerator
        /////////////////// Enables full parallelism between ELoRa instances
        uint8_t nwkId = RngSeedManager::GetRun();
        auto addrGen = CreateObject<LoraDeviceAddressGenerator>(nwkId);

        // Mac layer settings
        LorawanMacHelper macHelper;
        macHelper.SetRegion(LorawanMacHelper::EU);
        macHelper.SetAddressGenerator(addrGen);

        // Create the LoraNetDevices of the gateways
        phyHelper.SetType("ns3::GatewayLoraPhy");
        macHelper.SetType("ns3::GatewayLorawanMac");
        gwNetDev = helper.Install(phyHelper, macHelper, gateways);

        // Create the LoraNetDevices of the end devices
        phyHelper.SetType("ns3::EndDeviceLoraPhy");
        macHelper.SetType("ns3::ClassAEndDeviceLorawanMac");
        helper.Install(phyHelper, macHelper, endDevices);
    }

    /*************************
     *  Create Applications  *
     *************************/

    {
        // Install UDP forwarders in gateways
        UdpForwarderHelper forwarderHelper;
        forwarderHelper.SetAttribute("RemoteAddress", AddressValue(Ipv4Address("10.1.2.1")));
        forwarderHelper.SetAttribute("RemotePort", UintegerValue(destPort));
        forwarderHelper.Install(gateways);

        // Install applications in EDs
        if (testDev)
        {
            PeriodicSenderHelper appHelper;
            appHelper.SetPeriodGenerator(
                CreateObjectWithAttributes<ConstantRandomVariable>("Constant", DoubleValue(5.0)));
            appHelper.SetPacketSizeGenerator(
                CreateObjectWithAttributes<ConstantRandomVariable>("Constant", DoubleValue(5.0)));
            appHelper.Install(endDevices);
        }
        else
        {
            UrbanTrafficHelper appHelper;
            appHelper.SetDeviceGroups(Commercial);
            appHelper.Install(endDevices);
        }
    }

    /***************************
     *  Simulation and metrics *
     ***************************/

    ///////////////////// Signal handling
    OnInterrupt([](int signal) { csHelper.CloseConnection(signal); });
    ///////////////////// Register tenant, gateways, and devices on the real server
    csHelper.SetTenant(tenant);
    csHelper.InitConnection(apiAddr, apiPort, token);
    csHelper.Register(NodeContainer(endDevices, gateways));

    // Initialize SF emulating the ADR algorithm, then add variance to path loss
    std::vector<int> devPerSF(1, nDevices);
    if (initializeSF)
    {
        devPerSF = LorawanMacHelper::SetSpreadingFactorsUp(endDevices, gateways, channel);
    }
    loss->SetNext(rayleigh);

#ifdef NS3_LOG_ENABLE
    // Print current configuration
    PrintConfigSetup(nDevices, range, gatewayRings, devPerSF);
    helper.EnableSimulationTimePrinting(Seconds(3600));
#endif // NS3_LOG_ENABLE

    Config::ConnectWithoutContext(
        "/NodeList/*/DeviceList/0/$ns3::LoraNetDevice/Phy/$ns3::EndDeviceLoraPhy/EndDeviceState",
        MakeCallback(&OnStateChange));

    if (file)
    {
        helper.EnablePcap("lora", gwNetDev);
    }

    Simulator::Stop(Hours(1) * periods);

    // Start simulation
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}
