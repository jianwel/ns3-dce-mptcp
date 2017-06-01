/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

//
// Handoff scenario with Multipath TCPed iperf
//
// Simulation Topology:
// Scenario: H1 has 3G
//           during movement, MN keeps iperf session to SV.
//
//   <--------------------            ----------------------->
//                  LTE               Ethernet
//                   sim0 +----------+ sim1
//                  +------|  LTE  R  |------+
//                  |     +----------+      |
//              +---+                       +-----+
//          sim0|                                 |sim0
//     +----+---+                                 +----+---+
//     |   H1   |                                 |   H2   |
//     +---+----+                                 +----+---+
//          sim1|                                 |sim1
//              +--+                        +-----+
//                 | sim0 +----------+ sim1 |
//                  +-----|  WiFi R  |------+
//                        +----------+      
//                  WiFi              Ethernet
//   <--------------------            ----------------------->

#include "ns3/network-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/dce-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/wifi-module.h"
#include "ns3/lte-module.h"
#include "ns3/mobility-module.h"
#include "ns3/applications-module.h"
#include "ns3/netanim-module.h"
#include "ns3/constant-position-mobility-model.h"
#include "ns3/config-store-module.h"
#include "ns3/csma-module.h"
#include "ns3/bridge-module.h"
#include "ns3/csma-helper.h"
#include "ns3/inet-socket-address.h"

#include "ns3/config-store.h"

#include "ns3/flow-monitor.h"
#include "ns3/flow-monitor-helper.h"

#include <unistd.h>

using namespace ns3;
using std::cout;
using std::endl;


NS_LOG_COMPONENT_DEFINE ("DceMptcpLteWifi");

void setPos (Ptr<Node> n, int x, int y, int z)
{
  Ptr<ConstantPositionMobilityModel> loc = CreateObject<ConstantPositionMobilityModel> ();
  n->AggregateObject (loc);
  Vector locVec2 (x, y, z);
  loc->SetPosition (locVec2);
}

void
PrintTcpFlags (std::string key, std::string value)
{
  NS_LOG_INFO (key << "=" << value);
}

unsigned long int m_bytesTotal[5];
const int INTERVAL=5;
int nUE = 1;


const int AREA_X = 5000;
const int AREA_Y  = 5000;

const int IPERF_RANDOM_SART = 5; 


void
ReceivedPacket ( std::string context, Ptr<const Packet> p, const
Address& addr)
{
    m_bytesTotal[0] += p->GetSize ();
   // cout<<context<<" "<<p->GetSize ()<<endl;
}

void Throughput () // in Mbps calculated every 2s 
{ 
  double mbps = (((m_bytesTotal[0] * 8.0f) / 1000000.0f)/(double)INTERVAL);
  double time_tp = Simulator::Now ().GetSeconds (); 
  cout << "time: " << time_tp <<"\t"<< "NS3 total Throughput: " << mbps <<"Mbps"<<endl; 
  m_bytesTotal[0] = 0; 
  Simulator::Schedule (Seconds (INTERVAL), &Throughput); 
}


int main (int argc, char *argv[])
{
  LogComponentEnable ("DceMptcpLteWifi", LOG_LEVEL_ALL);
  std::string bufSize = "";
  bool disWifi = false;
  bool disLte = false;
  bool isDownlink = true;
  double stopTime = 20.0;
  std::string p2pdelay = "10ms";


  CommandLine cmd;
  cmd.AddValue ("bufsize", "Snd/Rcv buffer size.", bufSize);
  cmd.AddValue ("disWifi", "Disable WiFi.", disWifi);
  cmd.AddValue ("disLte", "Disable LTE.", disLte);
  cmd.AddValue ("isDownlink", "is Downlink flow or uplink flow.", isDownlink);
  cmd.AddValue ("nUE", "the number of UEs.", nUE);
  cmd.AddValue ("stopTime", "StopTime of simulatino.", stopTime);
  cmd.AddValue ("p2pDelay", "Delay of p2p links. default is 10ms.", p2pdelay);
  cmd.Parse (argc, argv);

ConfigStore inputConfig;
inputConfig.ConfigureDefaults ();


     MobilityHelper mobility;
   mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                  "MinX", DoubleValue (1.0),
                                  "MinY", DoubleValue (1.0),
                                  "DeltaX", DoubleValue (5.0),
                                  "DeltaY", DoubleValue (5.0),
                                  "GridWidth", UintegerValue (3),
                                  "LayoutType", StringValue ("RowFirst"));
   mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                              "Mode", StringValue ("Time"),
                              "Time", StringValue ("2s"),
                              "Speed", StringValue ("ns3::ConstantRandomVariable[Constant=1.0]"),
                              "Bounds", RectangleValue (Rectangle (0.0, 20.0, 0.0, 20.0)));

  
  Ptr<FlowMonitor> flowMonitor;
  FlowMonitorHelper flowHelper;
  flowMonitor = flowHelper.InstallAll();

uint pgwI = 0;
uint routerI = 0;




  if (disWifi && disLte)
    {
      NS_LOG_INFO ("no active interface");
      return 0;
    }

  if (disWifi)
      NS_LOG_INFO ("wifi is disabled");
      //cout<<"wifi is disabled"<<endl;

  if (disLte)
      NS_LOG_INFO ("lte is disabled");
   
  GlobalValue::Bind ("ChecksumEnabled", BooleanValue (true));
  NodeContainer nodes, routers, cn;

  //node 0-9
  nodes.Create (nUE);

  //wifi router is node 10
  routers.Create (1);

  //cn is node 11
  cn.Create(nUE);

  DceManagerHelper dceManager;
 // dceManager.SetNetworkStack ("ns3::LinuxSocketFdFactory",
 //                             "Library", StringValue ("liblinux.so"));

    dceManager.SetNetworkStack ("ns3::LinuxSocketFdFactory",
                              "Library", StringValue ("libsim-linux3.14.33.so"));
  LinuxStackHelper stack;
  stack.Install (nodes);
  stack.Install (routers);
  stack.Install (cn);

  dceManager.Install (nodes);
  dceManager.Install (routers);
  dceManager.Install (cn);

  //PointToPointHelper pointToPoint;
  NetDeviceContainer devices1;
   NetDeviceContainer                 devices2, devicesWifi;

  NetDeviceContainer csmaNetDevice;
  NetDeviceContainer csmaNetDeviceWifi;
  NetDeviceContainer wifiDevice;
  NetDeviceContainer lteBridgeDevices;
  NetDeviceContainer wifiBridgeDevices;


 BridgeHelper bridge;
 BridgeHelper bridgeW;


   //mobility.Install (nodes);
  

  Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();

  //epc is node 12
  Ptr<PointToPointEpcHelper> epcHelper = CreateObject<PointToPointEpcHelper> ();
  YansWifiPhyHelper phy;

  Ipv4AddressHelper address1, address2;
  std::ostringstream cmd_oss;
  address1.SetBase ("10.1.0.0", "255.255.255.0");
  address2.SetBase ("10.2.0.0", "255.255.255.0");
  Ipv4InterfaceContainer if1, if2, if2Wifi;
 // pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("10000Mbps"));
 // pointToPoint.SetChannelAttribute ("Delay", StringValue (p2pdelay));
  Ptr<RateErrorModel> em1 =
    CreateObjectWithAttributes<RateErrorModel> ("RanVar", StringValue ("ns3::UniformRandomVariable[Min=0.0,Max=1.0]"),
                                                "ErrorRate", DoubleValue (0.01),
                                                "ErrorUnit", EnumValue (RateErrorModel::ERROR_UNIT_PACKET)
                                                );

  //setPos (nodes.Get (0), -20, 30 / 2, 0);

       for(uint32_t u = 0; u < nodes.GetN (); ++u){

            setPos (cn.Get (u), 100, 30 / 2, 0);
            setPos (nodes.Get (u), -20, 30 / 2, 0);
       }
/*

  NodeContainer csmaSwitchs;
  csmaSwitchs.Create (2);

  */
  Config::SetDefault("ns3::DropTailQueue::MaxPackets", UintegerValue(1000000000));
  CsmaHelper csmahelper;
  CsmaHelper csmahelperWifi;

  csmahelper.SetChannelAttribute ("DataRate", DataRateValue (50000000000));
  csmahelper.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (1))); 

  csmahelperWifi.SetChannelAttribute ("DataRate", DataRateValue (50000000000));
  csmahelperWifi.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (1))); 


  // LTE
  if (!disLte)
    {
      // Left link: H1 <-> LTE-R
      NodeContainer enbNodes;

      Ptr<Node> bridgeLte = CreateObject<Node> ();
      //node 13
      enbNodes.Create(1);

      lteHelper->SetEpcHelper (epcHelper);
      Ptr<Node> pgw = epcHelper->GetPgwNode ();
      //setPos (enbNodes.Get (0), 60, 40000, 0);
      setPos (enbNodes.Get (0), -20, 14, 0);

        //lteHelper->SetSchedulerType ("ns3::PssFfMacScheduler");
        lteHelper->SetSchedulerType ("ns3::PfFfMacScheduler");

      NetDeviceContainer enbLteDevs = lteHelper->InstallEnbDevice (enbNodes);
      NetDeviceContainer ueLteDevs = lteHelper->InstallUeDevice (nodes);

      // Assign ip addresses
      if1 = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueLteDevs));


       for(uint32_t u = 0; u < nodes.GetN (); ++u){
      lteHelper->Attach (ueLteDevs.Get(u), enbLteDevs.Get(0));
       }

       for(uint32_t u = 0; u < nodes.GetN (); ++u){
      // setup ip routes
      cmd_oss.str ("");
      cmd_oss << "rule add from " << if1.GetAddress (u, 0) << " table " << 1;
      LinuxStackHelper::RunIp (nodes.Get (u), Seconds (0.1), cmd_oss.str ().c_str ());
      cmd_oss.str ("");
      cmd_oss << "route add default via " << "7.0.0.1 "  << " dev sim" << 0 << " table " << 1;
      LinuxStackHelper::RunIp (nodes.Get (u), Seconds (0.1), cmd_oss.str ().c_str ());
       }

      // LTE-R <-> H2
      // Right link
      //devices2 = pointToPoint.Install (cn.Get (0), pgw);


       for(uint32_t u = 0; u < nodes.GetN (); ++u){
      csmaNetDevice = csmahelper.Install(NodeContainer (cn.Get(u), bridgeLte) );
      devices2.Add(csmaNetDevice.Get(0));
      lteBridgeDevices.Add(csmaNetDevice.Get(1));

      pgwI = u+1;
       }

    
      csmaNetDevice = csmahelper.Install(NodeContainer (pgw, bridgeLte) );
      //devices2.Get (0)->SetAttribute ("ReceiveErrorModel", PointerValue (em1));
      // Assign ip addresses
      devices2.Add(csmaNetDevice.Get(0));
      lteBridgeDevices.Add(csmaNetDevice.Get(1));

    bridge.Install (bridgeLte, lteBridgeDevices);
      
    if2 = address2.Assign (devices2);

      for (int ii=0; ii< if2.GetN(); ii++)
          cout<<ii <<"  " <<if2.GetAddress (ii, 0)<<endl;


       for(uint32_t u = 0; u < nodes.GetN (); ++u){
      // setup ip routes
      cmd_oss.str ("");
      cmd_oss << "rule add from " << if2.GetAddress (u, 0) << " table " << (1);
      LinuxStackHelper::RunIp (cn.Get (u), Seconds (0.1), cmd_oss.str ().c_str ());
      cmd_oss.str ("");

      cout<<"cn to lte if index "<<devices2.Get (0)->GetIfIndex ()<<endl; 

      cmd_oss << "route add 10.2." << 0 << ".0/24 dev sim" << devices2.Get (0)->GetIfIndex ()  << " scope link table " << (1);
      LinuxStackHelper::RunIp (cn.Get (u), Seconds (0.1), cmd_oss.str ().c_str ());
       }

      setPos (pgw, 70, 0, 0);

      address2.NewNetwork ();
    }

  if (!disWifi)
    {

        Ptr<Node> bridgeWifi = CreateObject<Node> ();
      // Left link: H1 <-> WiFi-R
      WifiHelper wifi = WifiHelper::Default ();
    wifi.SetRemoteStationManager ("ns3::AarfWifiManager");
      phy = YansWifiPhyHelper::Default ();
      YansWifiChannelHelper phyChannel = YansWifiChannelHelper::Default ();
      NqosWifiMacHelper mac;
      phy.SetChannel (phyChannel.Create ());
      //mac.SetType ("ns3::AdhocWifiMac");
      //
      int WifiAPI = 0;

      cmd_oss.str ("");

      cmd_oss << "wifi-default-" << WifiAPI;
    Ssid ssid = Ssid (cmd_oss.str ());

  mac.SetType ("ns3::StaWifiMac", "Ssid", SsidValue (ssid),  "ActiveProbing", BooleanValue (false));
      
      wifi.SetStandard (WIFI_PHY_STANDARD_80211a);
       
      
      for(uint32_t u = 0; u < nodes.GetN (); ++u){
       wifiDevice = wifi.Install (phy, mac, nodes.Get (u));
      devices1.Add(wifiDevice);
     // routerI = u+1;
       }


      routerI = nodes.GetN();

          mac.SetType ("ns3::ApWifiMac", "Ssid", SsidValue (ssid));
       wifiDevice = wifi.Install (phy, mac,  routers.Get (0));

      devices1.Add(wifiDevice);
      // devices1 = wifi.Install (phy, mac, NodeContainer (nodes, routers.Get (0)));
      // Assign ip addresses
      if1 = address1.Assign (devices1);
     // address1.NewNetwork ();

       for(uint32_t u = 0; u < if1.GetN (); ++u){
          cout<<u <<" "<<if1.GetAddress(u, 0)<<endl; 
       }

     cout<<"routerI="<<routerI<<endl;


       for(uint32_t u = 0; u < nodes.GetN (); ++u){
      // setup ip routes
      cmd_oss.str ("");
      cmd_oss << "rule add from " << if1.GetAddress (u, 0) << " table " << 2;
      LinuxStackHelper::RunIp (nodes.Get (u), Seconds (0.1), cmd_oss.str ().c_str ());
      cmd_oss.str ("");
      cmd_oss << "route add 10.1." << 0 << ".0/24 dev sim"
              << devices1.Get (u)->GetIfIndex () << " scope link table " << 2;
      LinuxStackHelper::RunIp (nodes.Get (u), Seconds (0.1), cmd_oss.str ().c_str ());
      cmd_oss.str ("");
      cmd_oss << "route add default via " << if1.GetAddress (routerI, 0) << " dev sim" 
              << devices1.Get (u)->GetIfIndex () << " table " << 2;
      LinuxStackHelper::RunIp (nodes.Get (u), Seconds (0.1), cmd_oss.str ().c_str ());
       }


      cmd_oss.str ("");
      cmd_oss << "route add 10.1.0.0/16 via " << if1.GetAddress (routerI, 0) << " dev sim0";
      LinuxStackHelper::RunIp (routers.Get (0), Seconds (0.2), cmd_oss.str ().c_str ());

      // Global default route
      if (disLte)
        {

       for(uint32_t u = 0; u < nodes.GetN (); ++u){
          cmd_oss.str ("");
          cmd_oss << "route add default via " << if1.GetAddress (routerI, 0) << " dev sim" 
                  << devices1.Get (u)->GetIfIndex ();
          LinuxStackHelper::RunIp (nodes.Get (u), Seconds (0.1), cmd_oss.str ().c_str ());
       }
        }

      // Down/Up
#if 0
      cmd_oss.str ("");
      cmd_oss << "link set down dev sim1";
      LinuxStackHelper::RunIp (nodes.Get (0), Seconds (1.0), cmd_oss.str ().c_str ());
      cmd_oss.str ("");
      cmd_oss << "link set up dev sim1";
      LinuxStackHelper::RunIp (nodes.Get (0), Seconds (10.0), cmd_oss.str ().c_str ());
#endif


      
       for(uint32_t u = 0; u < nodes.GetN (); ++u){
      csmaNetDevice = csmahelperWifi.Install(NodeContainer (cn.Get(u), bridgeWifi) );
      devicesWifi.Add(csmaNetDevice.Get(0));
      wifiBridgeDevices.Add(csmaNetDevice.Get(1));

       }

 csmaNetDevice = csmahelperWifi.Install(NodeContainer (routers.Get(0), bridgeWifi) );

      devicesWifi.Add(csmaNetDevice.Get(0));
      wifiBridgeDevices.Add(csmaNetDevice.Get(1));

    bridgeW.Install (bridgeWifi, wifiBridgeDevices);
      
    if2Wifi = address2.Assign (devicesWifi);

      cmd_oss.str ("");
      cmd_oss<<if2Wifi.GetAddress (0, 0);
      char subNet = cmd_oss.str().at(5);


       for(uint32_t u = 0; u < nodes.GetN (); ++u){
      // setup ip routes
      cmd_oss.str ("");
      cmd_oss << "rule add from " << if2Wifi.GetAddress (u, 0) << " table " << (2);
      LinuxStackHelper::RunIp (cn.Get (u), Seconds (0.1), cmd_oss.str ().c_str ());
      cmd_oss.str ("");
      cout<<"interface index cn ="<<devicesWifi.Get(u)->GetIfIndex() <<endl;
        
      //TODO, need to change the 1 here to index dynamically !!
      cmd_oss << "route add 10.2." << subNet << ".0/24 dev sim" << devicesWifi.Get(u)->GetIfIndex() << " scope link table " << (2);
      LinuxStackHelper::RunIp (cn.Get (u), Seconds (0.1), cmd_oss.str ().c_str ());
      cmd_oss.str ("");
      cmd_oss << "route add 10.1.0.0/16 via " << if2Wifi.GetAddress (routerI, 0) << " dev sim" <<  devicesWifi.Get(u)->GetIfIndex() << " table " << (2);
      LinuxStackHelper::RunIp (cn.Get (u), Seconds (0.1), cmd_oss.str ().c_str ());
       }

       if(disLte)
       {
     cmd_oss.str ("");
       cmd_oss<< "route add default via ";
      cmd_oss << if2Wifi.GetAddress(routerI, 0);
      cmd_oss<< " dev sim0";

  for(uint32_t u = 0; u < cn.GetN (); ++u){
      LinuxStackHelper::RunIp (cn.Get (u), Seconds (0.1), cmd_oss.str());

       }

       }

      cmd_oss.str ("");
      cmd_oss << "route add 10.2.0.0/16 via " << if2Wifi.GetAddress (routerI, 0) << " dev sim1";
      LinuxStackHelper::RunIp (routers.Get (0), Seconds (0.2), cmd_oss.str ().c_str ());
      //setPos (routers.Get (0), 70, 30, 0);
      setPos (routers.Get (0), -20, 10, 0);
      
      address2.NewNetwork ();
    }

  if(!disLte)
  {

       for(uint32_t u = 0; u < nodes.GetN (); ++u){
  // default route
  LinuxStackHelper::RunIp (nodes.Get (u), Seconds (0.1), "route add default via 7.0.0.1 dev sim0");

       }

       cmd_oss.str ("");
       cmd_oss<< "route add default via ";
      cmd_oss << if2.GetAddress(pgwI, 0);
      cmd_oss<< " dev sim0";

       for(uint32_t u = 0; u < cn.GetN (); ++u){
  LinuxStackHelper::RunIp (cn.Get (u), Seconds (0.1), cmd_oss.str());

       }
  }

       for(uint32_t u = 0; u < nodes.GetN (); ++u){

  LinuxStackHelper::RunIp (nodes.Get (u), Seconds (0.1), "rule show");
  LinuxStackHelper::RunIp (nodes.Get (u), Seconds (4.5), "route show table all");
  LinuxStackHelper::RunIp (nodes.Get (u), Seconds (4.5), "route show table all");

  
  LinuxStackHelper::RunIp (cn.Get (u), Seconds (0.1), "rule show");
  LinuxStackHelper::RunIp (cn.Get (u), Seconds (4.5), "route show table all");
       }

       LinuxStackHelper::RunIp (routers.Get (0), Seconds (0.15), "rule show");
  LinuxStackHelper::RunIp (routers.Get (0), Seconds (4.4), "route show table all");


  LinuxStackHelper::RunIp (nodes.Get (0), Seconds (5), "addr");
  LinuxStackHelper::RunIp (nodes.Get (0), Seconds (10.1), "link set dev sim1 multipath backup");
  //LinuxStackHelper::RunIp (nodes.Get (0), Seconds (10.1), "link set dev sim1 down");


/*

  LinuxStackHelper::RunIp (cn.Get (0), Seconds (0.5), "link set dev sim0 multipath on");

       for(uint32_t u = 0; u < nodes.GetN (); ++u){
  LinuxStackHelper::RunIp (nodes.Get (u), Seconds (0.5), "link set dev sim0 multipath on");
       }
*/ 
  // debug
  stack.SysctlSet (nodes, ".net.mptcp.mptcp_debug", "1");

#if 1
  LinuxStackHelper::SysctlGet (nodes.Get (0), NanoSeconds (0),
                               ".net.ipv4.tcp_available_congestion_control", &PrintTcpFlags);
  LinuxStackHelper::SysctlGet (nodes.Get (0), NanoSeconds (0),
                               ".net.ipv4.tcp_rmem", &PrintTcpFlags);
  LinuxStackHelper::SysctlGet (nodes.Get (0), NanoSeconds (0),
                               ".net.ipv4.tcp_wmem", &PrintTcpFlags);
  LinuxStackHelper::SysctlGet (nodes.Get (0), NanoSeconds (0),
                               ".net.core.rmem_max", &PrintTcpFlags);
  LinuxStackHelper::SysctlGet (nodes.Get (0), NanoSeconds (0),
                               ".net.core.wmem_max", &PrintTcpFlags);
  LinuxStackHelper::SysctlGet (nodes.Get (0), Seconds (1),
                               ".net.ipv4.tcp_available_congestion_control", &PrintTcpFlags);
  LinuxStackHelper::SysctlGet (nodes.Get (0), Seconds (1),
                               ".net.ipv4.tcp_rmem", &PrintTcpFlags);
  LinuxStackHelper::SysctlGet (nodes.Get (0), Seconds (1),
                               ".net.ipv4.tcp_wmem", &PrintTcpFlags);
  LinuxStackHelper::SysctlGet (nodes.Get (0), Seconds (1),
                               ".net.core.rmem_max", &PrintTcpFlags);
  LinuxStackHelper::SysctlGet (nodes.Get (0), Seconds (1),
                               ".net.core.wmem_max", &PrintTcpFlags);
#endif
#if 1
  if (bufSize.length () != 0)
    {
      stack.SysctlSet (nodes, ".net.ipv4.tcp_rmem",
                       bufSize + " " + bufSize + " " + bufSize);
      //                       "4096 87380 " +bufSize);
      stack.SysctlSet (nodes, ".net.ipv4.tcp_wmem",
                       bufSize + " " + bufSize + " " + bufSize);
      stack.SysctlSet (nodes, ".net.core.rmem_max",
                       bufSize);
      stack.SysctlSet (nodes, ".net.core.wmem_max",
                       bufSize);
    }
#endif


 // DceApplicationHelper dce;
 // ApplicationContainer apps;

   // Launch iperf client on node 0
  ApplicationContainer apps;



       for(uint32_t u = 0; u < nodes.GetN (); ++u){
  cmd_oss.str ("");

if(!disLte)
{
      cmd_oss << "7.0.0."<<u+2; 
}
else
{
      cmd_oss << "10.1.0."<<u+1; 
}

  OnOffHelper onoff = OnOffHelper ("ns3::LinuxTcpSocketFactory",
                                   InetSocketAddress (cmd_oss.str().c_str(), 9));
  onoff.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
  onoff.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
  onoff.SetAttribute ("PacketSize", StringValue ("1024"));
  onoff.SetAttribute ("DataRate", StringValue ("10Mbps"));
  apps = onoff.Install (cn.Get (u));
  apps.Start (Seconds (5.0));
       }


  PacketSinkHelper sink = PacketSinkHelper ("ns3::LinuxTcpSocketFactory",
                                            InetSocketAddress (Ipv4Address::GetAny (), 9));



for(uint32_t u = 0; u < nodes.GetN (); ++u){

    apps = sink.Install (nodes.Get (u));
    apps.Start (Seconds (4));
}


////start to schedule the ip link check

DceApplicationHelper dce;

  ApplicationContainer Dapps;
 dce.SetBinary ("ip");

  dce.ResetArguments ();
  dce.ResetEnvironment ();
  dce.AddArgument ("-s");
  dce.AddArgument ("link");
  dce.AddArgument ("ls");
  dce.AddArgument ("sim0");


 Dapps = dce.Install (nodes.Get (0));
 Dapps.Start (Seconds (10.0));

 dce.ResetArguments ();
  dce.ResetEnvironment ();
  dce.AddArgument ("-s");
  dce.AddArgument ("link");
  dce.AddArgument ("ls");
  dce.AddArgument ("sim1");


 Dapps = dce.Install (nodes.Get (0));
 Dapps.Start (Seconds (11.0));

  uint16_t pid;
  pid = dce.GetPid(GetPointer(Dapps.Get(0)));
  cout<<" ##pid = "<<pid<<endl;


for(int ii=0; ii< nodes.GetN(); ii++)
{
    char aa[10];
    sprintf( aa, "%d", ii);
    //std::string sink = "/NodeList/"+std::string(aa)+"/ApplicationList/*/$ns3::PacketSink/Rx";
    std::string sink = "/NodeList/"+std::string(aa)+"/ApplicationList/*/$ns3::PacketSink/Rx";
    //std::string sink = "/NodeList/[25-29]/ApplicationList/*/$ns3::PacketSink/Rx";
    Config::Connect (sink, MakeCallback(&ReceivedPacket) );
}

Simulator::Schedule (Seconds (INTERVAL/2.0f), &Throughput);


  //pointToPoint.EnablePcapAll ("mptcp-lte-wifi", false);
  phy.EnablePcapAll ("wifi", false);
    csmahelper.EnablePcapAll ("csma-bridge", false);
    csmahelperWifi.EnablePcapAll ("bridge-wifi", false);
  //lteHelper->EnablePcapAll ("mptcp-lte-wifi", false);
  lteHelper->EnableTraces ();

  // Output config store to txt format
  Config::SetDefault ("ns3::ConfigStore::Filename", StringValue ("output-attributes.txt"));
  Config::SetDefault ("ns3::ConfigStore::FileFormat", StringValue ("RawText"));
  Config::SetDefault ("ns3::ConfigStore::Mode", StringValue ("Save"));
  ConfigStore outputConfig2;
  outputConfig2.ConfigureDefaults ();
  outputConfig2.ConfigureAttributes ();

  Simulator::Stop (Seconds (stopTime));
  Simulator::Run ();

  //flowMonitor->SerializeToXmlFile("allFlow.xml", true, true);
  //sleep(1);

  Simulator::Destroy ();

  return 0;
}
