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
#include "ns3/topology-read-module.h"
#include <sys/resource.h>

using namespace ns3;
NS_LOG_COMPONENT_DEFINE ("DceQuaggaBgpdCaida");

// Parameters
uint32_t nNodes = 2;
uint32_t stopTime = 6000;

static void
SetRlimit ()
{
  int ret;
  struct rlimit limit;
  limit.rlim_cur = 1000000;
  limit.rlim_max = 1000000;

  ret = setrlimit (RLIMIT_NOFILE, &limit);
  if (ret == -1)
    {
      perror ("setrlimit");
    }
  return;
}

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

static void AddAddress (Ptr<Node> node, Time at, int ifindex, const char *address)
{
  std::ostringstream oss;
  oss << "-f inet addr add " << address << " dev sim" << ifindex;
  RunIp (node, at, oss.str ());
}

int main (int argc, char *argv[])
{
  //
  //  Step 0
  //  Node Basic Configuration
  //

  CommandLine cmd;
  cmd.AddValue ("stopTime", "Time to stop(seconds)", stopTime);
  cmd.Parse (argc,argv);

  LogComponentEnable ("DceQuaggaBgpdCaida", LOG_LEVEL_ALL);
  SetRlimit ();

  //
  //  Step 1
  //  Node Basic Configuration
  //
  Ptr<TopologyReader> inFile = 0;
  TopologyReaderHelper topoHelp;
  NodeContainer nodes;

  std::string format ("Caida");
  std::string input ("./myscripts/ns-3-dce-quagga/example/asrel-as2500.txt");
  input = "./myscripts/ns-3-dce-quagga/example/caida-5node.txt";

  topoHelp.SetFileName (input);
  topoHelp.SetFileType (format);
  inFile = topoHelp.GetTopologyReader ();

  if (inFile != 0)
    {
      nodes = inFile->Read ();
    }

  if (nodes.GetN () == 0)
    {
      NS_LOG_ERROR ("Problems reading node information the topology file. Failing.");
      return -1;
    }
  if (inFile->LinksSize () == 0)
    {
      NS_LOG_ERROR ("Problems reading the topology file. Failing.");
      return -1;
    }
  NS_LOG_INFO ("Caida topology created with " << nodes.GetN () << " nodes and " <<
               inFile->LinksSize () << " links (from " << input << ")");

  // Address conf In virtual topology
  PointToPointHelper p2p;
  p2p.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("2ms"));

  int totlinks = inFile->LinksSize ();
  NS_LOG_INFO ("creating node containers");
  NodeContainer nc[totlinks];
  TopologyReader::ConstLinksIterator iter;
  int i = 0;
  for ( iter = inFile->LinksBegin (); iter != inFile->LinksEnd (); iter++, i++ )
    {
      nc[i] = NodeContainer (iter->GetFromNode (), iter->GetToNode ());
    }

  DceManagerHelper processManager;
  processManager.SetNetworkStack ("ns3::LinuxSocketFdFactory",
                                  "Library", StringValue ("liblinux.so"));
  processManager.Install (nodes);
  LinuxStackHelper stack;
  stack.Install (nodes);
  Ipv4AddressHelper address;
  address.SetBase ("10.0.0.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces;

  QuaggaHelper quagga;
  quagga.EnableBgp (nodes);

  NS_LOG_INFO ("creating net device containers");
  NetDeviceContainer ndc[totlinks];
  for (int i = 0; i < totlinks; i++)
    {
      ndc[i] = p2p.Install (nc[i]);
      // assgin ip addresses
      interfaces = address.Assign (ndc[i]);

      std::string link_base;
      std::stringstream str;
      str << "10." << (i / 256) << "." << (i % 256) << "";
      link_base = str.str ();
      // IP address configuration
      AddAddress (nc[i].Get (0), Seconds (0.1), ndc[i].Get (0)->GetIfIndex (), (link_base + ".1/24").c_str ());
      RunIp (nc[i].Get (0), Seconds (0.11), "link set lo up");

      AddAddress (nc[i].Get (1), Seconds (0.1), ndc[i].Get (1)->GetIfIndex (), (link_base + ".2/24").c_str ());
      RunIp (nc[i].Get (1), Seconds (0.11), "link set lo up");

      quagga.BgpAddNeighbor (nc[i].Get (0), link_base + ".2", quagga.GetAsn (nc[i].Get (1)));
      quagga.BgpAddNeighbor (nc[i].Get (1), link_base + ".1", quagga.GetAsn (nc[i].Get (0)));

      // peer link
      iter = inFile->LinksBegin ();
      std::advance (iter, i);
      if (iter->GetAttribute ("Relationship") == "0")
        {
          NS_LOG_DEBUG ("linkattr " << iter->GetAttribute ("Relationship"));
          quagga.BgpAddPeerLink (nc[i].Get (0), link_base + ".2");
          quagga.BgpAddPeerLink (nc[i].Get (1), link_base + ".1");
        }
      quagga.Install (nc[i]);
    }


  //  p2p.EnablePcapAll ("dce-quagga-bgpd-caida");

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
