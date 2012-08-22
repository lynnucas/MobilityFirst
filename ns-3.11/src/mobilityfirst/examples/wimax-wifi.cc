/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 *  Copyright (c) 2007,2008, 2009 INRIA, UDcast
 *
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
 *
 * Author: Mohamed Amine Ismail <amine.ismail@sophia.inria.fr>
 *                              <amine.ismail@udcast.com>
 */

//
// Default network topology includes a base station (BS) and 2
// subscriber station (SS).

//      +-----+
//      | SS0 |
//      +-----+
//     10.1.1.1
//      -------
//        ((*))
//
//                  10.1.1.7
//               +------------+
//               |Base Station| ==((*))
//               +------------+
//
//        ((*))
//       -------
//      10.1.1.2
//       +-----+
//       | SS1 |
//       +-----+

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/config-store-module.h"
#include "ns3/wimax-module.h"
#include "ns3/internet-module.h"
#include "ns3/global-route-manager.h"
#include "ns3/ipcs-classifier-record.h"
#include "ns3/service-flow.h"
#include <iostream>

NS_LOG_COMPONENT_DEFINE ("WimaxSimpleExample");

using namespace ns3;

// splits a string on a token
std::vector<std::string> split(const std::string& strValue, char separator)
{
    std::vector<std::string> vecstrResult;
    int startpos=0;
    int endpos=0;
    
    endpos = strValue.find_first_of(separator, startpos);
    while (endpos != -1)
    {       
        vecstrResult.push_back(strValue.substr(startpos, endpos-startpos)); // add to vector
        startpos = endpos+1; //jump past sep
        endpos = strValue.find_first_of(separator, startpos); // find next
        if(endpos==-1)
        {
            std::string temp = strValue.substr(startpos);
            temp.erase(remove_if(temp.begin(), temp.end(), isspace), temp.end()); // not really required
            vecstrResult.push_back(temp);
        }
    }
    return vecstrResult;
}

// to handle data file input
class FileHandler {
    const char* mName;
    int mCount;
    std::vector<std::string> data;
    
public:
    FileHandler(const char *name)
    {
        mName = name;
        mCount = 0;
    }
    void process()
    {
        std::ifstream myfile;
        std::string line;
        
        myfile.open(mName);
        
        if(myfile.is_open()) {
            while(getline(myfile, line)) {
                data.push_back(line);
            }
            mCount = data.size();
            myfile.close();
        }
        else {
            std::cout << "Cannot open " << mName << std::endl;
            return;
        }
    }
    std::vector<std::string> serialize() {
        std::vector<std::string> result;
        std::vector<std::string> temp;
        for(std::vector<std::string>::iterator it = data.begin(); it != data.end(); ++it) {
            temp = split(*it, ',');
            result.insert(result.end(), temp.begin(), temp.end());
        }
        return result;
    }
    int count()
    {
        return mCount;
    }
};

/*

int main (int argc, char *argv[])
{
    FileHandler fh("data.txt");
    fh.process();
    std::vector<std::string> data = fh.serialize(); // check if the strings have spaces
    
    std::vector<std::string> lats;
    std::vector<std::string> longs;
    
    for (uint32_t i = 1; i< data.size() ;) 
    {
        lats.push_back(data[i]);
        i = i + 3;
    }
    
    for (uint32_t i = 2; i< data.size();)
    {
        longs.push_back(data[i]);
        i = i + 3;
    }
    
  bool verbose = false;

  int duration = 7, schedType = 0;
  WimaxHelper::SchedulerType scheduler = WimaxHelper::SCHED_TYPE_SIMPLE;

  CommandLine cmd;
  cmd.AddValue ("scheduler", "type of scheduler to use with the network devices", schedType);
  cmd.AddValue ("duration", "duration of the simulation in seconds", duration);
  cmd.AddValue ("verbose", "turn on all WimaxNetDevice log components", verbose);
  cmd.Parse (argc, argv);
  LogComponentEnable ("UdpClient", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpServer", LOG_LEVEL_INFO);
  switch (schedType)
    {
    case 0:
      scheduler = WimaxHelper::SCHED_TYPE_SIMPLE;
      break;
    case 1:
      scheduler = WimaxHelper::SCHED_TYPE_MBQOS;
      break;
    case 2:
      scheduler = WimaxHelper::SCHED_TYPE_RTPS;
      break;
    default:
      scheduler = WimaxHelper::SCHED_TYPE_SIMPLE;
    }

  NodeContainer ssNodes;
  NodeContainer bsNodes;
    
    int nbase = fh.count();
    int nsubs = 2;

  ssNodes.Create (nsubs);
  bsNodes.Create (nbase);

  WimaxHelper wimax;

  NetDeviceContainer ssDevs, bsDevs;

  ssDevs = wimax.Install (ssNodes,
                          WimaxHelper::DEVICE_TYPE_SUBSCRIBER_STATION,
                          WimaxHelper::SIMPLE_PHY_TYPE_OFDM,
                          scheduler);
  bsDevs = wimax.Install (bsNodes, WimaxHelper::DEVICE_TYPE_BASE_STATION, WimaxHelper::SIMPLE_PHY_TYPE_OFDM, scheduler);

  Ptr<SubscriberStationNetDevice> ss[nsubs];

  for (int i = 0; i < 2; i++)
    {
      ss[i] = ssDevs.Get (i)->GetObject<SubscriberStationNetDevice> ();
      ss[i]->SetModulationType (WimaxPhy::MODULATION_TYPE_QAM16_12);
    }

  //Ptr<BaseStationNetDevice> bs[nbase];

  //bs[0] = bsDevs.Get (0)->GetObject<BaseStationNetDevice> ();
   // bs[1] = bsDevs.Get (1)->GetObject<BaseStationNetDevice> ();

  InternetStackHelper stack;
  stack.Install (bsNodes);
  stack.Install (ssNodes);

  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");

  Ipv4InterfaceContainer SSinterfaces = address.Assign (ssDevs);
  Ipv4InterfaceContainer BSinterface = address.Assign (bsDevs);

  if (verbose)
    {
      wimax.EnableLogComponents ();  // Turn on all wimax logging
    }
    
    MobilityHelper mobility;
    Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
    for (int i = 0; i < nbase; i++)
    {
        positionAlloc->Add (Vector (atof(lats[i].c_str()), atof(longs[i].c_str()), 0.0));
        Ptr<Node> n0 = bsNodes.Get (i);
        Ptr<ConstantPositionMobilityModel> nLoc =  n0->GetObject<ConstantPositionMobilityModel> ();
        if (nLoc == 0)
        {
            nLoc = CreateObject<ConstantPositionMobilityModel> ();
            n0->AggregateObject (nLoc);
        }
        Vector nVec (atof(lats[i].c_str()), atof(longs[i].c_str()), 0);
        nLoc->SetPosition (nVec);
    }
    mobility.SetPositionAllocator (positionAlloc);
    mobility.Install (bsNodes);

  UdpServerHelper udpServer;
  ApplicationContainer serverApps;
  UdpClientHelper udpClient;
  ApplicationContainer clientApps;

  udpServer = UdpServerHelper (100);

  serverApps = udpServer.Install (ssNodes.Get (0));
  serverApps.Start (Seconds (6));
  serverApps.Stop (Seconds (duration));

  udpClient = UdpClientHelper (SSinterfaces.GetAddress (0), 100);
  udpClient.SetAttribute ("MaxPackets", UintegerValue (1200));
  udpClient.SetAttribute ("Interval", TimeValue (Seconds (0.5)));
  udpClient.SetAttribute ("PacketSize", UintegerValue (1024));

  clientApps = udpClient.Install (ssNodes.Get (1));
  clientApps.Start (Seconds (6));
  clientApps.Stop (Seconds (duration));

  Simulator::Stop (Seconds (duration + 0.1));

  wimax.EnablePcap ("wimax-simple-ss0", ssNodes.Get (0)->GetId (), ss[0]->GetIfIndex ());
  wimax.EnablePcap ("wimax-simple-ss1", ssNodes.Get (1)->GetId (), ss[1]->GetIfIndex ());
  //wimax.EnablePcap ("wimax-simple-bs0", bsNodes.Get (0)->GetId (), bs[0]->GetIfIndex ());
  //  wimax.EnablePcap ("wimax-simple-bs1", bsNodes.Get (1)->GetId (), bs[1]->GetIfIndex ());

  IpcsClassifierRecord DlClassifierUgs (Ipv4Address ("0.0.0.0"),
                                        Ipv4Mask ("0.0.0.0"),
                                        SSinterfaces.GetAddress (0),
                                        Ipv4Mask ("255.255.255.255"),
                                        0,
                                        65000,
                                        100,
                                        100,
                                        17,
                                        1);
  ServiceFlow DlServiceFlowUgs = wimax.CreateServiceFlow (ServiceFlow::SF_DIRECTION_DOWN,
                                                          ServiceFlow::SF_TYPE_RTPS,
                                                          DlClassifierUgs);

  IpcsClassifierRecord UlClassifierUgs (SSinterfaces.GetAddress (1),
                                        Ipv4Mask ("255.255.255.255"),
                                        Ipv4Address ("0.0.0.0"),
                                        Ipv4Mask ("0.0.0.0"),
                                        0,
                                        65000,
                                        100,
                                        100,
                                        17,
                                        1);
  ServiceFlow UlServiceFlowUgs = wimax.CreateServiceFlow (ServiceFlow::SF_DIRECTION_UP,
                                                          ServiceFlow::SF_TYPE_RTPS,
                                                          UlClassifierUgs);
  ss[0]->AddServiceFlow (DlServiceFlowUgs);
  ss[1]->AddServiceFlow (UlServiceFlowUgs);

  NS_LOG_INFO ("Starting simulation.....");
  Simulator::Run ();

  ss[0] = 0;
  ss[1] = 0;
  //bs[0] = 0;
   // bs[1] = 0;

  Simulator::Destroy ();
  NS_LOG_INFO ("Done.");

  return 0;
} */
 
 int main (int argc, char *argv[])
 {
 bool verbose = false;
 
 int duration = 7, schedType = 0;
 WimaxHelper::SchedulerType scheduler = WimaxHelper::SCHED_TYPE_SIMPLE;
 
 CommandLine cmd;
 cmd.AddValue ("scheduler", "type of scheduler to use with the network devices", schedType);
 cmd.AddValue ("duration", "duration of the simulation in seconds", duration);
 cmd.AddValue ("verbose", "turn on all WimaxNetDevice log components", verbose);
 cmd.Parse (argc, argv);
 LogComponentEnable ("UdpClient", LOG_LEVEL_INFO);
 LogComponentEnable ("UdpServer", LOG_LEVEL_INFO);
 switch (schedType)
 {
 case 0:
 scheduler = WimaxHelper::SCHED_TYPE_SIMPLE;
 break;
 case 1:
 scheduler = WimaxHelper::SCHED_TYPE_MBQOS;
 break;
 case 2:
 scheduler = WimaxHelper::SCHED_TYPE_RTPS;
 break;
 default:
 scheduler = WimaxHelper::SCHED_TYPE_SIMPLE;
 }
 
 NodeContainer ssNodes;
 NodeContainer bsNodes;
     
     int nbase = 2;
     int nsubs = 2;
 
 ssNodes.Create (nsubs);
 bsNodes.Create (nbase);
 
 WimaxHelper wimax;
 
 NetDeviceContainer ssDevs, bsDevs;
 
 ssDevs = wimax.Install (ssNodes,
 WimaxHelper::DEVICE_TYPE_SUBSCRIBER_STATION,
 WimaxHelper::SIMPLE_PHY_TYPE_OFDM,
 scheduler);
 bsDevs = wimax.Install (bsNodes, WimaxHelper::DEVICE_TYPE_BASE_STATION, WimaxHelper::SIMPLE_PHY_TYPE_OFDM, scheduler);
 
 wimax.EnableAscii ("bs-devices", bsDevs);
 wimax.EnableAscii ("ss-devices", ssDevs);
 
 Ptr<SubscriberStationNetDevice> ss[2];
 
 for (int i = 0; i < 2; i++)
 {
 ss[i] = ssDevs.Get (i)->GetObject<SubscriberStationNetDevice> ();
 ss[i]->SetModulationType (WimaxPhy::MODULATION_TYPE_QAM16_12);
 }
 
 //Ptr<BaseStationNetDevice> bs;
 
 //bs = bsDevs.Get (0)->GetObject<BaseStationNetDevice> ();
 
 InternetStackHelper stack;
 stack.Install (bsNodes);
 stack.Install (ssNodes);
 
 Ipv4AddressHelper address;
 address.SetBase ("10.1.1.0", "255.255.255.0");
 
 Ipv4InterfaceContainer SSinterfaces = address.Assign (ssDevs);
 Ipv4InterfaceContainer BSinterface = address.Assign (bsDevs);
 
 if (verbose)
 {
 wimax.EnableLogComponents ();  // Turn on all wimax logging
 }
UdpServerHelper udpServer;
ApplicationContainer serverApps;
UdpClientHelper udpClient;
ApplicationContainer clientApps;

udpServer = UdpServerHelper (100);

serverApps = udpServer.Install (ssNodes.Get (0));
serverApps.Start (Seconds (6));
serverApps.Stop (Seconds (duration));

udpClient = UdpClientHelper (SSinterfaces.GetAddress (0), 100);
udpClient.SetAttribute ("MaxPackets", UintegerValue (1200));
udpClient.SetAttribute ("Interval", TimeValue (Seconds (0.5)));
udpClient.SetAttribute ("PacketSize", UintegerValue (1024));

clientApps = udpClient.Install (ssNodes.Get (1));
clientApps.Start (Seconds (6));
clientApps.Stop (Seconds (duration));

Simulator::Stop (Seconds (duration + 0.1));

IpcsClassifierRecord DlClassifierUgs (Ipv4Address ("0.0.0.0"),
                                      Ipv4Mask ("0.0.0.0"),
                                      SSinterfaces.GetAddress (0),
                                      Ipv4Mask ("255.255.255.255"),
                                      0,
                                      65000,
                                      100,
                                      100,
                                      17,
                                      1);
ServiceFlow DlServiceFlowUgs = wimax.CreateServiceFlow (ServiceFlow::SF_DIRECTION_DOWN,
                                                        ServiceFlow::SF_TYPE_RTPS,
                                                        DlClassifierUgs);

IpcsClassifierRecord UlClassifierUgs (SSinterfaces.GetAddress (1),
                                      Ipv4Mask ("255.255.255.255"),
                                      Ipv4Address ("0.0.0.0"),
                                      Ipv4Mask ("0.0.0.0"),
                                      0,
                                      65000,
                                      100,
                                      100,
                                      17,
                                      1);
ServiceFlow UlServiceFlowUgs = wimax.CreateServiceFlow (ServiceFlow::SF_DIRECTION_UP,
                                                        ServiceFlow::SF_TYPE_RTPS,
                                                        UlClassifierUgs);
ss[0]->AddServiceFlow (DlServiceFlowUgs);
ss[1]->AddServiceFlow (UlServiceFlowUgs);

NS_LOG_INFO ("Starting simulation.....");
Simulator::Run ();

ss[0] = 0;
ss[1] = 0;
//bs = 0;

Simulator::Destroy ();
NS_LOG_INFO ("Done.");

return 0;
}