/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/* dumbbell.cc

Course:     Graduate Networks
Assignment: Project 3 - Network Simulation
Semester:   Fall 2020
Author:     Adam Brinckman

(Source1)--\                                                  /--(Sink1)
            \                                                /
        100Mbps, 1ms                                 100Mbps, 1ms
              \                                            /
              (Router)------ 10Mbps, 10ms ----------(Router)
              /                                            \
        100Mbps, 1ms                                 100Mbps, 1ms
            /                                                \
(Source2)--/                                                  \--(Sink2)

This simulation starts two CBR applications at both sources. Source1 starts with a bit rate of .5Mb/s and increases in scheduled steps. 
Source2 stays at a steady rate of 10Mb/s throughout the entire simulation. 
Tracers are hooked into the source and sink applications to track the number of packets sent and received. 
An event is scheduled to bump the bit rate every 15 seconds and also to record the percentage of packets dropped at each step.
*/

#include <fstream>
#include <iostream>
#include <vector>
#include <string>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/packet-sink.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/error-model.h"
#include "ns3/tcp-header.h"
#include "ns3/udp-header.h"
#include "ns3/enum.h"
#include "ns3/event-id.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/traffic-control-module.h"
#include "ns3/point-to-point-dumbbell.h"

#define DEBUG 0
#define SIM_STEP_DURATION 15.0  
#define SIM_STEPS 8             // Step up the bit rate # of times

using namespace ns3;

static Ptr<OutputStreamWrapper> bicInFlightStream;
static Ptr<OutputStreamWrapper> hyblaInFlightStream;
static Ptr<OutputStreamWrapper> htcpInFlightStream;
static Ptr<OutputStreamWrapper> highSpeedInFlightStream;
static Ptr<OutputStreamWrapper> newRenoInFlightStream;
static double duration;

NS_LOG_COMPONENT_DEFINE ("DumbbellSimulator");

static void
// BicInFlightTracer (uint32_t old, uint32_t inFlight)
BicInFlightTracer (const Ptr< const Packet > packet, const TcpHeader &header, const Ptr< const TcpSocketBase > socket)
{
  NS_UNUSED (header);
  NS_UNUSED (socket);
  *bicInFlightStream->GetStream () << Simulator::Now ().GetSeconds () << " " << packet->GetSize () << std::endl;
}

static void
HyblaInFlightTracer (uint32_t old, uint32_t inFlight)
{
  NS_UNUSED (old);
  *hyblaInFlightStream->GetStream () << Simulator::Now ().GetSeconds () << " " << inFlight << std::endl;
}

static void
HtcpInFlightTracer (uint32_t old, uint32_t inFlight)
{
  NS_UNUSED (old);
  *htcpInFlightStream->GetStream () << Simulator::Now ().GetSeconds () << " " << inFlight << std::endl;
}

static void
HighSpeedInFlightTracer (uint32_t old, uint32_t inFlight)
{
  NS_UNUSED (old);
  *highSpeedInFlightStream->GetStream () << Simulator::Now ().GetSeconds () << " " << inFlight << std::endl;
}

static void
// NewRenoInFlightTracer (uint32_t old, uint32_t inFlight)
NewRenoInFlightTracer (const Ptr< const Packet > packet, const TcpHeader &header, const Ptr< const TcpSocketBase > socket)
{
  NS_UNUSED (header);
  NS_UNUSED (socket);
  *newRenoInFlightStream->GetStream () << Simulator::Now ().GetSeconds () << " " << packet->GetSize () << std::endl;
}

static void
TraceBicInFlight (std::string &in_flight_file_name, std::string nodeId)
{
  AsciiTraceHelper ascii;
  bicInFlightStream = ascii.CreateFileStream (in_flight_file_name.c_str ());
  Config::ConnectWithoutContext ("/NodeList/" + nodeId + "/$ns3::TcpL4Protocol/SocketList/0/Rx", MakeCallback (&BicInFlightTracer));
}

static void
TraceHyblaInFlight (std::string &in_flight_file_name, std::string nodeId)
{
  AsciiTraceHelper ascii;
  hyblaInFlightStream = ascii.CreateFileStream (in_flight_file_name.c_str ());
  Config::ConnectWithoutContext ("/NodeList/" + nodeId + "/$ns3::TcpL4Protocol/SocketList/0/CongestionWindow", MakeCallback (&HyblaInFlightTracer));
}

static void
TraceHtcpInFlight (std::string &in_flight_file_name, std::string nodeId)
{
  AsciiTraceHelper ascii;
  htcpInFlightStream = ascii.CreateFileStream (in_flight_file_name.c_str ());
  Config::ConnectWithoutContext ("/NodeList/" + nodeId + "/$ns3::TcpL4Protocol/SocketList/0/CongestionWindow", MakeCallback (&HtcpInFlightTracer));
}

static void
TraceHighSpeedInFlight (std::string &in_flight_file_name, std::string nodeId)
{
  AsciiTraceHelper ascii;
  highSpeedInFlightStream = ascii.CreateFileStream (in_flight_file_name.c_str ());
  Config::ConnectWithoutContext ("/NodeList/" + nodeId + "/$ns3::TcpL4Protocol/SocketList/0/CongestionWindow", MakeCallback (&HighSpeedInFlightTracer));
}

static void
TraceNewRenoInFlight (std::string &in_flight_file_name, std::string nodeId)
{
  AsciiTraceHelper ascii;
  newRenoInFlightStream = ascii.CreateFileStream (in_flight_file_name.c_str ());
  Config::ConnectWithoutContext ("/NodeList/" + nodeId + "/$ns3::TcpL4Protocol/SocketList/0/Rx", MakeCallback (&NewRenoInFlightTracer));
}

int 
main (int argc, char *argv[])
{
  std::string transport_prot;
  double error_p = 0.0;
  double drop_p = 0.0000001;
  std::string bandwidth = "1Gbps";
  std::string delay = "1ms";
  std::string access_bandwidth = "10Gbps";
  std::string access_delay = "10ms";
  bool tracing = false;
  std::string prefix_file_name = "Tcp";
  uint64_t data_mbytes = 0;
  uint32_t mtu_bytes = 1500;
  duration = 100.0;
  uint32_t run = 0;
  bool legacy = false;
  bool sack = true;
  uint16_t num_flows = 4;
  std::string queue_disc_type = "ns3::CobaltQueueDisc";
  std::string recovery = "ns3::TcpClassicRecovery";

  CommandLine cmd (__FILE__);
  cmd.AddValue ("legacy", "Test with legacy", legacy);
  cmd.AddValue ("transport_prot",  "Single transport protocol to use", transport_prot);
  cmd.AddValue ("error_p", "Bit error rate", error_p);
  cmd.AddValue ("drop_p", "Packet drop rate", drop_p);
  cmd.AddValue ("bandwidth", "Bottleneck bandwidth", bandwidth);
  cmd.AddValue ("delay", "Bottleneck delay", delay);
  cmd.AddValue ("access_bandwidth", "Access link bandwidth", access_bandwidth);
  cmd.AddValue ("access_delay", "Access link delay", access_delay);
  cmd.AddValue ("tracing", "Flag to enable/disable tracing", tracing);
  cmd.AddValue ("prefix_name", "Prefix of output trace file", prefix_file_name);
  cmd.AddValue ("data", "Number of Megabytes of data to transmit", data_mbytes);
  cmd.AddValue ("mtu", "Size of IP packets to send in bytes", mtu_bytes);
  cmd.AddValue ("num_flows", "Number of flows", num_flows);
  cmd.AddValue ("duration", "Time to allow flows to run in seconds", duration);
  cmd.AddValue ("run", "Run index (for setting repeatable seeds)", run);
  cmd.AddValue ("sack", "Enable or disable SACK option", sack);
  cmd.AddValue ("num_flows", "Number of flows to run", num_flows);
  cmd.AddValue ("recovery", "Recovery algorithm type to use (e.g., ns3::TcpPrrRecovery", recovery);
  cmd.Parse (argc, argv);
  
  std::vector<std::string> transports;
  if (num_flows == 1)
    {
      transports.push_back("ns3::Tcp" + transport_prot);
    }
  else
    {
      transports.push_back("ns3::TcpBic");
      transports.push_back("ns3::TcpHybla");
      transports.push_back("ns3::TcpHtcp"); 
      transports.push_back("ns3::TcpHighSpeed");
    }
  
  
  if (legacy == true)
    {
      num_flows++; // account for legacy NewReno
      transports.push_back ("ns3::TcpNewReno");
    }

  uint32_t nLeftLeaf = num_flows;
  uint32_t nRightLeaf = num_flows;

  // Calculate the ADU size
  Header* temp_header = new Ipv4Header ();
  uint32_t ip_header = temp_header->GetSerializedSize ();
  NS_LOG_LOGIC ("IP Header size is: " << ip_header);
  delete temp_header;
  temp_header = new TcpHeader ();
  uint32_t tcp_header = temp_header->GetSerializedSize ();
  NS_LOG_LOGIC ("TCP Header size is: " << tcp_header);
  delete temp_header;
  uint32_t tcp_adu_size = mtu_bytes - 20 - (ip_header + tcp_header);
  NS_LOG_LOGIC ("TCP ADU size is: " << tcp_adu_size);

  // Set the simulation start and stop time
  double start_time = 0.1;
  double stop_time = start_time + duration;

  // 2 MB of TCP buffer
  Config::SetDefault ("ns3::TcpSocket::RcvBufSize", UintegerValue (1 << 21));
  Config::SetDefault ("ns3::TcpSocket::SndBufSize", UintegerValue (1 << 21));
  Config::SetDefault ("ns3::TcpSocketBase::Sack", BooleanValue (sack));

  Config::SetDefault ("ns3::TcpL4Protocol::RecoveryType",
                      TypeIdValue (TypeId::LookupByName (recovery)));

  DataRate access_b (access_bandwidth);
  DataRate bottle_b (bandwidth);
  Time access_d (access_delay);
  Time bottle_d (delay);

  uint32_t size = static_cast<uint32_t>((std::min (access_b, bottle_b).GetBitRate () / 8) *
    ((access_d + bottle_d) * 2).GetSeconds ());
  Config::SetDefault ("ns3::PfifoFastQueueDisc::MaxSize",
                      QueueSizeValue (QueueSize (QueueSizeUnit::PACKETS, size / mtu_bytes)));

  PointToPointHelper leftHelper, bottleneckHelper, rightHelper;
  leftHelper.SetDeviceAttribute ("DataRate", StringValue (access_bandwidth));
  leftHelper.SetChannelAttribute ("Delay", StringValue (access_delay));

  bottleneckHelper.SetDeviceAttribute ("DataRate", StringValue (bandwidth));
  bottleneckHelper.SetChannelAttribute ("Delay", StringValue (delay));

  rightHelper.SetDeviceAttribute ("DataRate", StringValue (access_bandwidth));
  rightHelper.SetChannelAttribute ("Delay", StringValue (access_delay));

  // Configure the error model
  // Here we use RateErrorModel with packet error rate
  Ptr<UniformRandomVariable> uv = CreateObject<UniformRandomVariable> ();
  uv->SetStream (50);
  RateErrorModel error_model;
  error_model.SetRandomVariable (uv);
  error_model.SetUnit (RateErrorModel::ERROR_UNIT_PACKET);
  error_model.SetRate (error_p);

  bottleneckHelper.SetDeviceAttribute ("ReceiveErrorModel", PointerValue (&error_model));

  InternetStackHelper stack;

  Ipv4AddressHelper leftIp;
  leftIp.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4AddressHelper rightIp;
  rightIp.SetBase ("10.2.1.0", "255.255.255.0");
  Ipv4AddressHelper routerIp;
  routerIp.SetBase ("10.3.1.0", "255.255.255.0");

  PointToPointDumbbellHelper dumbbellTopology (nLeftLeaf, leftHelper, nRightLeaf, rightHelper, bottleneckHelper);
  dumbbellTopology.InstallStack (stack);
  dumbbellTopology.AssignIpv4Addresses (leftIp, rightIp, routerIp);

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  // NetDeviceContainer gateways;
  // Ptr<NetDevice> gateway;
  // Ptr<TrafficControlLayer> tc = dumbbellTopology.GetRight ()->GetObject<TrafficControlLayer> ();
  // for (uint32_t i = 0; i < dumbbellTopology.GetRight ()->GetNDevices (); i++)
  //   {
  // gateway = dumbbellTopology.GetRight ()->GetDevice (0);
  // tc->DeleteRootQueueDiscOnDevice (gateway);
  // gateways.Add (gateway);
    // }

  // TrafficControlHelper tchCobalt;
  // Config::SetDefault ("ns3::PfifoFastQueueDisc::Pdrop", DoubleValue (drop_p));
  // tchCobalt.SetRootQueueDisc ("ns3::CobaltQueueDisc");
  // tchCobalt.Install (gateway);

  TypeId tid;
  uint32_t nodeId;
  std::string specificNode;
  for (uint32_t i = 0; i < num_flows; i++) 
    {
      tid = TypeId::LookupByName (transports[i]);
      std::cout << tid << std::endl;
      nodeId = dumbbellTopology.GetLeft (i)->GetId ();
      specificNode = std::string("/NodeList/") + std::to_string (nodeId) + "/$ns3::TcpL4Protocol/SocketType";
      Config::Set (specificNode, TypeIdValue (tid));
      nodeId = dumbbellTopology.GetRight (i)->GetId ();
      specificNode = std::string("/NodeList/") + std::to_string (nodeId) + "/$ns3::TcpL4Protocol/SocketType";
      Config::Set (specificNode, TypeIdValue (tid));
    }

  uint16_t port = 50000;
  Address sinkLocalAddress (InetSocketAddress (Ipv4Address::GetAny (), port));
  PacketSinkHelper sinkHelper ("ns3::TcpSocketFactory", sinkLocalAddress);

  ApplicationContainer sinkApp;
  for (uint16_t i = 0; i < num_flows; i++)
    {
      Address remoteAddress (InetSocketAddress (dumbbellTopology.GetRightIpv4Address (i), port));
      Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (tcp_adu_size));
      BulkSendHelper ftp ("ns3::TcpSocketFactory", Address ());
      ftp.SetAttribute ("Remote", AddressValue (remoteAddress));
      ftp.SetAttribute ("SendSize", UintegerValue (tcp_adu_size));
      ftp.SetAttribute ("MaxBytes", UintegerValue (data_mbytes * 1000000));

      ApplicationContainer sourceApp = ftp.Install (dumbbellTopology.GetLeft (i));
      sourceApp.Start (Seconds (start_time * i)); // + (i * 10)));
      sourceApp.Stop (Seconds (stop_time - 3));

      sinkHelper.SetAttribute ("Protocol", TypeIdValue (TcpSocketFactory::GetTypeId ()));
      sinkHelper.SetAttribute ("Local", AddressValue (remoteAddress));
      sinkApp = sinkHelper.Install (dumbbellTopology.GetRight (i));
      sinkApp.Start (Seconds (start_time * i)); // + (i * 10)));
      sinkApp.Stop (Seconds (stop_time));
    }

  // Set up tracing if enabled
  if (tracing)
    {
      if (num_flows > 2)
        {
          Simulator::Schedule (Seconds (0.00001), &TraceBicInFlight, prefix_file_name + "Bic-inflight.data", std::to_string(dumbbellTopology.GetLeft (0)->GetId ()));
          Simulator::Schedule (Seconds (0.10001), &TraceHyblaInFlight, prefix_file_name + "Hybla-inflight.data", std::to_string(dumbbellTopology.GetLeft (1)->GetId ()));
          Simulator::Schedule (Seconds (0.20001), &TraceHtcpInFlight, prefix_file_name + "Htcp-inflight.data", std::to_string(dumbbellTopology.GetLeft (2)->GetId ()));
          Simulator::Schedule (Seconds (0.30001), &TraceHighSpeedInFlight, prefix_file_name + "Highspeed-inflight.data", std::to_string(dumbbellTopology.GetLeft (3)->GetId ()));
          if (legacy == true)
            {
              Simulator::Schedule (Seconds (0.40001), &TraceNewRenoInFlight, prefix_file_name + "NewReno-inflight.data", std::to_string(dumbbellTopology.GetLeft (4)->GetId ()));
            }
        }
      else
        {
           Simulator::Schedule (Seconds (0.00001), &TraceBicInFlight, prefix_file_name + transport_prot + "-inflight.data", std::to_string(dumbbellTopology.GetRight (0)->GetId ()));
           if (legacy == true)
            {
              Simulator::Schedule (Seconds (0.10001), &TraceNewRenoInFlight, prefix_file_name + "NewReno-" + transport_prot + "-inflight.data", std::to_string(dumbbellTopology.GetRight (1)->GetId ()));
            }
        }
      
    }

  Simulator::Stop (Seconds (stop_time));
  Simulator::Run ();  
  Simulator::Destroy ();

  tid = TypeId::LookupByName ("ns3::PacketSink");
  Ptr<PacketSink> pSink = sinkApp.Get (0)->GetObject<PacketSink> ();
  double avgThroughput = (pSink->GetTotalRx () * 8) / (1e6 * stop_time);
  std::cout << "avg throughput: " << avgThroughput << "Mbps" << std::endl;
  
  return 0;
}
