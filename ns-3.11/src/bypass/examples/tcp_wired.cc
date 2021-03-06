#include <iostream>
#include <fstream>
#include <string>
#include <cassert>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/ipv4-list-routing-helper.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/ipv4-global-routing-helper.h"


using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("BypassExample");

int main (int argc, char *argv[])
{
	LogComponentEnable("BypassExample",LOG_LEVEL_ALL);
	uint32_t maxBytes = 100000;

	NS_LOG_INFO("Create nodes");
	NodeContainer c;
  	c.Create (7);
  	
	NodeContainer n01 = NodeContainer (c.Get (0), c.Get (1));
	NodeContainer n12 = NodeContainer (c.Get (1), c.Get (2));
	NodeContainer n23 = NodeContainer (c.Get (2), c.Get (3));
	NodeContainer n34 = NodeContainer (c.Get (3), c.Get (4));
	NodeContainer n45 = NodeContainer (c.Get (4), c.Get (5));
	NodeContainer n56 = NodeContainer (c.Get (5), c.Get (6));
	
	InternetStackHelper internet;
	internet.Install (c);

	// create the channels first without any IP addressing information
    	NS_LOG_INFO ("Create channels.");
  	PointToPointHelper p2p;
  	p2p.SetDeviceAttribute ("DataRate", StringValue ("54Mbps"));
	
  	NetDeviceContainer nd01 = p2p.Install (n01);
	NetDeviceContainer nd12 = p2p.Install (n12);
	NetDeviceContainer nd23 = p2p.Install (n23);
	NetDeviceContainer nd34 = p2p.Install (n34);
	NetDeviceContainer nd45 = p2p.Install (n45);
	NetDeviceContainer nd56 = p2p.Install (n56);
	
		//p2p.SetDeviceAttribute ("DataRate", StringValue ("11Mbps"));
	
	// Later,add IP addresses.
	NS_LOG_INFO ("Assign IP Addresses.");
  	Ipv4AddressHelper ipv4;
  	ipv4.SetBase ("10.1.1.0", "255.255.255.0");
	Ipv4InterfaceContainer i01 = ipv4.Assign (nd01);
	ipv4.SetBase ("10.1.2.0", "255.255.255.0");
	Ipv4InterfaceContainer i12 = ipv4.Assign (nd12);
	
	ipv4.SetBase ("10.1.3.0", "255.255.255.0");
	Ipv4InterfaceContainer i23 = ipv4.Assign (nd23);
	ipv4.SetBase ("10.1.4.0", "255.255.255.0");
  	Ipv4InterfaceContainer i34 = ipv4.Assign (nd34);
	ipv4.SetBase ("10.1.5.0", "255.255.255.0");
  	Ipv4InterfaceContainer i45 = ipv4.Assign (nd45);
	ipv4.SetBase ("10.1.6.0", "255.255.255.0");
  	Ipv4InterfaceContainer i56 = ipv4.Assign (nd56);

	// Create router nodes, initialize routing database and set up the routing
  	// tables in the nodes.
  	Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

	 NS_LOG_INFO ("Create Applications.");

	//	
	// Create a BulkSendApplication and install it on node 0
	//
  	uint16_t port = 9;  // well-known echo port number


  	BulkSendHelper source ("ns3::TcpSocketFactory",
                         InetSocketAddress (i56.GetAddress (1), port));
  	// Set the amount of data to send in bytes.  Zero is unlimited.
  	source.SetAttribute ("MaxBytes", UintegerValue (maxBytes));
  	ApplicationContainer sourceApps = source.Install (c.Get (0));
  	sourceApps.Start (Seconds (0.0));
  	sourceApps.Stop (Seconds (100.0));

	//
	// Create a PacketSinkApplication and install it on node 1
	//
  	PacketSinkHelper sink ("ns3::TcpSocketFactory",
                         InetSocketAddress (Ipv4Address::GetAny (), port));
  	ApplicationContainer sinkApps = sink.Install (c.Get (6));
  	sinkApps.Start (Seconds (0.0));
  	sinkApps.Stop (Seconds (100.0));

	Ptr<FlowMonitor> flowmon;
	FlowMonitorHelper flowmonHelper;
      	flowmon = flowmonHelper.InstallAll ();

	Simulator::Stop (Seconds (100));
	NS_LOG_INFO ("Run Simulation.");
	
  	Simulator::Run ();

	flowmon->SerializeToXmlFile ("simple-wired-routing.flowmon", false, false);

  	Simulator::Destroy ();

	NS_LOG_INFO ("Done");  	
	return 0;
}

