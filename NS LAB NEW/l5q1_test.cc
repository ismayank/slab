// n0    n1      n3     n4       n5
// |    |      |     |       |
// |    |      |     |       |
//=============================

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/netanim-module.h"
using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("l5q1");
int main(int argc, char *argv[])
{
  LogComponentEnable("l5q1",LOG_LEVEL_INFO);

  //uint32_t nCsma = 5;
uint32_t maxBytes = 512;
 std::string datarate="0.5Mbps";
  std::string delay="2ms";

  NodeContainer c;
  c.Create(5);

  NodeContainer c0 = NodeContainer (c.Get (0), c.Get (1),c.Get (2),c.Get (3),c.Get (4));

  CsmaHelper csma;
  csma.SetQueue ("ns3::DropTailQueue", "MaxSize", StringValue ("50p"));
  csma.SetChannelAttribute ("DataRate", DataRateValue (DataRate (5000000)));  //BANDWIDTH
  csma.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (2)));  //DELAY

  NetDeviceContainer nd0 = csma.Install (c0);

  NS_LOG_INFO ("Add IP Stack.");
  InternetStackHelper internet;
  internet.Install (c);

  NS_LOG_INFO ("Assign IP Addresses.");
  Ipv4AddressHelper address1;
  address1.SetBase ("10.1.1.0", "255.255.255.0");

  Ipv4InterfaceContainer interfaces1 = address1.Assign (nd0);

uint16_t port = 9; // well-known echo port number
BulkSendHelper source ("ns3::TcpSocketFactory",
InetSocketAddress (interfaces1.GetAddress (4), port));   //PUT HERE DESTINATION KA  INTERFACE 
// Set the amount of data to send in bytes. Zero is unlimited.
source.SetAttribute ("MaxBytes", UintegerValue (maxBytes));
ApplicationContainer sourceApps = source.Install (c.Get (0));   //SOURCE NODE  
sourceApps.Start (Seconds (2.0));
sourceApps.Stop (Seconds (5.3));

PacketSinkHelper sink ("ns3::TcpSocketFactory",
InetSocketAddress (Ipv4Address::GetAny (), port));
ApplicationContainer sinkApps = sink.Install (c.Get (4));  // DESTINATION NODE
sinkApps.Start (Seconds (2.0));
sinkApps.Stop (Seconds (5.3));

Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  NS_LOG_INFO ("Configure Tracing.");
 
  AsciiTraceHelper ascii;
  Ptr<OutputStreamWrapper> stream = ascii.CreateFileStream ("practice.tr");
  //p2p.EnableAsciiAll (stream);
  csma.EnableAsciiAll(stream);


NS_LOG_INFO ("Run Simulation.");
  AnimationInterface anim ("practice1.xml");
  anim.SetConstantPosition ( c.Get(0), 1, 20);
  anim.SetConstantPosition ( c.Get(1), 10, 20);
  anim.SetConstantPosition ( c.Get(2), 20, 20);
  anim.SetConstantPosition ( c.Get(3), 30, 20);
  anim.SetConstantPosition ( c.Get(4), 40, 20);
  

    

  Simulator::Run ();
  Simulator::Destroy ();
  NS_LOG_INFO ("Done.");

  return 0;

}



 











