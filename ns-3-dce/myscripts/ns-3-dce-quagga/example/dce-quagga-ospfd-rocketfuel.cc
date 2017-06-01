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

#include "ns3/v4ping.h"

#include <sys/resource.h>
#undef NS3_MPI
#ifdef NS3_MPI
#include <mpi.h>
#include "ns3/mpi-interface.h"
#endif
using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("quagga-ospfd-rocketfuel");

// Parameters
uint32_t stopTime = 3600;

#ifdef UNUSE
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
#endif

int
main (int argc, char *argv[])
{
  //  LogComponentEnable ("quagga-ospfd-rocketfuel", LOG_LEVEL_INFO);
  std::string topoFile="myscripts/ns-3-dce-quagga/example/3967.weights.intra";
#ifdef NS3_MPI
  // Distributed simulation setup
  MpiInterface::Enable (&argc, &argv);
  GlobalValue::Bind ("SimulatorImplementationType",
                     StringValue ("ns3::DistributedSimulatorImpl"));

  uint32_t systemId = MpiInterface::GetSystemId ();
  uint32_t systemCount = MpiInterface::GetSize ();
#endif

  CommandLine cmd;
  cmd.AddValue ("stopTime", "Time to stop(seconds)", stopTime);
  cmd.AddValue ("topoFile", "topology file of rocketfuel dataset", topoFile);
  cmd.Parse (argc,argv);

  //
  // Step o
  // Read Topology information
  // ------------------------------------------------------------
  // -- Read topology data.
  // --------------------------------------------

  // Pick a topology reader based in the requested format.

  Ptr<TopologyReader> inFile = 0;
  TopologyReaderHelper topoHelp;

  NodeContainer nodes;

  std::string format ("Rocketfuel");
  std::string input (topoFile);

  topoHelp.SetFileName (input);
  topoHelp.SetFileType (format);
  inFile = topoHelp.GetTopologyReader ();

  if (inFile != 0)
    {
      nodes = inFile->Read ();
    }

  if (nodes.GetN () == 0)
    {
      NS_ASSERT ("Problems reading node information the topology file. Failing.");
      return -1;
    }
  if (inFile->LinksSize () == 0)
    {
      NS_LOG_ERROR ("Problems reading the topology file. Failing.");
      return -1;
    }
  NS_LOG_INFO ("Rocketfuel topology created with " << nodes.GetN () << " nodes and " <<
               inFile->LinksSize () << " links (from " << input << ")");

  // Tricky things XXX
  // Change systemId for MPI by hand...
#ifdef NS3_MPI
  for (uint32_t i = 0; i < nodes.GetN (); i++)
    {
      nodes.Get (i)->SetSystemId (i % systemCount);
    }
#endif

  //
  //  Step 1
  //  Node Basic Configuration
  //

  // Internet stack install
  InternetStackHelper stack;    // IPv4 is required for GlobalRouteMan
  Ipv4DceRoutingHelper ipv4RoutingHelper;
  stack.SetRoutingHelper (ipv4RoutingHelper);
  stack.Install (nodes);

  //
  // Step 2
  // Address Configuration
  //
  //
  Ipv4AddressHelper ipv4AddrHelper;

  NS_LOG_INFO ("creating ip4 addresses");
  Ipv4AddressHelper address;
  address.SetBase ("10.0.0.0", "255.255.255.252");

  int totlinks = inFile->LinksSize ();
  NS_LOG_INFO ("creating node containers");
  NodeContainer nc[totlinks];
  TopologyReader::ConstLinksIterator iter;
  int i = 0;
  for ( iter = inFile->LinksBegin (); iter != inFile->LinksEnd (); iter++, i++ )
    {
      nc[i] = NodeContainer (iter->GetFromNode (), iter->GetToNode ());
    }


  NS_LOG_INFO ("creating net device containers");
  NetDeviceContainer ndc[totlinks];
  PointToPointHelper p2p;
  for (int i = 0; i < totlinks; i++)
    {
      // p2p.SetChannelAttribute ("Delay", TimeValue(MilliSeconds(weight[i])));
      p2p.SetChannelAttribute ("Delay", StringValue ("2ms"));
      p2p.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
      ndc[i] = p2p.Install (nc[i]);
    }
//  p2p.EnablePcapAll ("quagga-rocketfuel");

  NS_LOG_INFO ("creating ipv4 interfaces");
  Ipv4InterfaceContainer ipic[totlinks];
  for (int i = 0; i < totlinks; i++)
    {
      ipic[i] = address.Assign (ndc[i]);
      address.NewNetwork ();
    }


  //
  //  Application configuration for Nodes
  //

  NS_LOG_INFO ("creating quagga process");
  //run quagga programs in every node
  DceManagerHelper processManager;
  QuaggaHelper quagga;

  //
  // Step 3
  // Traffic configuration
  //
#ifdef NS3_MPI
  if (systemId == 0)
#endif
  {
    Ptr<V4Ping> app = CreateObject<V4Ping> ();
    //Ptr<Ipv4> ipv4Server = nodes.Get (25)->GetObject<Ipv4> ();
    Ptr<Ipv4> ipv4Server = nodes.Get (nodes.GetN () - 1)->GetObject<Ipv4> ();
    app->SetAttribute ("Remote", Ipv4AddressValue (ipv4Server->GetAddress (1, 0).GetLocal ()));
    app->SetAttribute ("Verbose", BooleanValue (true));
    nodes.Get (0)->AddApplication (app);
    app->SetStartTime (Seconds (20.0));
    app->SetStopTime (Seconds (stopTime));
  }

  for (uint32_t i = 0; i < nodes.GetN (); i++)
    {
#ifdef NS3_MPI
      if (i % systemCount == systemId % systemCount)
#endif
      {
        //     std::cout << "[" << systemId << "] start quagga Node " << i << std::endl;
        processManager.Install (nodes.Get (i));
        quagga.EnableOspf (nodes.Get (i), "10.0.0.0/8"); // FIXME
        quagga.EnableOspfDebug (nodes.Get (i));
        quagga.Install (nodes.Get (i));
      }
    }

  //
  // Step 9
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
