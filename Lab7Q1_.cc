/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

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

// Default Network Topology
//
//   Wifi 10.1.3.0
//  AP
//  *    *    *    *    *    *    *    *    *    *
//  |    |    |    |    |    |    |    |    |    |
// n0   n1   n2   n3   n4   n5   n6   n7   n8   n9

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("ThirdScriptExample");

int 
main (int argc, char *argv[])
{
  bool verbose = true;
  uint32_t nWifi = 10 ;

  CommandLine cmd (__FILE__);
  cmd.AddValue ("nWifi", "Number of wifi STA devices", nWifi);
  cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);

  cmd.Parse (argc,argv);

  // The underlying restriction of 18 is due to the grid position
  // allocator's configuration; the grid layout will exceed the
  // bounding box if more than 18 nodes are provided.
  // if (nWifi > 18)
  //   {
  //     std::cout << "nWifi should be 18 or less; otherwise grid layout exceeds the bounding box" << std::endl;
  //     return 1;
  //   }

  if (verbose)
    {
      LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
      LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
    }

  NodeContainer wifiStaNodes;     
  wifiStaNodes.Create (nWifi);                      //CREATE TOTAL NO OF NODES
  NodeContainer wifiApNode = wifiStaNodes.Get (0);  //MAKE ONE OF THEM AS ACCESS POINT

  YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
  YansWifiPhyHelper phy;
  phy.SetChannel (channel.Create ());              //CREATE WIFI CHANNEL
  NS_LOG_INFO ("Create Applications.");     
  

  WifiHelper wifi;
  wifi.SetRemoteStationManager ("ns3::AarfWifiManager");   //RATE CONTROL ALGORITHM

  WifiMacHelper mac;
  Ssid ssid = Ssid ("ns-3-ssid");
  mac.SetType ("ns3::StaWifiMac",
               "Ssid", SsidValue (ssid),
               "ActiveProbing", BooleanValue (false));

  NetDeviceContainer staDevices;
  staDevices = wifi.Install (phy, mac, wifiStaNodes);     //INSTALL WIFI ON ALL NODES

  mac.SetType ("ns3::ApWifiMac",
               "Ssid", SsidValue (ssid));

  NetDeviceContainer apDevices;
  apDevices = wifi.Install (phy, mac, wifiApNode);         // INSTALL WIFI ON ALL ACCESS POINT 

  MobilityHelper mobility;

  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (0.0),
                                 "MinY", DoubleValue (0.0),
                                 "DeltaX", DoubleValue (5.0),
                                 "DeltaY", DoubleValue (10.0),
                                 "GridWidth", UintegerValue (3),
                                 "LayoutType", StringValue ("RowFirst"));

  mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                             "Bounds", RectangleValue (Rectangle (-50, 50, -50, 50)));
  mobility.Install (wifiStaNodes);        //SET MOBILITY ON ALL STATION NODES

  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  //mobility.Install (wifiApNode);                    // FIX THE ACCESS POINT

  InternetStackHelper stack;
  //stack.Install (wifiApNode);
  stack.Install (wifiStaNodes);


  Ipv4AddressHelper address;

  address.SetBase ("10.1.3.0", "255.255.255.0");
  Ipv4InterfaceContainer staInterface = address.Assign (staDevices);
  Ipv4InterfaceContainer apInterface = address.Assign (apDevices);

  UdpEchoServerHelper echoServer (9);

  ApplicationContainer serverApps = echoServer.Install (wifiStaNodes.Get (nWifi - 1));
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (10.0));

  UdpEchoClientHelper echoClient (apInterface.GetAddress (0), 9);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (1));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

  ApplicationContainer clientApps = 
    echoClient.Install (wifiApNode.Get (0));
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (10.0));

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  AnimationInterface anim("Lab7Q1.xml");
  //Wireless Trace generation
  //wifiPhyHelper.EnableAsciiAll("l7r1-50Mbps.tr");

//Config::SetDefault("ns3::TcpL4Protocol::SocketType",StringValue("ns3::TcpNewReno"));
/*[08-02 11:31] KHUSHI CHANDRA - 210953110
    

MobilityHelper mobility;

  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

  mobility.Install (nodes);



  Ptr<ConstantPositionMobilityModel> s1 = nodes.Get (0)->GetObject<ConstantPositionMobilityModel> ();

  Ptr<ConstantPositionMobilityModel> s2 = nodes.Get (1)->GetObject<ConstantPositionMobilityModel> ();

  Ptr<ConstantPositionMobilityModel> s3 = nodes.Get (2)->GetObject<ConstantPositionMobilityModel> ();



  s1->SetPosition (Vector ( 0, 0, 0  ));

  s2->SetPosition (Vector ( 30, 90., 0  ));

  s3->SetPosition (Vector ( 90, 90, 0 ));*/

  Simulator::Stop (Seconds (10.0));

  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
