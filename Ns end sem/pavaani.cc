#include <iostream>
#include <fstream>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/point-to-point-module.h"
#include "ns3/netanim-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("CsmaMulticastExample");

int 
main (int argc, char *argv[])
{
  // LogComponentEnable ("CsmaMulticastExample", LOG_LEVEL_INFO);

  // Select DIX/Ethernet II-style encapsulation (no LLC/Snap header)
  Config::SetDefault ("ns3::CsmaNetDevice::EncapsulationMode", StringValue ("Dix"));

  CommandLine cmd (__FILE__);
  cmd.Parse (argc, argv);

  Address serverAddress; 

  NS_LOG_INFO ("Create nodes.");
  NodeContainer c;
  c.Create (10);

  NodeContainer c0 = NodeContainer (c.Get (0), c.Get (1), c.Get (2));
  NodeContainer c1 = NodeContainer (c.Get(1), c.Get (3), c.Get (4), c.Get (5));
  NodeContainer p0 = NodeContainer (c.Get(3),c.Get(6));
  NodeContainer p1 = NodeContainer (c.Get(6), c.Get(7));
  NodeContainer c2 = NodeContainer (c.Get (7), c.Get (8), c.Get (9));

  NS_LOG_INFO ("Build Topology.");
  PointToPointHelper p2p;
  p2p.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("2ms"));

  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", DataRateValue (DataRate (5000000)));
  csma.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (2)));
 

  NetDeviceContainer ndc0 = csma.Install (c0);  
  NetDeviceContainer ndc1 = csma.Install (c1);  
  NetDeviceContainer ndc2 = csma.Install (c2);
  NetDeviceContainer ndp0 = p2p.Install (p0);
  NetDeviceContainer ndp1 = p2p.Install (p1);

  NS_LOG_INFO ("Add IP Stack.");
  InternetStackHelper internet;
  internet.Install (c);

  NS_LOG_INFO ("Assign IP Addresses.");
  Ipv4AddressHelper addr0,addr1,addr2,addr3,addr4;
  addr0.SetBase ("10.1.1.0", "255.255.255.0");
  addr1.SetBase ("10.1.2.0", "255.255.255.0");
  addr2.SetBase ("10.1.3.0", "255.255.255.0");
  addr3.SetBase ("10.1.4.0", "255.255.255.0");
  addr4.SetBase ("10.1.5.0", "255.255.255.0");

  Ipv4InterfaceContainer ic0 = addr0.Assign (ndc0);
  Ipv4InterfaceContainer ic1 = addr1.Assign (ndc1);
  Ipv4InterfaceContainer ic2 = addr2.Assign (ndc2);
  Ipv4InterfaceContainer ip0 = addr3.Assign (ndp0);
  Ipv4InterfaceContainer ip1 = addr4.Assign (ndp1);

  Ipv4Address multicastSource ("10.1.1.1");
  Ipv4Address multicastGroup ("225.1.2.4");

  Ipv4StaticRoutingHelper multicast;


  Ptr<Node> multicastRouter = c.Get (1);  
  Ptr<NetDevice> inputIf = ndc0.Get (1); 
  NetDeviceContainer outputDevices; 
  outputDevices.Add (ndc1.Get (0)); 

  multicast.AddMulticastRoute (multicastRouter, multicastSource, 
                               multicastGroup, inputIf, outputDevices);

  Ptr<Node> multicastRouter1 = c.Get (3);  
  Ptr<NetDevice> inputIf1 = ndc1.Get (1); 
  NetDeviceContainer outputDevices1; 
  outputDevices1.Add (ndp0.Get (0)); 

  multicast.AddMulticastRoute (multicastRouter1, multicastSource, 
                               multicastGroup, inputIf1, outputDevices1);

  Ptr<Node> multicastRouter2 = c.Get (6);  
  Ptr<NetDevice> inputIf2 = ndp0.Get (1); 
  NetDeviceContainer outputDevices2; 
  outputDevices2.Add (ndp1.Get (0)); 

  multicast.AddMulticastRoute (multicastRouter2, multicastSource, 
                               multicastGroup, inputIf2, outputDevices2);

 
  Ptr<Node> sender = c.Get (0);
  Ptr<NetDevice> senderIf = ndc0.Get (0);
  multicast.SetDefaultMulticastRoute (sender, senderIf);

  NS_LOG_INFO ("Create Applications.");

  uint16_t multicastPort = 9;  


  OnOffHelper onoff ("ns3::UdpSocketFactory", 
                     Address (InetSocketAddress (multicastGroup, multicastPort)));
  onoff.SetConstantRate (DataRate ("255b/s"));
  onoff.SetAttribute ("PacketSize", UintegerValue (128));

  ApplicationContainer srcC = onoff.Install (c0.Get (0));
  srcC.Start (Seconds (1.));
  srcC.Stop (Seconds (10.));


  PacketSinkHelper sink ("ns3::TcpSocketFactory",
                         InetSocketAddress (Ipv4Address::GetAny (), multicastPort));

  ApplicationContainer sinkC = sink.Install (p1.Get (1)); 
  // Start the sink
  sinkC.Start (Seconds (1.0));
  sinkC.Stop (Seconds (10.0));

  uint16_t port = 9;  // well-known echo port number
  UdpEchoServerHelper server (port);
  ApplicationContainer apps = server.Install (c.Get (9));
  apps.Start (Seconds (20.0));
  apps.Stop (Seconds (30.0));

  serverAddress = Address(ic2.GetAddress (2));

  uint32_t packetSize = 1024;
  uint32_t maxPacketCount = 1;
  Time interPacketInterval = Seconds (1.);
  UdpEchoClientHelper client (serverAddress, port);
  client.SetAttribute ("MaxPackets", UintegerValue (maxPacketCount));
  client.SetAttribute ("Interval", TimeValue (interPacketInterval));
  client.SetAttribute ("PacketSize", UintegerValue (packetSize));
  apps = client.Install (c.Get (7));
  apps.Start (Seconds (22.0));
  apps.Stop (Seconds (30.0));

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  NS_LOG_INFO ("Configure Tracing.");

  AsciiTraceHelper ascii;
  Ptr<OutputStreamWrapper> stream = ascii.CreateFileStream ("midsem.tr");
  p2p.EnableAsciiAll(stream);
  csma.EnableAsciiAll (stream);

  NS_LOG_INFO ("Run Simulation.");
  AnimationInterface anim ("midsem1.xml");
 
  anim.SetConstantPosition(c.Get(0),30,10);
  anim.SetConstantPosition(c.Get(1),40,10);
  anim.SetConstantPosition(c.Get(2),50,10);
  anim.SetConstantPosition(c.Get(3),40,20);
  anim.SetConstantPosition(c.Get(4),50,20);
  anim.SetConstantPosition(c.Get(5),60,20);
  anim.SetConstantPosition(c.Get(6),40,30);
  anim.SetConstantPosition(c.Get(7),30,50);
  anim.SetConstantPosition(c.Get(8),40,50);
  anim.SetConstantPosition(c.Get(9),50,50);



  NS_LOG_INFO ("Run Simulation.");
  Simulator::Run ();
  Simulator::Destroy ();
  NS_LOG_INFO ("Done.");
}