#include <iostream>
#include <fstream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"
#include "ns3/netanim-module.h"
#include "ns3/point-to-point-module.h"
//				    10.1.2.0
//	     10.1.1.0	        =================
//	=================	|	|	|
//	|	|	|     --n3	n4	n5------|
//	n0	n1	n2---|				n6
//			      --n7	n8	n9------|
//				|	|	|
//				=================
//				    10.1.3.0
using namespace ns3;
NS_LOG_COMPONENT_DEFINE ("CsmaBroadcastExample");
int main (int argc, char *argv[])
{
     LogComponentEnable ("CsmaBroadcastExample", LOG_LEVEL_INFO);
     LogComponentEnable ("OnOffApplication", LOG_LEVEL_INFO);
     
     Config::SetDefault ("ns3::CsmaNetDevice::EncapsulationMode", StringValue ("Dix"));
     CommandLine cmd;
     cmd.Parse (argc, argv);
   
     NS_LOG_INFO ("Create nodes.");
     NodeContainer c;
     c.Create (10);
     NodeContainer c0 = NodeContainer (c.Get(0),c.Get(1),c.Get(2));
     NodeContainer c1 = NodeContainer (c.Get(3),c.Get(4),c.Get(5));
     NodeContainer c2= NodeContainer(c.Get(7),c.Get(8),c.Get(9));
       
     NS_LOG_INFO ("Build Topology.");
     
     PointToPointHelper p2p;
     p2p.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
     p2p.SetChannelAttribute("Delay", StringValue("2ms"));
     
      CsmaHelper csma;
      csma.SetChannelAttribute ("DataRate", DataRateValue (DataRate (5000000)));
      csma.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (2)));
     
      NetDeviceContainer csmadevices1 = csma.Install(c0);
      NetDeviceContainer csmadevices2 = csma.Install(c1);
      NetDeviceContainer csmadevices3 = csma.Install (c2);
      NetDeviceContainer p2pdevices1= p2p.Install(c.Get(2),c.Get(3));
      NetDeviceContainer p2pdevices2= p2p.Install(c.Get(2),c.Get(7));
      NetDeviceContainer p2pdevices3= p2p.Install(c.Get(5),c.Get(6));
      NetDeviceContainer p2pdevices4= p2p.Install(c.Get(9),c.Get(6));
    
      NS_LOG_INFO ("Add IP Stack.");
      InternetStackHelper internet;
      internet.Install (c);
    
      NS_LOG_INFO ("Assign IP Addresses.");
      Ipv4AddressHelper csma_addr1,csma_addr2,csma_addr3,p2p_addr1,p2p_addr2,p2p_addr3,p2p_addr4;
      csma_addr1.SetBase ("10.1.1.0", "255.255.255.0");
      csma_addr2.SetBase ("10.1.2.0", "255.255.255.0");
      csma_addr3.SetBase("10.1.3.0","255.255.255.0");
      p2p_addr1.SetBase("10.1.4.0","255.255.255.0");
      p2p_addr2.SetBase("10.1.5.0","255.255.255.0");
      p2p_addr3.SetBase("10.1.6.0","255.255.255.0");
      p2p_addr4.SetBase("10.1.7.0","255.255.255.0");
      
      Ipv4InterfaceContainer interfaces1 = csma_addr1.Assign (csmadevices1);
      Ipv4InterfaceContainer interfaces2 = csma_addr2.Assign (csmadevices2);
      Ipv4InterfaceContainer interfaces3 = csma_addr3.Assign (csmadevices3);
      Ipv4InterfaceContainer interfaces4 = p2p_addr1.Assign (p2pdevices1);
      Ipv4InterfaceContainer interfaces5 = p2p_addr2.Assign (p2pdevices2);
      Ipv4InterfaceContainer interfaces6 = p2p_addr3.Assign (p2pdevices3);
      Ipv4InterfaceContainer interfaces7 = p2p_addr4.Assign (p2pdevices4);
    
      NS_LOG_INFO ("Configure broadcasting.");
 
     OnOffHelper onoffCSMA ("ns3::UdpSocketFactory", Address (InetSocketAddress (Ipv4Address ("255.255.255.255"), 9)));
     onoffCSMA.SetConstantRate (DataRate ("255b/s"));
     onoffCSMA.SetAttribute ("PacketSize", UintegerValue (128));

     ApplicationContainer srcCCSMA1 = onoffCSMA.Install (c.Get (0));
     srcCCSMA1.Add(onoffCSMA.Install(c.Get(3)));
     srcCCSMA1.Add(onoffCSMA.Install(c.Get(7)));
     srcCCSMA1.Start (Seconds (1.0));
     srcCCSMA1.Stop (Seconds (10.0));
     
     OnOffHelper onoffp2p ("ns3::UdpSocketFactory", Address (InetSocketAddress (Ipv4Address ("255.255.255.255"), 9)));
     onoffp2p.SetConstantRate (DataRate ("255b/s"));
     onoffp2p.SetAttribute ("PacketSize", UintegerValue (128));

     ApplicationContainer srcp2p = onoffp2p.Install (c.Get (2));
     srcp2p.Add(onoffp2p.Install(c.Get(5)));
     srcp2p.Add(onoffp2p.Install(c.Get(9)));
     srcp2p.Add(onoffp2p.Install(c.Get(6)));
     srcp2p.Add(onoffp2p.Install(c.Get(7)));
     srcp2p.Add(onoffp2p.Install(c.Get(3)));
     srcp2p.Start (Seconds (1.0));
     srcp2p.Stop (Seconds (10.0));
     
     PacketSinkHelper sink ("ns3::UdpSocketFactory",InetSocketAddress (Ipv4Address::GetAny (), 9));
   
     ApplicationContainer sinkC = sink.Install (c.Get (6));
     sinkC.Start (Seconds (1.0));
     sinkC.Stop (Seconds (10.0));
   
     NS_LOG_INFO ("Configure Tracing.");
     AsciiTraceHelper ascii;
     csma.EnableAsciiAll (ascii.CreateFileStream ("paavques.tr"));
    
   
     AnimationInterface anim("animFile_paavques.xml"); 
     for (uint32_t i = 0; i < 10; ++i) 
     {
        anim.SetConstantPosition(c.Get(i), 100, 10*i);
    }
     NS_LOG_INFO ("Run Simulation.");
     Simulator::Run ();
     Ptr<PacketSink> sink1 = DynamicCast<PacketSink> (sinkC.Get (0));
     std::cout << "Total Bytes Received: " << sink1->GetTotalRx () << std::endl;
     Simulator::Destroy ();
     NS_LOG_INFO ("Done.");
   }