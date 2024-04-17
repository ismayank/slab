// Default Network Topology
//
//       10.1.1.0
// n0 -------------- n1
//    point-to-point
 
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/netanim-module.h"
using namespace ns3;
 
NS_LOG_COMPONENT_DEFINE ("FirstScriptExample");
 
int
main (int argc, char *argv[])
{
  CommandLine cmd (__FILE__);
  cmd.Parse (argc, argv);
  Time::SetResolution (Time::NS);
  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
 
  NodeContainer nodes;
  nodes.Create (3);
 
  PointToPointHelper pointToPoint;
  NetDeviceContainer device1, device2;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));
  device1.Add(pointToPoint.Install(nodes.Get(0), nodes.Get(1)));
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));
  device2.Add(pointToPoint.Install(nodes.Get(1), nodes.Get(2)));

 
  
  //devices = pointToPoint.Install (nodes);
 
  InternetStackHelper stack;
  stack.Install (nodes);
 
  Ipv4AddressHelper address1 , address2;
  address1.SetBase ("10.1.1.0", "255.255.255.0");
  address2.SetBase ("10.1.2.0", "255.255.255.0");
 
  Ipv4InterfaceContainer interfaces1 = address1.Assign (device1);
  Ipv4InterfaceContainer interfaces2 = address2.Assign (device2);
 
  UdpEchoServerHelper echoServer (9);
 
  ApplicationContainer serverApps = echoServer.Install (nodes.Get (2));
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (10.0));
 
  UdpEchoClientHelper echoClient (interfaces2.GetAddress (1), 9);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (6));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));
 
  ApplicationContainer clientApps = echoClient.Install (nodes.Get (0));
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (10.0));
  AnimationInterface anim ("netAnim2.xml"); 
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
 
  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}