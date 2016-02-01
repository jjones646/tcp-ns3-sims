/*
 *  Jonathan Jones
 *  ECE 6110
 *  Project 1
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"

using namespace ns3;


// Set the documentation name
NS_LOG_COMPONENT_DEFINE ("TCPThroughtputMeasurements");


namespace
{
const int ECHO_SERVER_PORT = 9;
const int TCP_SERVER_PORT = 2000;
const int RAND_NUM_SEED = 11223344;
}


int main (int argc, char* argv[])
{
    // ===== Simulation Configs =====
    // Set the log levels for this module
    LogComponentEnable("TCPThroughtputMeasurements", NS_LOG_ALL);

    // 1 ns time resolution, the default value
    Time::SetResolution(Time::NS);

    // Go ahead and find the tcp type ID's
    const TypeId tcpTahoe = TypeId::LookupByName("ns3::TcpTahoe");
    const TypeId tcpReno = TypeId::LookupByName("ns3::TcpReno");


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


    NS_LOG(LOG_DEBUG, "Device A address: " << nicsA.GetAddress());// << ", " << nicsB.GetAddress().Print() << ", " << nicsC.GetAddress().Print());


    // ===== Application Server(s) =====
    // Create a TCP packet sink
    NS_LOG(LOG_DEBUG, "Creating TCP server on port " << TCP_SERVER_PORT);

    Address tcpSinkAddr(InetSocketAddress(Ipv4Address::GetAny(), TCP_SERVER_PORT));
    PacketSinkHelper tcpSink("ns3::TcpSocketFactory", tcpSinkAddr);
    ApplicationContainer serverApps;

    // Assign it to the list of app servers
    serverApps = tcpSink.Install(nodes.Get(0));

    // Set start/stop times
    serverApps.Start(Seconds(0.0));
    serverApps.Stop(Seconds(10.0));


    // ===== Application Client(s) =====
    // Create an application to send TCP data to the server
    NS_LOG(LOG_DEBUG, "Creating TCP client");

    BulkSendHelper tcpClient("ns3::TcpSocketFactory", nicsB.Get(0));
    ApplicationContainer clientApps;

    // Sending to the server node
    clientApps = tcpClient.Install(nodes.Get(0));
    clientApps = tcpClient.Install(nodes.Get(0));

    echoClient.SetAttribute("MaxPackets", UintegerValue(1));
    echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    echoClient.SetAttribute("PacketSize", UintegerValue(1024));

    // Set start/stop times
    clientApps.Start(Seconds(0.1));
    clientApps.Stop(Seconds(10.0));


    // ===== Run Simulation =====
    Simulator::Run();
    Simulator::Destroy();


    return 0;
}





