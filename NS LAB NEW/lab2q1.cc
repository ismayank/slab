#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/netanim-module.h" 

// Default Network Topology
//
//       10.1.1.0
// n0 -------n1------- n2   n3   n4   n5   n6
//    point-to-point    |    |    |    |    |
//                      =====================
//                      LAN 10.1.2.0


using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("SecondScriptExample");

int 
main (int argc, char *argv[])
{
  bool verbose = true;
  uint32_t nCsma = 4;

  CommandLine cmd (__FILE__);
  cmd.AddValue ("nCsma", "Number of \"extra\" CSMA nodes/devices", nCsma);
  cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);

  cmd.Parse (argc,argv);

  if (verbose)
    {
      LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
      LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
    }

  nCsma = nCsma == 0 ? 1 : nCsma;

  NodeContainer p2pNodes;
  p2pNodes.Create (3);
  

  NodeContainer csmaNodes;
  //csmaNodes.Add (p2pNodes.Get (1));
  csmaNodes.Add (p2pNodes.Get (2));
  csmaNodes.Create (nCsma);

  PointToPointHelper pointToPoint1, pointToPoint2;
  NetDeviceContainer devices1, devices2;

  pointToPoint1.SetDeviceAttribute ("DataRate", StringValue ("1Mbps"));
  pointToPoint1.SetChannelAttribute ("Delay", StringValue ("50ms"));
  devices1.Add(pointToPoint1.Install(p2pNodes.Get(0), p2pNodes.Get(1)));

  pointToPoint2.SetDeviceAttribute ("DataRate", StringValue ("1Mbps"));
  pointToPoint2.SetChannelAttribute ("Delay", StringValue ("50ms"));
  devices2.Add(pointToPoint2.Install(p2pNodes.Get(1), p2pNodes.Get(2)));
  
  //NetDeviceContainer p2pDevices;
  //p2pDevices = pointToPoint1.Install (p2pNodes);

  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
  csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));

  NetDeviceContainer csmaDevices;
  csmaDevices = csma.Install (csmaNodes);
  
  //NodeContainer p2pNodes1;
  //p2pNodes.Create (1);

  InternetStackHelper stack;
  stack.Install (p2pNodes.Get (0));
  stack.Install (p2pNodes.Get (1));
  stack.Install (csmaNodes);

  Ipv4AddressHelper address1, address2, address;
  address1.SetBase ("10.1.1.0", "255.255.255.0");
  address2.SetBase ("10.1.2.0", "255.255.255.0");

  Ipv4InterfaceContainer interfaces1 = address1.Assign (devices1);
  Ipv4InterfaceContainer interfaces2 = address2.Assign (devices2);

  address.SetBase ("10.1.3.0", "255.255.255.0");
  Ipv4InterfaceContainer csmaInterfaces;
  csmaInterfaces = address.Assign (csmaDevices);

  UdpEchoServerHelper echoServer (9);

  ApplicationContainer serverApps = echoServer.Install (csmaNodes.Get (nCsma));
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (10.0));

  UdpEchoClientHelper echoClient (csmaInterfaces.GetAddress (nCsma), 9);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (1));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

  ApplicationContainer clientApps = echoClient.Install (p2pNodes.Get (0));
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (10.0));

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  pointToPoint1.EnablePcapAll ("second");
  pointToPoint2.EnablePcapAll ("second");
  csma.EnablePcap ("second", csmaDevices.Get (1), true);
  
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  AnimationInterface anim("animFile.xml"); 
  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
