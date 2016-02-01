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
}


int main (int argc, char* argv[])
{
    // ===== Simulation Configs =====
    // Set the log levels
    LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);

    // 1 ns time resolution, the default value
    Time::SetResolution(Time::NS);


    // ===== Nodes =====
    // Create the container used in simulation for
    // representing the computers
    NodeContainer nodes;
    nodes.Create(4);


    // ===== PointToPoint Links =====
    // Create 3 point-to-point links
    PointToPointHelper linkA, linkB, linkC;

    // Set their attributes
    linkA.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    linkA.SetChannelAttribute("Delay", StringValue("10ms"));

    linkB.SetDeviceAttribute("DataRate", StringValue("1Mbps"));
    linkB.SetChannelAttribute("Delay", StringValue("20ms"));

    linkC.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    linkC.SetChannelAttribute("Delay", StringValue("10ms"));


    // ===== Network Devices =====
    NetDeviceContainer devicesA, devicesB, devicesC;

    // Install the devices onto their respective links
    devicesA = linkA.Install(nodes.Get(0), nodes.Get(1));
    devicesB = linkB.Install(nodes.Get(1), nodes.Get(2));
    devicesC = linkC.Install(nodes.Get(2), nodes.Get(3));


    // ===== Internet Stack Assignment =====
    // Set IPv4, IPv6, UDP, & TCP stacks to all nodes in the simulation
    InternetStackHelper stack;
    stack.InstallAll();


    // ===== IPv4 Addresses ======
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


    // ===== Application Server(s) =====
    UdpEchoServerHelper echoServer(ECHO_SERVER_PORT);

    // Add the echo server to an app container
    ApplicationContainer serverApps;
    serverApps = echoServer.Install(nodes.Get(0));

    // Set start/stop times
    serverApps.Start(Seconds(0.0));
    serverApps.Stop(Seconds(5.0));

    // Simulation a client for the echo server
    UdpEchoClientHelper echoClient (nicsA.GetAddress(1), ECHO_SERVER_PORT);
    echoClient.SetAttribute("MaxPackets", UintegerValue(1));
    echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    echoClient.SetAttribute("PacketSize", UintegerValue(1024));


    // ===== Application Client(s) =====
    // Add the clients to a different app container
    ApplicationContainer clientApps;
    clientApps = echoClient.Install(nodes.Get(0));

    // Set start/stop times
    clientApps.Start(Seconds(1.0));
    clientApps.Stop(Seconds(4.0));


    // ===== Run Simulation =====
    Simulator::Run();
    Simulator::Destroy();


    return 0;
}


// Create and bind the socket...
// TypeId tid = TypeId::LookupByName ("ns3::TcpTahoe");
// std::stringstream nodeId;
// nodeId << n0n1.Get (0)->GetId ();
// std::string specificNode = "/NodeList/" + nodeId.str () + "/$ns3::TcpL4Protocol/SocketType";
// Config::Set (specificNode, TypeIdValue (tid));
// Ptr<Socket> localSocket =
//   Socket::CreateSocket (n0n1.Get (0), TcpSocketFactory::GetTypeId ());


