#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/ssid.h"
#include "ns3/netanim-module.h"
#include "ns3/rip-helper.h"

// Default Network Topology
//
//   Wifi 10.1.3.0
//                 AP
//  *    *    *    *
//  |    |    |    |    10.1.1.0
// n5   n6   n7   n0 -------------- n1   n2   n3   n4
//                   point-to-point  |    |    |    |
//                                   ================
//                                     LAN 10.1.2.0

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("ThirdScriptExample");

void
TearDownLink(Ptr<Node> nodeA, Ptr<Node> nodeB, uint32_t interfaceA, uint32_t interfaceB)
{
    nodeA->GetObject<Ipv4>()->SetDown(interfaceA);
    nodeB->GetObject<Ipv4>()->SetDown(interfaceB);
}


int 
main (int argc, char *argv[])
{
  bool verbose = true;
  uint32_t nCsma = 3;
  uint32_t nWifi = 3;
  bool tracing = false;
  std::string speed=     "ns3::ConstantRandomVariable[Constant=20.5]";   
  std::string distance = "ns3::ConstantRandomVariable[Constant=50.0]";
    bool printRoutingTables = false;
    bool showPings = false;
    std::string SplitHorizon("PoisonReverse");


  CommandLine cmd (__FILE__);
  cmd.AddValue ("nCsma", "Number of \"extra\" CSMA nodes/devices", nCsma);
  cmd.AddValue ("nWifi", "Number of wifi STA devices", nWifi);
  cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);
  cmd.AddValue ("tracing", "Enable pcap tracing", tracing);
  cmd.AddValue("printRoutingTables",
                 "Print routing tables at 30, 60 and 90 seconds",
                 printRoutingTables);
    cmd.AddValue("showPings", "Show Ping6 reception", showPings);
    cmd.AddValue("splitHorizonStrategy",
                 "Split Horizon strategy to use (NoSplitHorizon, SplitHorizon, PoisonReverse)",
                 SplitHorizon);


  cmd.Parse (argc,argv);
  if (SplitHorizon == "NoSplitHorizon")
    {
        Config::SetDefault("ns3::Rip::SplitHorizon", EnumValue(RipNg::NO_SPLIT_HORIZON));
    }
    else if (SplitHorizon == "SplitHorizon")
    {
        Config::SetDefault("ns3::Rip::SplitHorizon", EnumValue(RipNg::SPLIT_HORIZON));
    }
    else
    {
        Config::SetDefault("ns3::Rip::SplitHorizon", EnumValue(RipNg::POISON_REVERSE));
    }


  // The underlying restriction of 18 is due to the grid position
  // allocator's configuration; the grid layout will exceed the
  // bounding box if more than 18 nodes are provided.
  if (nWifi > 18)
    {
      std::cout << "nWifi should be 18 or less; otherwise grid layout exceeds the bounding box" << std::endl;
      return 1;
    }

  if (verbose)
    {
      LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
      LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
    }

  NodeContainer c;
  c.Create (12);
  NodeContainer  c0 = NodeContainer(c.Get(9), c.Get(10), c.Get(11));
  NodeContainer  p0 = NodeContainer(c.Get(5), c.Get(7));
  NodeContainer  p1 = NodeContainer(c.Get(7), c.Get(8));
  NodeContainer  p2 = NodeContainer(c.Get(8), c.Get(10));
  NodeContainer  wifiStaNodes = NodeContainer(c.Get(1), c.Get(2), c.Get(3), c.Get(4));
  wifiStaNodes.Add(c.Get(5));
  wifiStaNodes.Add(c.Get(6));
  NodeContainer  wifiStaNode2 = NodeContainer(c.Get(1), c.Get(2), c.Get(3), c.Get(4));
  wifiStaNode2.Add(c.Get(6));
  NodeContainer wifiApNode = c.Get (5);

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));


  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
  csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));

  NetDeviceContainer nd0 = pointToPoint.Install(p0);
  NetDeviceContainer nd1 = pointToPoint.Install(p1);
  NetDeviceContainer nd2 = pointToPoint.Install(p2);
  NetDeviceContainer nd3 = csma.Install(c0);

  

  YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
  YansWifiPhyHelper phy;
  phy.SetChannel (channel.Create ());

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

  RipHelper ripRouting;
    ripRouting.ExcludeInterface(c.Get(9), 1);
    ripRouting.ExcludeInterface(c.Get(1), 1);

    //ripRouting.SetInterfaceMetric(c.Get(3), 3, 10);
    //ripRouting.SetInterfaceMetric(c.Get(4), 1, 10);

    Ipv4ListRoutingHelper listRH;
    listRH.Add(ripRouting, 0);
    //  Ipv4StaticRoutingHelper staticRh;
    //  listRH.Add (staticRh, 5);

  MobilityHelper mobility;

mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (0.0),
                                 "MinY", DoubleValue (0.0),
                                 "DeltaX", DoubleValue (5.0),
                                 "DeltaY", DoubleValue (10.0),
                                 "GridWidth", UintegerValue (3),
                                 "LayoutType", StringValue ("RowFirst"));

  mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                             "Bounds", RectangleValue (Rectangle (-50, 50, -50, 50)), "Distance", DoubleValue(50.0));


  mobility.Install (wifiStaNode2);

  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (wifiApNode);

  InternetStackHelper stack;
  stack.SetIpv6StackInstall(false);
  stack.SetRoutingHelper(listRH);
  stack.Install (c);


  Ipv4AddressHelper address;

  address.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i0 = address.Assign (nd0);

  address.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer i1 = address.Assign (nd1);
  
  address.SetBase ("10.1.3.0", "255.255.255.0");
  Ipv4InterfaceContainer i2 = address.Assign (nd2);

  address.SetBase ("10.1.4.0", "255.255.255.0");
  Ipv4InterfaceContainer i3 = address.Assign (nd3);

  address.SetBase ("10.1.5.0", "255.255.255.0");
  Ipv4InterfaceContainer i4 = address.Assign (staDevices);
  Ipv4InterfaceContainer i5 = address.Assign (apDevices);


    Ptr<Ipv4StaticRouting> staticRouting;
staticRouting = Ipv4RoutingHelper::GetRouting<Ipv4StaticRouting>(
    c.Get(9)->GetObject<Ipv4>()->GetRoutingProtocol());
if (staticRouting)
{
    NS_LOG_INFO("yes");
    staticRouting->SetDefaultRoute("10.1.4.2", 1);
}

staticRouting = Ipv4RoutingHelper::GetRouting<Ipv4StaticRouting>(
    c.Get(1)->GetObject<Ipv4>()->GetRoutingProtocol());
if (staticRouting)
{
    staticRouting->SetDefaultRoute("10.1.5.1", 1);
}


  UdpEchoServerHelper echoServer (9);

  ApplicationContainer serverApps = echoServer.Install (c.Get (1));
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (10.0));

  UdpEchoClientHelper echoClient (i4.GetAddress (0), 9);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (1));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

  ApplicationContainer clientApps = 
    echoClient.Install (c.Get (9));
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (10.0));

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  Simulator::Stop (Seconds (10.0));

  AsciiTraceHelper ascii;
  Ptr<OutputStreamWrapper>stream = ascii.CreateFileStream("endsem1.tr");
  pointToPoint.EnableAsciiAll(stream);
  csma.EnableAsciiAll(stream);
  phy.EnableAsciiAll(stream);
  AnimationInterface anim("endsem1.xml");
  anim.SetConstantPosition(c.Get(10), 50, 50);
  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}

