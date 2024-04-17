// Network topology
//                     		   n10       n8---n7
//                                / |        |
//                               /  ==========
//            n5-------n0  n1  n2       |
//            | \      |   |   |        n9
//            |  \     ==========
//            n6  n4       |
//                         n3


#include <iostream>
#include <fstream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"
#include "ns3/netanim-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/ipv4-global-routing-helper.h"
using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("topotest2");

int 
main (int argc, char *argv[])
{
  //CommandLine cmd (__FILE__);
  //cmd.Parse (argc, argv);
  uint32_t maxBytes = 512;
  NS_LOG_INFO ("Create nodes.");
  NodeContainer c;
  c.Create (10);
  // We will later want two subcontainers of these nodes, for the two LANs
  NodeContainer c0 = NodeContainer (c.Get (0), c.Get (1), c.Get (2));
  //c0.Add(c.Get(3));

  NodeContainer c1 = NodeContainer (c.Get (3), c.Get (4), c.Get (5));
  NodeContainer c2 = NodeContainer (c.Get (6), c.Get (7), c.Get (8));
  


  NS_LOG_INFO ("Build Topology.");
  NetDeviceContainer nd3, nd4, nd5, nd6, nd7, nd8;
  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", DataRateValue (DataRate (5000000)));
  csma.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (2)));
  PointToPointHelper p2p;
  p2p.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("2ms"));

  nd3.Add(p2p.Install(c.Get(2), c.Get(3)));
  nd4.Add(p2p.Install(c.Get(2), c.Get(6)));
  nd5.Add(p2p.Install(c.Get(8), c.Get(9)));
  nd6.Add(p2p.Install(c.Get(5), c.Get(9)));
  //nd7.Add(p2p.Install(c.Get(2), c.Get(10)));
  //nd8.Add(p2p.Install(c.Get(8), c.Get(7)));
  
  // We will use these NetDevice containers later, for IP addressing
  NetDeviceContainer nd0 = csma.Install (c0);  // First LAN
  NetDeviceContainer nd1 = csma.Install (c1); 
  NetDeviceContainer nd2 = csma.Install (c2);  // Second LAN
  //NetDeviceContainer nd3 = p2p.Install (p1);
  NS_LOG_INFO ("Add IP Stack.");
  InternetStackHelper internet;
  internet.Install (c);

  NS_LOG_INFO ("Assign IP Addresses.");
  Ipv4AddressHelper address1, address2, address3, address4, address5, address6, address7, address8;
  address1.SetBase ("10.1.1.0", "255.255.255.0");
  address2.SetBase ("10.1.2.0", "255.255.255.0");
  address3.SetBase ("10.1.3.0", "255.255.255.0");
  address4.SetBase ("10.1.4.0", "255.255.255.0");
  address5.SetBase ("10.1.5.0", "255.255.255.0");
  address6.SetBase ("10.1.6.0", "255.255.255.0");
  address7.SetBase ("10.1.7.0", "255.255.255.0");
 // address8.SetBase ("10.1.8.0", "255.255.255.0");

  Ipv4InterfaceContainer interfaces1 = address1.Assign (nd0);
  Ipv4InterfaceContainer interfaces2 = address2.Assign (nd1);
  Ipv4InterfaceContainer interfaces3 = address3.Assign (nd2);
  Ipv4InterfaceContainer interfaces4 = address4.Assign (nd3);
  Ipv4InterfaceContainer interfaces5 = address5.Assign (nd4);
  Ipv4InterfaceContainer interfaces6 = address6.Assign (nd5);
  Ipv4InterfaceContainer interfaces7 = address7.Assign (nd6);
  

  
  //Ipv4GlobalRoutingHelper::PopulateRoutingTables ();


  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  NS_LOG_INFO ("Configure Tracing.");
 
  AsciiTraceHelper ascii;
  Ptr<OutputStreamWrapper> stream = ascii.CreateFileStream ("mark.tr");
  p2p.EnableAsciiAll (stream);
  csma.EnableAsciiAll(stream);


  NS_LOG_INFO ("Run Simulation.");
  AnimationInterface anim ("mark.xml");

  anim.SetConstantPosition ( c.Get(0), 1, 20);
  anim.SetConstantPosition ( c.Get(1), 10, 20);
  anim.SetConstantPosition ( c.Get(2), 20, 20);
  anim.SetConstantPosition ( c.Get(3), 26, 10);
  anim.SetConstantPosition ( c.Get(4), 36, 10);
  anim.SetConstantPosition ( c.Get(5), 46, 10);
  anim.SetConstantPosition ( c.Get(9), 80, 20);
  anim.SetConstantPosition ( c.Get(6), 26, 60);
  anim.SetConstantPosition ( c.Get(7), 36, 60);
  anim.SetConstantPosition ( c.Get(8), 46, 60);
  

    

  Simulator::Run ();
  Simulator::Destroy ();
  NS_LOG_INFO ("Done.");
}
