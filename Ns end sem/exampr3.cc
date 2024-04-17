#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-apps-module.h"
#include "ns3/internet-module.h"
#include "ns3/ipv6-routing-table-entry.h"
#include "ns3/ipv6-static-routing-helper.h"
#include "ns3/netanim-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/ssid.h"
#include <fstream>
#include <ns3/core-module.h>
#include <ns3/network-module.h>
#include <ns3/internet-module.h>
#include <ns3/applications-module.h>
#include <ns3/point-to-point-helper.h>
#include <ns3/ipv4-address-helper.h>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("RipNgSimpleRouting");


int
main(int argc, char** argv)
{

 uint32_t maxBytes = 512;
  std::string speed=     "ns3::ConstantRandomVariable[Constant=20.5]";
  std::string distance = "ns3::ConstantRandomVariable[Constant=50.0]";
    bool verbose = false;
    bool printRoutingTables = false;
    bool showPings = false;
    std::string SplitHorizon("");

    CommandLine cmd(__FILE__);
    cmd.AddValue("verbose", "turn on log components", verbose);
    cmd.AddValue("printRoutingTables",
                 "Print routing tables at 30, 60 and 90 seconds",
                 printRoutingTables);
    cmd.AddValue("showPings", "Show Ping6 reception", showPings);
    cmd.AddValue("splitHorizonStrategy",
                 "Split Horizon strategy to use (NoSplitHorizon, SplitHorizon, PoisonReverse)",
                 SplitHorizon);
    cmd.Parse(argc, argv);

    if (verbose)
    {
        LogComponentEnable("RipNgSimpleRouting", LOG_LEVEL_INFO);
        LogComponentEnable("RipNg", LOG_LEVEL_ALL);
        LogComponentEnable("Icmpv6L4Protocol", LOG_LEVEL_INFO);
        LogComponentEnable("Ipv6Interface", LOG_LEVEL_ALL);
        LogComponentEnable("Icmpv6L4Protocol", LOG_LEVEL_ALL);
        LogComponentEnable("NdiscCache", LOG_LEVEL_ALL);
        LogComponentEnable("Ping", LOG_LEVEL_ALL);
    }

    if (SplitHorizon == "NoSplitHorizon")
    {
        Config::SetDefault("ns3::RipNg::SplitHorizon", EnumValue(RipNg::NO_SPLIT_HORIZON));
    }
    else if (SplitHorizon == "SplitHorizon")
    {
        Config::SetDefault("ns3::RipNg::SplitHorizon", EnumValue(RipNg::SPLIT_HORIZON));
    }
    else
    {
        Config::SetDefault("ns3::RipNg::SplitHorizon", EnumValue(RipNg::POISON_REVERSE));
    }

    NS_LOG_INFO("Create nodes.");
     NodeContainer c;
    c.Create(13);
    
    
    NodeContainer  c0 = NodeContainer(c.Get(0), c.Get(1), c.Get(2));//lan 1
    NodeContainer  c1 = NodeContainer(c.Get(10), c.Get(11), c.Get(12));//lan 2
    
    //P2p
    NodeContainer  p0 = NodeContainer(c.Get(2), c.Get(3));
    NodeContainer  p1 = NodeContainer(c.Get(9), c.Get(10));
    
    //wifiDevices
     NodeContainer  wifiStaNodes = NodeContainer(c.Get(3), c.Get(4), c.Get(5), c.Get(6));
     wifiStaNodes.Add(c.Get(7));
     wifiStaNodes.Add(c.Get(8));
     wifiStaNodes.Add(c.Get(9));
     //StationNodes
      NodeContainer  wifiStaNodes2 = NodeContainer(c.Get(3), c.Get(4), c.Get(5), c.Get(6));
     wifiStaNodes2.Add(c.Get(7));
     wifiStaNodes2.Add(c.Get(8));
     
     //accesssPoint
     NodeContainer wifiApNode = c.Get (9);
    
   
    NodeContainer routers( c.Get(2));
    NodeContainer nodes(c.Get(1), c.Get(3));

    NS_LOG_INFO("Create channels.");
   
    PointToPointHelper p2pa;
    p2pa.SetDeviceAttribute("DataRate", StringValue("8Mbps"));
    p2pa.SetChannelAttribute("Delay", StringValue("5ms"));
    
    PointToPointHelper p2pb;
    p2pb.SetDeviceAttribute("DataRate", StringValue("6Mbps"));
    p2pb.SetChannelAttribute("Delay", StringValue("10ms"));
    
    NetDeviceContainer nd1 = p2pa.Install(p0);
    NetDeviceContainer nd2 = p2pb.Install(p1);
    
   CsmaHelper csma1;
 csma1.SetChannelAttribute ("DataRate", StringValue ("200Mbps"));
 csma1.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (12)));
    
    NetDeviceContainer nd3 = csma1.Install(c0);
     CsmaHelper csma2;
 csma2.SetChannelAttribute ("DataRate", StringValue ("200Mbps"));
 csma2.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (8)));
    NetDeviceContainer nd4 = csma2.Install(c1);
    
    
    
    YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
  YansWifiPhyHelper phy;
  phy.SetChannel (channel.Create ());
  NS_LOG_INFO ("Create Applications.");
  
  WifiHelper wifi;
  wifi.SetRemoteStationManager ("ns3::AarfWifiManager");

  WifiMacHelper mac;
  Ssid ssid = Ssid ("ns-3-ssid");
  mac.SetType ("ns3::StaWifiMac",
               "Ssid", SsidValue (ssid),
               "ActiveProbing", BooleanValue (false));

  NetDeviceContainer staDevices;
  staDevices = wifi.Install (phy, mac, wifiStaNodes);

  mac.SetType ("ns3::ApWifiMac",
               "Ssid", SsidValue (ssid));

  NetDeviceContainer apDevices;
  apDevices = wifi.Install (phy, mac, wifiApNode);

  MobilityHelper mobility;

  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (0.0),
                                 "MinY", DoubleValue (0.0),
                                 "DeltaX", DoubleValue (5.0),
                                 "DeltaY", DoubleValue (10.0),
                                 "GridWidth", UintegerValue (3),
                                 "LayoutType", StringValue ("RowFirst"));

  //mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel"); // for constant position
  
  
  
//mobility.SetPositionAllocator (positionAlloc);//dont use

//for varing sopeed distsnce
  mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
    "Speed",StringValue(speed), "Distance", DoubleValue(50.0));
  
     
                             
  mobility.Install (wifiStaNodes2);

  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (wifiApNode);

  InternetStackHelper stack;
  stack.Install (c);


  Ipv4AddressHelper address;

  address.SetBase ("10.1.3.0", "255.255.255.0");
  Ipv4InterfaceContainer staInterface = address.Assign (staDevices);
  Ipv4InterfaceContainer apInterface = address.Assign (apDevices);
    
    
    NS_LOG_INFO ("Assign IP Addresses.");
  Ipv4AddressHelper address1, address2, address3, address4, address5, address6, address7, address8;
  address1.SetBase ("10.1.1.0", "255.255.255.0");
  address2.SetBase ("10.1.2.0", "255.255.255.0");
  //address3.SetBase ("10.1.3.0", "255.255.255.0");
  address4.SetBase ("10.1.4.0", "255.255.255.0");
  address5.SetBase ("10.1.5.0", "255.255.255.0");

 
  Ipv4InterfaceContainer interfaces1 = address1.Assign (nd1);
  Ipv4InterfaceContainer interfaces2 = address2.Assign (nd2);
  
   Ipv4InterfaceContainer interfaces3 = address4.Assign (nd3);
    Ipv4InterfaceContainer interfaces4 = address5.Assign (nd4);
 
    
    
    



  uint16_t port = 9; // well-known echo port number
  
  //PACKET HISAAB SE BHEJ NE KE LIYE//////PACKET 50MS SE BHEJNE KE LIYE////
  /*
    TcpEchoServerHelper echoServer(9);
    ApplicationContainer serverApps = echoServer.Install(c.Get(2));//DESTINATION
    serverApps.Start(Seconds(1.0));
    serverApps.Stop(Seconds(11.0));

    // Create and setup TCP client on node N12
    TcpEchoClientHelper echoClient(interfaces1.GetAddress(0), 9);
    echoClient.SetAttribute("MaxPackets", UintegerValue((8 * 1000) / 50));  // 8 seconds worth of packets, every 50ms
    echoClient.SetAttribute("Interval", TimeValue (MilliSeconds (50)));
    echoClient.SetAttribute("PacketSize", UintegerValue(1024));

    ApplicationContainer clientApps = echoClient.Install(c.Get(12));//SOURCE NODE
    clientApps.Start(Seconds(3.0));  // Start communication at 3 seconds
    clientApps.Stop(Seconds(11.0));  // End communication at 11 seconds
    
    */
   
  
//TCP CODE THIS ONE///////////
  
BulkSendHelper source ("ns3::TcpSocketFactory",
InetSocketAddress (interfaces1.GetAddress (0), port));   //PUT HERE DESTINATION KA  INTERFACE 
// Set the amount of data to send in bytes. Zero is unlimited.
source.SetAttribute ("MaxBytes", UintegerValue (maxBytes));
ApplicationContainer sourceApps = source.Install (c.Get (12));   //SOURCE NODE  
sourceApps.Start (Seconds (3.0));
sourceApps.Stop (Seconds (11.3));

PacketSinkHelper sink ("ns3::TcpSocketFactory",
InetSocketAddress (Ipv4Address::GetAny (), port));
ApplicationContainer sinkApps = sink.Install (c.Get (2));  // DESTINATION NODE
sinkApps.Start (Seconds (4.0));
sinkApps.Stop (Seconds (11.3));

 

//no reply  from node 9 to node 3
 /*OnOffHelper onoff ("ns3::UdpSocketFactory", 
                     Address (InetSocketAddress (Ipv4Address ("255.255.255.255"), port)));
  onoff.SetConstantRate (DataRate ("500kb/s"));

  ApplicationContainer app = onoff.Install (c.Get (9));
  // Start the application
  app.Start (Seconds (1.0));
  app.Stop (Seconds (5.0));

  // Create an optional packet sink to receive these packets
  PacketSinkHelper sink ("ns3::UdpSocketFactory",
                         Address (InetSocketAddress (Ipv4Address::GetAny (), port)));
  app = sink.Install (c0.Get (0));
  app.Add (sink.Install (c.Get (3)));
  app.Start (Seconds (2.0));
  app.Stop (Seconds (5.0));*/

 /* Install TCP Receiver on the access point */
 
   // Simulator::Schedule(Seconds(1.1), &CalculateThroughput);




Ipv4GlobalRoutingHelper::PopulateRoutingTables ();


  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  NS_LOG_INFO ("Configure Tracing.");
 
  AsciiTraceHelper ascii;
  Ptr<OutputStreamWrapper> stream = ascii.CreateFileStream ("practice.tr");
  p2pa.EnableAsciiAll (stream);
   p2pb.EnableAsciiAll (stream);
  csma1.EnableAsciiAll(stream);
   csma2.EnableAsciiAll(stream);
   phy.EnableAsciiAll(stream);


  NS_LOG_INFO ("Run Simulation.");
  AnimationInterface anim ("darling2.xml");
  anim.SetConstantPosition ( c.Get(0), 5, 10);
  anim.SetConstantPosition ( c.Get(1), 15, 10);
  anim.SetConstantPosition ( c.Get(2), 10, 30);
  anim.SetConstantPosition ( c.Get(3), 20, 20);
  anim.SetConstantPosition ( c.Get(4), 25, 15);
  anim.SetConstantPosition ( c.Get(6), 35, 15);
  anim.SetConstantPosition ( c.Get(8), 45, 15);
  anim.SetConstantPosition ( c.Get(5), 35, 10);
  anim.SetConstantPosition ( c.Get(7), 35, 30);
  anim.SetConstantPosition(c.Get(9), 50, 20);
  anim.SetConstantPosition ( c.Get(10), 60, 35);
  anim.SetConstantPosition ( c.Get(11), 55, 10);
anim.SetConstantPosition ( c.Get(12), 65, 10);
    
   
  Simulator::Run ();
  Simulator::Destroy ();
  NS_LOG_INFO ("Done.");
}
