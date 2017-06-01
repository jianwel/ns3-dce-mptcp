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
#include "ns3/csma-helper.h"
#include "ns3/v4ping.h"
#include "ns3/ping6.h"
#include "ns3/ethernet-header.h"
#include "ns3/icmpv4.h"

#define OUTPUT(x)                                                       \
  {                                                                     \
    std::ostringstream oss;                                             \
    oss << "file=" << __FILE__ << " line=" << __LINE__ << " "           \
        << x << std::endl;                                              \
    std::string s = oss.str ();                                         \
    std::cerr << s.c_str ();                                            \
  }


static std::string g_testError;

extern "C" void dce_quagga_test_store_test_error (const char *s)
{
  g_testError = s;
}

using namespace ns3;
namespace ns3 {

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
  oss << "-f inet6 addr add " << address << " dev " << name;
  RunIp (node, at, oss.str ());
}

class DceQuaggaTestCase : public TestCase
{
public:
  DceQuaggaTestCase (std::string testname, Time maxDuration, bool useK, bool skip);
  void CsmaRxCallback (std::string context, Ptr<const Packet> packet);
private:
  virtual void DoRun (void);
  static void Finished (int *pstatus, uint16_t pid, int status);

  std::string m_testname;
  Time m_maxDuration;
  bool m_useKernel;
  bool m_pingStatus;
  bool m_debug;
  bool m_skip;
};

void
DceQuaggaTestCase::CsmaRxCallback (std::string context, Ptr<const Packet> originalPacket)
{
  if (m_pingStatus)
    {
      return;
    }
  uint16_t protocol;
  Ptr<Packet> packet = originalPacket->Copy ();
  EthernetHeader header (false);
  packet->RemoveHeader (header);
  protocol = header.GetLengthType ();
  Ipv4Header v4hdr;
  Icmpv4Header icmphdr;
  Ipv6Header v6hdr;
  Icmpv6Header icmp6hdr;
  switch (protocol)
    {
    case 0x0800:   //IPv4
      packet->RemoveHeader (v4hdr);
      packet->RemoveHeader (icmphdr);
      if (icmphdr.GetType () == Icmpv4Header::ECHO_REPLY)
        {
          m_pingStatus = true;
        }
      break;
    case 0x86DD:   //IPv6
      packet->RemoveHeader (v6hdr);
      packet->RemoveHeader (icmp6hdr);
      if (icmp6hdr.GetType () == Icmpv6Header::ICMPV6_ECHO_REPLY)
        {
          m_pingStatus = true;
        }
      break;
    default:
      break;
    }

  //  std::cout << context << " " << packet << " protocol " << protocol << std::endl;
}

DceQuaggaTestCase::DceQuaggaTestCase (std::string testname, Time maxDuration, bool useK, bool skip)
  : TestCase (skip ? "(SKIP) " + testname + (useK ? " (kernel)" : " (ns3)") : testname
              + (useK ? " (kernel)" : " (ns3)")),
    m_testname (testname),
    m_maxDuration (maxDuration),
    m_useKernel (useK),
    m_pingStatus (false),
    m_debug (false),
    m_skip (skip)
{
}
void
DceQuaggaTestCase::Finished (int *pstatus, uint16_t pid, int status)
{
  *pstatus = status;
}
void
DceQuaggaTestCase::DoRun (void)
{
  if (m_skip)
    {
      return;
    }

  //
  //  Step 1
  //  Node Basic Configuration
  //
  std::string routerPort;

  NodeContainer nodes;
  nodes.Create (2);

  CsmaHelper csma;

  NetDeviceContainer devices, dev1, dev2;
  devices = csma.Install (nodes);
  dev1 = csma.Install (nodes.Get (0));
  dev2 = csma.Install (nodes.Get (1));
  DceManagerHelper processManager;

  //  processManager.SetLoader ("ns3::DlmLoaderFactory");
  processManager.SetTaskManagerAttribute ("FiberManagerType",
                                          EnumValue (0));
  //
  // Step 2
  // Address Configuration
  //
  //
  if (m_useKernel == false)
    {
      Ipv4AddressHelper ipv4AddrHelper;
      Ipv6AddressHelper ipv6AddrHelper;
      // Internet stack install
      InternetStackHelper stack;    // IPv4 is required for GlobalRouteMan
      Ipv4DceRoutingHelper ipv4RoutingHelper;
      stack.SetRoutingHelper (ipv4RoutingHelper);
      stack.Install (nodes);

      ipv4AddrHelper.SetBase ("10.0.0.0", "255.255.255.0");
      ipv4AddrHelper.Assign (devices);
      ipv6AddrHelper.SetBase ("2001:db8:0:1::", Ipv6Prefix (64));
      ipv6AddrHelper.Assign (devices);

      ipv4AddrHelper.SetBase ("11.0.0.0", "255.255.255.0");
      ipv4AddrHelper.Assign (dev1);
      ipv6AddrHelper.NewNetwork ();
      ipv6AddrHelper.Assign (dev1);

      ipv4AddrHelper.SetBase ("12.0.0.0", "255.255.255.0");
      ipv4AddrHelper.Assign (dev2);
      ipv6AddrHelper.NewNetwork ();
      ipv6AddrHelper.Assign (dev2);

      processManager.SetNetworkStack ("ns3::Ns3SocketFdFactory");
      processManager.Install (nodes);

      routerPort = "ns3-device0";
      if (m_debug)
        {
          Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> ("routes-" + m_testname + ".log", std::ios::out);
          ipv4RoutingHelper.PrintRoutingTableAllEvery (Seconds (10), routingStream);
        }
    }
  else if (m_useKernel == true)
    {
      processManager.SetNetworkStack ("ns3::LinuxSocketFdFactory",
                                      "Library", StringValue ("liblinux.so"));
      processManager.Install (nodes);

      // IP address configuration
      AddAddress (nodes.Get (0), Seconds (0.1), "sim0", "2001:db8:0:1::1/64");
      AddAddress (nodes.Get (0), Seconds (0.1), "sim1", "2001:db8:0:2::1/64");
      RunIp (nodes.Get (0), Seconds (0.11), "-f inet addr add 10.0.0.1/24 dev sim0");
      RunIp (nodes.Get (0), Seconds (0.11), "-f inet addr add 11.0.0.1/24 dev sim1");
      RunIp (nodes.Get (0), Seconds (0.11), "link set lo up");
      RunIp (nodes.Get (0), Seconds (0.11), "link set sim0 up");
      RunIp (nodes.Get (0), Seconds (0.11), "link set sim1 up");

      AddAddress (nodes.Get (1), Seconds (0.1), "sim0", "2001:db8:0:1::2/64");
      AddAddress (nodes.Get (1), Seconds (0.1), "sim1", "2001:db8:0:3::2/64");
      RunIp (nodes.Get (1), Seconds (0.11), "-f inet addr add 10.0.0.2/24 dev sim0");
      RunIp (nodes.Get (1), Seconds (0.11), "-f inet addr add 12.0.0.1/24 dev sim1");
      RunIp (nodes.Get (1), Seconds (0.11), "link set lo up");
      RunIp (nodes.Get (1), Seconds (0.11), "link set sim0 up");
      RunIp (nodes.Get (1), Seconds (0.11), "link set sim1 up");
      if (m_debug)
        {
          RunIp (nodes.Get (0), Seconds (0.2), "link show");
          RunIp (nodes.Get (0), Seconds (60.3), "route show table all");
          RunIp (nodes.Get (1), Seconds (60.3), "route show table all");
          RunIp (nodes.Get (0), Seconds (0.4), "addr list");
        }

      routerPort = "sim0";
    }
  QuaggaHelper quagga;
  if (m_testname == "ripd")
    {
      quagga.EnableRip (nodes, routerPort.c_str ());
      if (m_debug)
        {
          quagga.EnableRipDebug (nodes);
        }
    }
  else if (m_testname == "ripngd")
    {
      quagga.EnableRipng (nodes, routerPort.c_str ());
      if (m_debug)
        {
          quagga.EnableRipngDebug (nodes);
        }
    }
  else if (m_testname == "ospfd")
    {
      quagga.EnableOspf (nodes, "10.0.0.0/24");
      if (m_debug)
        {
          quagga.EnableOspfDebug (nodes);
        }
    }
  else if (m_testname == "ospf6d")
    {
      quagga.EnableOspf6 (nodes, routerPort.c_str ());
      if (m_debug)
        {
          quagga.EnableOspf6Debug (nodes);
        }
    }
  else if (m_testname == "bgpd")
    {
      quagga.EnableBgp (nodes);
      quagga.BgpAddNeighbor (nodes.Get (0), "10.0.0.2", quagga.GetAsn (nodes.Get (1)));
      quagga.BgpAddNeighbor (nodes.Get (1), "10.0.0.1", quagga.GetAsn (nodes.Get (0)));
    }
  else if (m_testname == "bgpd_v6")
    {
      quagga.EnableBgp (nodes);
      quagga.BgpAddNeighbor (nodes.Get (0), "2001:db8:0:1::2", quagga.GetAsn (nodes.Get (1)));
      quagga.BgpAddNeighbor (nodes.Get (1), "2001:db8:0:1::1", quagga.GetAsn (nodes.Get (0)));
    }
  else if (m_testname == "radvd")
    {
      quagga.EnableRadvd (nodes.Get (0), routerPort.c_str (), "2001:db8:0:1::/64");
      if (m_debug)
        {
          quagga.EnableZebraDebug (nodes.Get (0));
        }
    }

  quagga.Install (nodes);

  if (m_debug)
    {
      csma.EnablePcapAll ("dce-quagga-test-" + m_testname + "-" + (m_useKernel ? "kern" : "ns3"));
    }

  //
  // Step 3
  // Set up ping application
  //
  DceApplicationHelper dce;
  ApplicationContainer apps;
  if (m_testname == "ripd"
      || m_testname == "ospfd"
      || m_testname == "bgpd")
    {
      if (m_useKernel)
        {
          dce.SetBinary ("ping");
          dce.SetStackSize (1 << 20);
          dce.ResetArguments ();
          dce.ResetEnvironment ();
          dce.AddArgument ("11.0.0.1");

          apps = dce.Install (nodes.Get (1));
          apps.Start (Seconds (60.0));
        }
      else
        {
          Ptr<V4Ping> app = CreateObject<V4Ping> ();
          app->SetAttribute ("Remote", Ipv4AddressValue ("11.0.0.1"));
          if (m_debug)
            {
              app->SetAttribute ("Verbose", BooleanValue (true));
            }
          nodes.Get (1)->AddApplication (app);
          app->SetStartTime (Seconds (60.0));
          app->SetStopTime (m_maxDuration);
        }
    }
  else
    {
      if (m_useKernel)
        {
          dce.SetBinary ("ping6");
          dce.SetStackSize (1 << 20);
          dce.ResetArguments ();
          dce.ResetEnvironment ();
          dce.AddArgument ("2001:db8:0:2::1");

          apps = dce.Install (nodes.Get (1));
          apps.Start (Seconds (60.0));
        }
      else
        {
          Ptr<Ping6> app = CreateObject<Ping6> ();
          app->SetAttribute ("RemoteIpv6", Ipv6AddressValue ("2001:db8:0:2::1"));
          app->SetIfIndex (0);
          nodes.Get (1)->AddApplication (app);
          app->SetStartTime (Seconds (60.0));
          app->SetStopTime (m_maxDuration);
        }
    }

  Config::Connect ("/NodeList/1/DeviceList/0/$ns3::CsmaNetDevice/MacRx",
                   MakeCallback (&DceQuaggaTestCase::CsmaRxCallback, this));
  //
  // Step 4
  // Now It's ready to GO!
  //
  if (m_maxDuration.IsStrictlyPositive ())
    {
      Simulator::Stop (m_maxDuration);
    }
  Simulator::Run ();
  Simulator::Destroy ();


  //
  // Step 5
  // Vetify the test
  //
  NS_TEST_ASSERT_MSG_EQ (m_pingStatus, true, "Quagga test " << m_testname  << " with " <<
                         (m_useKernel ? "kernel" : "ns3") << " did not return successfully: " << g_testError);
  if (m_debug)
    {
      OUTPUT ("Quagga test " << m_testname << " with " <<
              (m_useKernel ? "kernel" : "ns3")
              << " stack done. status = " << m_pingStatus);

      ::system (("/bin/mv -f files-0 files-0-" + m_testname + "-" + (m_useKernel ? "kernel" : "ns3")).c_str ());
      ::system (("/bin/mv -f files-1 files-1-" + m_testname + "-" + (m_useKernel ? "kernel" : "ns3")).c_str ());
    }
  else
    {
      ::system ("/bin/rm -rf files-*/usr/local/etc/*.pid");
      ::system ("find files-* -name 'zserv.*' | xargs rm -f");
    }
}

static class DceQuaggaTestSuite : public TestSuite
{
public:
  DceQuaggaTestSuite ();
private:
} g_processTests;
//


DceQuaggaTestSuite::DceQuaggaTestSuite ()
  : TestSuite ("dce-quagga", UNIT)
{
  typedef struct
  {
    const char *name;
    int duration;
    bool useKernel;
  } testPair;

  const testPair tests[] = {
#ifdef FIXME
    { "radvd", 120, false},
    { "ripd", 120, false},
    { "ripngd", 120, false},
    { "ospf6d", 120, false},
    { "bgpd_v6", 120, false},
#endif
    { "ospfd", 120, false},
    { "bgpd", 120, false},
    { "radvd", 120, true},
    { "ripd", 120, true},
    { "ripngd", 120, true},
    { "ospfd", 120, true},
    { "ospf6d", 120, true},
    { "bgpd", 120, true},
    { "bgpd_v6", 120, true},
  };
 
  ::system ("/bin/rm -rf files-*/usr/local/etc/*.pid");
  ::system ("find files-* -name 'zserv.*' | xargs rm -f");
  TypeId tid;
  bool kern = TypeId::LookupByNameFailSafe ("ns3::LinuxSocketFdFactory", &tid);
  // for the moment: not supported quagga for freebsd
  std::string filePath = SearchExecFile ("DCE_PATH", "liblinux.so", 0);
  for (unsigned int i = 0; i < sizeof(tests) / sizeof(testPair); i++)
    {
      if (filePath.length () <= 0 && (kern && tests[i].useKernel))
        {
          continue;
        }
      AddTestCase (new DceQuaggaTestCase (std::string (tests[i].name),
                                          Seconds (tests[i].duration), tests[i].useKernel,
                                          (!kern && tests[i].useKernel)),
                   TestCase::QUICK);
    }
}

} // namespace ns3
