/*
 *  Jonathan Jones
 *  ECE 6110
 *  Project 1
 */

#include <sstream>
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
const int ECHO_SERVER_PORT = 9;
const int TCP_SERVER_PORT = 2000;
const int RAND_NUM_SEED = 11223344;
const std::string flowMonFn = "tcp-flow-results.flowmon";
}

void SetSimConfigs() {
    // ===== Simulation Configs =====
    // Set the log levels for this module
    LogComponentEnable("TCPThroughtputMeasurements", LOG_LEVEL_ALL);

    // Set the log levels for the bulk TCP transfer applications
    // LogComponentEnable("BulkSendApplication", LOG_LEVEL_ALL);
    LogComponentEnable("PacketSink", LOG_LEVEL_ALL);

    // 1 ns time resolution, the default value
    Time::SetResolution(Time::NS);
}


int main (int argc, char* argv[]) {
    // Initial simulation configurations
    SetSimConfigs();

    // Go ahead and find the tcp type ID's
    const TypeId tcpTahoe = TypeId::LookupByName("ns3::TcpTahoe");
    const TypeId tcpReno = TypeId::LookupByName("ns3::TcpReno");

    // Parse any command line arguments
    CommandLine cmd;
    size_t segSize =    128;
    size_t winSize =    2000;
    size_t queueSize =  2000;
    size_t nFlows =     1;
    size_t nFlowBytes = 100000000;
    bool   flowMonEN =  true;

    cmd.AddValue("segSize",    "TCP segment size in bytes", segSize);
    cmd.AddValue("winSize",    "TCP maximum receiver advertised window size in bytes", winSize);
    cmd.AddValue("queueSize",  "Queue limit on the bottleneck link in bytes", queueSize);
    cmd.AddValue("nFlows",     "Number of simultaneous TCP flows", nFlows);
    cmd.AddValue("nFlowBytes", "Number of bytes to send for each TCP flow", nFlowBytes);
    cmd.AddValue("flowMon",    "Enable/Disable flow monitoring", flowMonEN);
    cmd.Parse(argc, argv);


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

    linkC.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    linkC.SetChannelAttribute("Delay", StringValue("10ms"));

    // NS_LOG(LOG_INFO, "Constructing the hubs for the bottleneck link");
    // PointToPointStarHelper hubB1(3, linkB);


    // ===== Network Devices =====
    NS_LOG(LOG_DEBUG, "Creating network interface devices");

    NetDeviceContainer devicesA, devicesB, devicesC;

    // Install the devices onto their respective links
    devicesA = linkA.Install(nodes.Get(0), nodes.Get(1));
    devicesB = linkB.Install(nodes.Get(1), nodes.Get(2));
    devicesC = linkC.Install(nodes.Get(2), nodes.Get(3));


    // ===== TCP Type =====
    // Set the type of TCP that will be used for all simulations
    NS_LOG(LOG_DEBUG, "Setting default TCP type to Tahoe");

    Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue("ns3::TcpTahoe"));


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

    // The TCP sink address
    Address tcpSinkAddr(InetSocketAddress(nicsA.GetAddress(0), TCP_SERVER_PORT));

    // ===== Application Client(s) =====
    // Create an application to send TCP data to the server
    NS_LOG(LOG_DEBUG, "Creating " << nFlows << "TCP flow" << (nFlows == 1 ? "" : "s"));

    BulkSendHelper tcpSource("ns3::TcpSocketFactory", tcpSinkAddr);
    ApplicationContainer clientApps;

    // Set the attributes for how we send the data
    tcpSource.SetAttribute("MaxBytes", UintegerValue(nFlowBytes));

    // Sending to the server node
    clientApps = tcpSource.Install(nodes.Get(nodes.GetN() - 1));

    NS_LOG(LOG_INFO, "TCP flow created at " << nicsC.GetAddress(0));

    // Set start/stop times
    clientApps.Start(Seconds(0.1));


    // ===== Application Server(s) =====
    // Create a TCP packet sink
    NS_LOG(LOG_DEBUG, "Creating TCP sink on " << nicsA.GetAddress(0) << ":" << TCP_SERVER_PORT);

    PacketSinkHelper tcpSink("ns3::TcpSocketFactory", tcpSinkAddr);
    ApplicationContainer serverApps;

    // tcpSink.SetAttribute("SegmentSize", UintegerValue(segSize));
    // tcpSink.SetAttribute("MaxWindowSize", UintegerValue(winSize));
    // tcpSink.SetAttribute("QueueSize", UintegerValue(queueSize));

    // Assign it to the list of app servers
    serverApps = tcpSink.Install(nodes.Get(0));

    // Set start/stop times
    serverApps.Start(Seconds(0.0));

    // Flow Monitor
    FlowMonitorHelper flowMon;
    if (flowMonEN == true)
        flowMon.InstallAll();

    // onOffHelper onOff("ns3::TcpSocketFactory", Address());
    // onOff.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
    // onOff.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));

    // // Assign an app to each spoke of the star link
    // ApplicationContainer spokeApps;
    // for (size_t i = 0; i < starLink.SpokeCount(); ++i) {
    //     AddressValue remoteAddress(InetSocketAddress(hubB1.GetHubIpv4Address(i), TCP_SERVER_PORT));
    //     onOff.SetAttribute("Remote", remoteAddress);
    //     spokeApps.Add(onOff.Install(starLink.GetSpokeNode(i)));
    // }


    // ===== Run Simulation =====
    NS_LOG(LOG_INFO, "Starting simulation");
    Simulator::Run();
    
    if (flowMonEN == true) {
        NS_LOG(LOG_INFO, "Saving flow results to " << flowMonFn.c_str());
        flowMon.SerializeToXmlFile (flowMonFn.c_str(), false, false);
    }

    NS_LOG(LOG_INFO, "Simulation complete");
    Simulator::Destroy();


    return 0;
}





