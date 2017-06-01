/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) jianwel@g.clemson.edu
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
 */

#ifndef KERNEL_LOG_READER_HELPER_H
#define KERNEL_LOG_READER_HELPER_H

//#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/node-container.h"
#include <vector>
#include <string>

namespace ns3 {
    typedef std::map<std::string, double> m_dataMap;
    typedef std::map<std::string, std::vector<double> > m_dataCache;

/**
 * \brief Helper class that adds ns3::Ipv4DceRouting objects
 *
 * This class is expected to be used in conjunction with
 * ns3::InternetStackHelper::SetRoutingHelper
 */
class KernelLogReaderHelper 
{
public:

    KernelLogReaderHelper (const std::vector<std::string>& mNames,const NodeContainer* nodes, const std::string& pattern, size_t maxLineWidth = 300);
    ~KernelLogReaderHelper ();
    void doOneRead();
    
    double getMetric(int nodeID, const char* metricName);



private:

    char* line;
    char patternString[20];
    char metric[20];
    std::string pattern;

    size_t maxLineWidth;

    std::vector<int> nodeIDs;
    std::vector<std::string> mNames;
    std::vector<m_dataMap> m_values;
    std::vector<m_dataCache> m_caches;
    std::vector<unsigned int> last_posV;

    std::ostringstream cmd_oss;   
    FILE* getFileFP(int nodeID);
  /**
   * \internal
   * \brief Assignment operator declared private and not implemented to disallow
   * assignment and prevent the compiler from happily inserting its own.
   */
  KernelLogReaderHelper &operator = (const KernelLogReaderHelper &o);

  /**
   * \brief Construct an KernelLogReader from another previously
   * initialized instance (Copy Constructor).
   */
  KernelLogReaderHelper (const KernelLogReaderHelper &);
};

} // namespace ns3

#endif /* KERNEL_LOG_READER_HELPER_H */
