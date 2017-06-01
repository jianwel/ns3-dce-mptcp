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

#include <vector>
#include "ns3/log.h"
#include "ns3/ptr.h"
#include "ns3/names.h"
#include "ns3/node.h"
#include "ns3/assert.h"
#include <cstdio>


#include "kernel-log-reader-helper.h"

NS_LOG_COMPONENT_DEFINE ("KernelLogReaderHelper");

using std::cout;
using std::cerr;
using std::endl;
using std::string;

namespace ns3 {

KernelLogReaderHelper::KernelLogReaderHelper (const std::vector<std::string> & mNames0, const NodeContainer* nodes, const string& pattern0, size_t maxLineWidth0) : mNames(mNames0), maxLineWidth(maxLineWidth0), pattern(pattern0)
{
    for(int i=0; i< nodes->GetN(); i++)
    {
        std::cout<<"id from get id: "<<nodes->Get(i)->GetId()<<std::endl;
        nodeIDs.push_back(nodes->Get(i)->GetId());
    }
    for (int i=0; i< mNames.size(); i++)
    {
        cout<<"monitorint -- "<<endl;
        cout<<mNames[i]<<endl;
    }

    m_values.resize(nodes->GetN());
    m_caches.resize(nodes->GetN());
    last_posV.resize(nodes->GetN());
    for (int i=0; i< last_posV.size(); i++)
        last_posV[i] = 0;


    line = (char*) malloc(maxLineWidth); 
        
}


KernelLogReaderHelper::~KernelLogReaderHelper ()
{
    if(!line)
    {
        free(line);
        line = NULL;
    }
}


FILE* KernelLogReaderHelper::getFileFP(int nodeID)
{ 
    cmd_oss.str("");
    cmd_oss<<nodeID;
    string filename = "files-"+cmd_oss.str()+"/var/log/messages";
    return fopen(filename.c_str(), "r");
}


double KernelLogReaderHelper::getMetric(int nodeID, const char* metricName)
{
    if (m_values[nodeID].size()==0)
    {
        cerr<<"no data right now for node "<< nodeID<<endl;
        return -1;
    }
    
    m_dataMap::iterator ldit = m_values[nodeID].find(string(metricName));
          if(ldit !=  m_values[nodeID].end())
             return ldit->second;
          else
          {
              cerr<<"can not find srrt metric"<<endl;
              return -1;
          }

}

void KernelLogReaderHelper::doOneRead()
{
    m_values.clear();
  m_values.resize(nodeIDs.size());

    for(int i=0; i< nodeIDs.size(); i++)
    {
        FILE* fp = getFileFP(nodeIDs[i]);

        if(fp == NULL)
            continue;
    
        int lastReadN = 0;
        int readN = 0;
        int scanN = 0;
        double value =0;

        //first fseek to the last_pos
    fseek(fp, last_posV[i], SEEK_SET);

    while (readN = getline(&line, &maxLineWidth, fp) != EOF)
    {
        lastReadN = strlen(line);
       // cout<<"last "<<lastReadN<<endl;
        scanN =0; 
        scanN = sscanf(line, "%s : %s = %lf", patternString, metric, &value);
        //cout<<patternString<<endl;
        //cout<<metric <<" "<<value<<endl;
        m_caches[i][string(patternString)].push_back(value);
    }

        //cout<<"#last "<<lastReadN<<endl;

    int ntell = ftell(fp);
    cout<<"ntell=" <<ntell<<endl;

 if(scanN >0 && !strcmp(patternString, pattern.c_str()))
    {
      //cout<<"inside  "<<lastReadN<<" "<<ntell<<endl;
      ntell -= lastReadN;
    }
     last_posV[i] = ntell;

        fclose(fp);

        //calc the average

    for(m_dataCache::iterator it = m_caches[i].begin(); it != m_caches[i].end(); it++)
    {
        double sum_cache = 0;
        for (int j=0; j< it->second.size(); j++)
        {
            sum_cache += it->second[j];
        }

        double avg = sum_cache / it->second.size();
        m_values[i][it->first] = avg;

    }
    
}//for nodes

  //clear the cache vectors
  m_caches.clear();
  m_caches.resize(nodeIDs.size());

}//oneRead


}
 // namespace ns3
