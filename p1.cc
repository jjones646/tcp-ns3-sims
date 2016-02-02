/*
 *  Jonathan Jones
 *  ECE 6110
 *  Project 1
 */

#include <algorithm>
#include <string>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/point-to-point-layout-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-helper.h"

// #include "ns3/netanim-module.h"
// #include "ns3/csma-module.h"
// #include "ns3/ipv4-global-routing-helper.h"

using namespace ns3;


// Set the documentation name
NS_LOG_COMPONENT_DEFINE ("TCPThroughtputMeasurements");


namespace
{
const int TCP_SERVER_PORT = 8080;
const int RAND_NUM_SEED = 11223344;
}


void SetSimConfigs() {
    // Set the log levels for this module
    LogComponentEnable("TCPThroughtputMeasurements", LOG_LEVEL_ALL);
    LogComponentEnable("TCPThroughtputMeasurements", LOG_LEVEL_INFO);

    // Set the log levels for the bulk TCP transfer applications
    // LogComponentEnable("BulkSendApplication", LOG_LEVEL_ALL);
    // LogComponentEnable("PacketSink", LOG_LEVEL_ALL);

    // 1 ns time resolution, the default value
    Time::SetResolution(Time::NS);

    // The the random number seed globally
    RngSeedManager::SetSeed(RAND_NUM_SEED);
}


int main (int argc, char* argv[]) {
    // Initial simulation configurations
    SetSimConfigs();

    // Parse any command line arguments
    CommandLine cmd;
    size_t segSize =    128;
    size_t winSize =    2000;
    size_t queueSize =  2000;
    size_t nFlows =     1;
    size_t nFlowBytes = 100000000;
    std::string tcpType = "Tahoe";
    std::string pcapFn = "tcp-trace-results";
    bool   traceEN =    false;

    cmd.AddValue("segSize",    "TCP segment size in bytes", segSize);
    cmd.AddValue("winSize",    "TCP maximum receiver advertised window size in bytes", winSize);
    cmd.AddValue("queueSize",  "Queue limit on the bottleneck link in bytes", queueSize);
    cmd.AddValue("nFlows",     "Number of simultaneous TCP flows", nFlows);
    cmd.AddValue("nFlowBytes", "Number of bytes to send for each TCP flow", nFlowBytes);
    cmd.AddValue("tcpType",    "Simulate using Tahoe or Reno", tcpType);
    cmd.AddValue("trace",      "Enable/Disable dumping the trace at the TCP sink", traceEN);
    cmd.AddValue("traceFile",  "Base name given to where the results are saved when enabled", pcapFn);
    cmd.Parse(argc, argv);

    // convert the tcpType argument's value to lower case always
    std::transform(tcpType.begin(), tcpType.end(), tcpType.begin(), ::tolower);


    // ===== Nodes =====
    // Create the container used in simulation for
    // representing the computers
    NS_LOG(LOG_DEBUG, "Creating " << 4 << " nodes");

    NodeContainer nodes;
    nodes.Create(4);


    // ===== PointToPoint Links =====
    // Create 3 point-to-point links
    NS_LOG(LOG_DEBUG, "Creating point-to-point links");

    PointToPointHelper linkA, linkB, linkC;

    // Set their attributes
    linkA.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    linkA.SetChannelAttribute("Delay", StringValue("10ms"));

    linkB.SetDeviceAttribute("DataRate", StringValue("1Mbps"));
    linkB.SetChannelAttribute("Delay", StringValue("20ms"));
    // linkB.SetDeviceAttribute("TxQueue", UintegerValue(queueSize));

    linkC.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    linkC.SetChannelAttribute("Delay", StringValue("10ms"));


    // ===== Network Devices =====
    NS_LOG(LOG_DEBUG, "Creating network interface devices");

    NetDeviceContainer devicesA, devicesB, devicesC;

    // Install the devices onto their respective links
    devicesA = linkA.Install(nodes.Get(0), nodes.Get(1));
    devicesB = linkB.Install(nodes.Get(1), nodes.Get(2));
    devicesC = linkC.Install(nodes.Get(2), nodes.Get(3));

    // TypeId typeDev = NetDevice::GetTypeId();
    // for (size_t j = 0; j < typeDev.GetAttributeN(); ++j) {
    //     NS_LOG(LOG_INFO, typeDev.GetName() << " => " << typeDev.GetAttributeFullName(j));
    // }


    // ===== Internet Stack Assignment =====
    // Set IPv4, IPv6, UDP, & TCP stacks to all nodes in the simulation
    NS_LOG(LOG_DEBUG, "Setting simulation to use IPv4, IPv6, UDP, & TCP stacks");

    InternetStackHelper stack;
    stack.InstallAll();


    // ===== IPv4 Addresses ======
    NS_LOG(LOG_DEBUG, "Assigning IPv4 addresses");

    Ipv4AddressHelper addrsA, addrsB, addrsC;
    Ipv4InterfaceContainer nicsA, nicsB, nicsC;

    // Define the addresses
    addrsA.SetBase("10.0.0.0",   "255.255.255.0");
    addrsB.SetBase("10.64.0.0",  "255.255.255.0");
    addrsC.SetBase("10.128.0.0", "255.255.255.0");

    // Assign the addresses to devices
    nicsA = addrsA.Assign(devicesA);
    nicsB = addrsB.Assign(devicesB);
    nicsC = addrsC.Assign(devicesC);

    // Enable IPv4 routing
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();


    // ===== TCP Type =====
    // Set the type of TCP that will be used for all simulations
    if (tcpType.compare("reno") == 0) {
        Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue("ns3::TcpReno"));
        NS_LOG(LOG_INFO, "Using TCP RENO");
    }
    else {
        Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue("ns3::TcpTahoe"));
        NS_LOG(LOG_INFO, "Using TCP TAHOE");
    }

    // Set the default segment size used for all TCP connections
    NS_LOG(LOG_INFO, "Setting TCP segment size to " << segSize << " bytes");
    Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(segSize));

    // Set the default max window size for all TCP connections
    NS_LOG(LOG_INFO, "Setting TCP max advertised window size to " << winSize << " bytes");
    Config::SetDefault("ns3::TcpSocketBase::MaxWindowSize", UintegerValue(winSize));

    // The TCP sink address
    Address tcpSinkAddr(InetSocketAddress(nicsA.GetAddress(0), TCP_SERVER_PORT));


    // ===== Application Client(s) =====
    // Create an application to send TCP data to the server
    NS_LOG(LOG_DEBUG, "Creating " << nFlows << " TCP flow" << (nFlows == 1 ? "" : "s"));

    // App container for holding all of the TCP flows
    ApplicationContainer clientApps;

    // Create the random variable object for setting the initial flow start times
    Ptr<UniformRandomVariable> randVar = CreateObject<UniformRandomVariable>();

    for (size_t i = 0; i < nFlows; ++i) {
        BulkSendHelper tcpSource("ns3::TcpSocketFactory", tcpSinkAddr);
        ApplicationContainer tcpFlow;

        // Set the attributes for how we send the data
        tcpSource.SetAttribute("MaxBytes", UintegerValue(nFlowBytes));

        // Sending to the server node
        tcpFlow = tcpSource.Install(nodes.Get(nodes.GetN() - 1));

        // Set the starting time to some uniformly distributed random time between 0.0s and 0.1s
        tcpFlow.Start(Seconds(0.1 * (randVar->GetValue(0, randVar->GetMax()) / randVar->GetMax())));

        // Add this app container to the object holding all of our source flow apps
        clientApps.Add(tcpFlow);
    }


    // ===== Application Server(s) =====
    // Create a TCP packet sink
    NS_LOG(LOG_DEBUG, "Creating TCP sink on " << nicsA.GetAddress(0) << ":" << TCP_SERVER_PORT);

    PacketSinkHelper tcpSink("ns3::TcpSocketFactory", tcpSinkAddr);
    ApplicationContainer serverApps;

    // Assign it to the list of app servers
    serverApps = tcpSink.Install(nodes.Get(0));

    // Set start/stop times
    serverApps.Start(Seconds(0.0));

    // Set up tracing if enabled
    if (traceEN == true) {
        AsciiTraceHelper ascii;
        linkA.EnableAsciiAll(ascii.CreateFileStream(pcapFn + ".tr"));
        linkA.EnablePcapAll(pcapFn, false);
    }

    // ===== Run Simulation =====
    NS_LOG(LOG_INFO, "Starting simulation");

    Simulator::Run();
    Time endTime = Simulator::Now();

    Simulator::Destroy();

    NS_LOG(LOG_INFO, "Simulation complete");

    // Print out the overall goodput
    Ptr<PacketSink> sinkApp = DynamicCast<PacketSink>(serverApps.Get(0));
    NS_LOG(LOG_ALL, "goodput, " << sinkApp->GetTotalRx() / endTime);


    return 0;
}





