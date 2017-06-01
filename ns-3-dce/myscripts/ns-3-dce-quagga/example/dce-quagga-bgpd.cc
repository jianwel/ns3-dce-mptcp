/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2012 Hajime Tazaki, NICT
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
 * Author: Hajime Tazaki <tazaki@nict.go.jp>
 */

#include "ns3/network-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/dce-module.h"
#include "ns3/quagga-helper.h"
#include "ns3/point-to-point-helper.h"

using namespace ns3;
NS_LOG_COMPONENT_DEFINE ("DceQuaggaBgpd");

// Parameters
uint32_t nNodes = 2;
uint32_t stopTime = 6000;
std::string netStack = "linux";


static void RunIp (Ptr<Node> node, Time at, std::string str)
{
  DceApplicationHelper process;
  ApplicationContainer apps;
  process.SetBinary ("ip");
  process.SetStackSize (1 << 16);
  process.ResetArguments ();
  process.ParseArguments (str.c_str ());
  apps = process.Install (node);
  apps.Start (at);
}

static void AddAddress (Ptr<Node> node, Time at, const char *name, const char *address)
{
  std::ostringstream oss;
  oss << "-f inet addr add " << address << " dev " << name;
  RunIp (node, at, oss.str ());
}

int main (int argc, char *argv[])
{
  //
  //  Step 0
  //  Node Basic Configuration
  //

  CommandLine cmd;
  cmd.AddValue ("nNodes", "Number of Router nodes", nNodes);
  cmd.AddValue ("stopTime", "Time to stop(seconds)", stopTime);
  cmd.AddValue ("netStack", "What network stack", netStack);
  cmd.Parse (argc,argv);

  //
  //  Step 1
  //  Node Basic Configuration
  //
  NodeContainer nodes;
  nodes.Create (nNodes);

  // Address conf In virtual topology
  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

  NetDeviceContainer devices;
  devices = pointToPoint.Install (nodes);


  DceManagerHelper processManager;
  if (netStack == "ns3")
    {
      Ipv4AddressHelper ipv4AddrHelper;
      // Internet stack install
      InternetStackHelper stack;    // IPv4 is required for GlobalRouteMan
      Ipv4DceRoutingHelper ipv4RoutingHelper;
      stack.SetRoutingHelper (ipv4RoutingHelper);
      stack.Install (nodes);

      ipv4AddrHelper.SetBase ("10.0.0.0", "255.255.255.0");
      Ipv4InterfaceContainer interfaces = ipv4AddrHelper.Assign (devices);
      Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

      processManager.SetNetworkStack ("ns3::Ns3SocketFdFactory");
      processManager.Install (nodes);

      QuaggaHelper quagga;
      quagga.EnableBgp (nodes);
      quagga.BgpAddNeighbor (nodes.Get (0), "10.0.0.2", quagga.GetAsn (nodes.Get (1)));
      quagga.BgpAddNeighbor (nodes.Get (1), "10.0.0.1", quagga.GetAsn (nodes.Get (0)));
      quagga.EnableZebraDebug (nodes);
      quagga.Install (nodes);

    }
  else if (netStack == "linux")
    {
      //      processManager.SetLoader ("ns3::DlmLoaderFactory");
      processManager.SetTaskManagerAttribute ("FiberManagerType",
                                              EnumValue (0));
      processManager.SetNetworkStack ("ns3::LinuxSocketFdFactory",
                                      "Library", StringValue ("liblinux.so"));
      processManager.Install (nodes);

      // IP address configuration
      AddAddress (nodes.Get (0), Seconds (0.1), "sim0", "10.0.0.1/24");
      RunIp (nodes.Get (0), Seconds (0.11), "link set lo up");
      RunIp (nodes.Get (0), Seconds (0.11), "link set sim0 up");

      AddAddress (nodes.Get (1), Seconds (0.1), "sim0", "10.0.0.2/24");
      RunIp (nodes.Get (1), Seconds (0.11), "link set lo up");
      RunIp (nodes.Get (1), Seconds (0.11), "link set sim0 up");

      QuaggaHelper quagga;
      quagga.EnableBgp (nodes);
      quagga.BgpAddNeighbor (nodes.Get (0), "10.0.0.2", quagga.GetAsn (nodes.Get (1)));
      quagga.BgpAddNeighbor (nodes.Get (1), "10.0.0.1", quagga.GetAsn (nodes.Get (0)));
      quagga.Install (nodes);
    }

  pointToPoint.EnablePcapAll ("dce-quagga-bgpd");

  //
  // Now It's ready to GO!
  //
  if (stopTime != 0)
    {
      Simulator::Stop (Seconds (stopTime));
    }
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
