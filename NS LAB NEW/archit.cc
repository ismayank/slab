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

NS_LOG_COMPONENT_DEFINE ("CsmaMulticastExample");

int 
main (int argc, char *argv[])
{
  //CommandLine cmd (__FILE__);
  //cmd.Parse (argc, argv);
  uint32_t maxBytes = 512;
  NS_LOG_INFO ("Create nodes.");
  NodeContainer c;
  c.Create (12);
  // We will later want two subcontainers of these nodes, for the two LANs
  NodeContainer c0 = NodeContainer (c.Get (2), c.Get (4), c.Get (5),c.Get (3));
  c0.Add(c.Get(3));

  NodeContainer c1 = NodeContainer (c.Get (1), c.Get (10), c.Get (11),c.Get (8));
  NodeContainer p0 = NodeContainer (c.Get (2),c.Get (0));
  NodeContainer p1 = NodeContainer (c.Get (0),c.Get (1));
  NodeContainer p2 = NodeContainer (c.Get (8),c.Get (9));
  NodeContainer p3 = NodeContainer (c.Get (9),c.Get (7));
  NodeContainer p4 = NodeContainer (c.Get (7),c.Get (3));
  NodeContainer p5 = NodeContainer (c.Get (3),c.Get (6));

  


  NS_LOG_INFO ("Build Topology.");
  NetDeviceContainer nd3, nd4, nd5, nd6, nd7, nd8;
  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", DataRateValue (DataRate (5000000)));
  csma.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (2)));
  PointToPointHelper p2p;
  p2p.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("2ms"));

  nd3.Add(p2p.Install(c.Get(2), c.Get(0)));
  nd4.Add(p2p.Install(c.Get(0), c.Get(1)));
  nd5.Add(p2p.Install(c.Get(8), c.Get(9)));
  nd6.Add(p2p.Install(c.Get(9), c.Get(7)));
  nd7.Add(p2p.Install(c.Get(7), c.Get(3)));
  nd8.Add(p2p.Install(c.Get(3), c.Get(6)));
  
  // We will use these NetDevice containers later, for IP addressing
  NetDeviceContainer nd0 = csma.Install (c0);  // First LAN
  NetDeviceContainer nd1 = csma.Install (c1);  // Second LAN
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
  address8.SetBase ("10.1.8.0", "255.255.255.0");

  Ipv4InterfaceContainer interfaces1 = address1.Assign (nd0);
  Ipv4InterfaceContainer interfaces2 = address2.Assign (nd1);
  Ipv4InterfaceContainer interfaces3 = address3.Assign (nd3);
  Ipv4InterfaceContainer interfaces4 = address4.Assign (nd4);
  Ipv4InterfaceContainer interfaces5 = address5.Assign (nd5);
  Ipv4InterfaceContainer interfaces6 = address6.Assign (nd6);
  Ipv4InterfaceContainer interfaces7 = address7.Assign (nd7);
  Ipv4InterfaceContainer interfaces8 = address8.Assign (nd8);

  uint16_t port = 9; // well-known echo port number

   Ipv4Address multicastSource ("10.1.1.1");
  Ipv4Address multicastGroup ("225.1.2.4");

  Ipv4StaticRoutingHelper multicast;

  // 1) Configure a (static) multicast route on node n2 (multicastRouter)
  Ptr<Node> multicastRouter = c.Get (3);  // The node in question
  Ptr<NetDevice> inputIf = nd0.Get (3);  // The input NetDevice
  NetDeviceContainer outputDevices;  // A container of output NetDevices
  outputDevices.Add (nd7.Get (1));  // (we only need one NetDevice here)
  // outputDevices.Add (nd3.Get(0)); 

  multicast.AddMulticastRoute (multicastRouter, multicastSource, 
                               multicastGroup, inputIf, outputDevices);
  
   Ptr<Node> multicastRouter2 = c.Get (7);  // The node in question
  Ptr<NetDevice> inputIf2 = nd7.Get (0);  // The input NetDevice
  NetDeviceContainer outputDevices2;  // A container of output NetDevices
  outputDevices2.Add (nd6.Get (1));  // (we only need one NetDevice here)
  // outputDevices.Add (nd3.Get(0)); 

  multicast.AddMulticastRoute (multicastRouter2, multicastSource, 
                               multicastGroup, inputIf2, outputDevices2);

        
   Ptr<Node> multicastRouter3 = c.Get (9);  // The node in question
  Ptr<NetDevice> inputIf3 = nd6.Get (0);  // The input NetDevice
  NetDeviceContainer outputDevices3;  // A container of output NetDevices
  outputDevices3.Add (nd5.Get (1));  // (we only need one NetDevice here)
  // outputDevices.Add (nd3.Get(0)); 

  multicast.AddMulticastRoute (multicastRouter3, multicastSource, 
                               multicastGroup, inputIf3, outputDevices3);

        
  Ptr<Node> sender = c.Get (2);
  Ptr<NetDevice> senderIf = nd0.Get (0);
  multicast.SetDefaultMulticastRoute (sender, senderIf);



  uint16_t multicastPort = 9;

  OnOffHelper onoff ("ns3::UdpSocketFactory", 
                     Address (InetSocketAddress (multicastGroup, multicastPort)));
  onoff.SetConstantRate (DataRate ("255b/s"));
  onoff.SetAttribute ("PacketSize", UintegerValue (128));

  ApplicationContainer srcC = onoff.Install (c0.Get (0));
  srcC.Start (Seconds (1.));
  srcC.Stop (Seconds (10.));


  PacketSinkHelper sink ("ns3::UdpSocketFactory",
                         InetSocketAddress (Ipv4Address::GetAny (), multicastPort));


  ApplicationContainer sinkC = sink.Install (p2.Get (0)); 
  // Start the sink
  sinkC.Start (Seconds (1.0));
  sinkC.Stop (Seconds (10.0));

  /*
BulkSendHelper source ("ns3::TcpSocketFactory",
InetSocketAddress (interfaces4.GetAddress (1), port));   //PUT HERE DESTINATION KA  INTERFACE 
// Set the amount of data to send in bytes. Zero is unlimited.
source.SetAttribute ("MaxBytes", UintegerValue (maxBytes));
ApplicationContainer sourceApps = source.Install (c.Get (11));   //SOURCE NODE  
sourceApps.Start (Seconds (2.0));
sourceApps.Stop (Seconds (5.3));

PacketSinkHelper sink ("ns3::TcpSocketFactory",
InetSocketAddress (Ipv4Address::GetAny (), port));
ApplicationContainer sinkApps = sink.Install (c.Get (4));  // DESTINATION NODE
sinkApps.Start (Seconds (2.0));
sinkApps.Stop (Seconds (5.3));

UdpEchoServerHelper echoServer (9);

  ApplicationContainer serverApps1 = echoServer.Install (c.Get (7));
  serverApps1.Start (Seconds (4.8));
  serverApps1.Stop (Seconds (10.0));

  UdpEchoClientHelper echoClient (interfaces8.GetAddress (1), 9);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (6));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

  ApplicationContainer clientApps1 = echoClient.Install (c.Get (6));
  clientApps1.Start (Seconds (4.8));
  clientApps1.Stop (Seconds (10.0));

  */

  //Ipv4GlobalRoutingHelper::PopulateRoutingTables ();


  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  NS_LOG_INFO ("Configure Tracing.");
 
  AsciiTraceHelper ascii;
  Ptr<OutputStreamWrapper> stream = ascii.CreateFileStream ("practice.tr");
  p2p.EnableAsciiAll (stream);
  csma.EnableAsciiAll(stream);


  NS_LOG_INFO ("Run Simulation.");
  AnimationInterface anim ("practice1.xml");
  anim.SetConstantPosition ( c.Get(2), 1, 20);
  anim.SetConstantPosition ( c.Get(4), 20, 20);
  anim.SetConstantPosition ( c.Get(5), 30, 20);
  anim.SetConstantPosition ( c.Get(3), 40, 20);
  anim.SetConstantPosition ( c.Get(7), 40, 40);
  anim.SetConstantPosition ( c.Get(9), 45, 40);
  anim.SetConstantPosition ( c.Get(8), 80, 40);
  anim.SetConstantPosition ( c.Get(0), 1, 10);
  anim.SetConstantPosition ( c.Get(1), 20, 10);
  anim.SetConstantPosition(c.Get(10), 70, 15);
  anim.SetConstantPosition ( c.Get(11), 75, 20);
  anim.SetConstantPosition ( c.Get(6), 45, 30);

    

  Simulator::Run ();
  Simulator::Destroy ();
  NS_LOG_INFO ("Done.");
}
