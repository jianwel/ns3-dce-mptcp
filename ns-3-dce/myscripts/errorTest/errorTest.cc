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

#include "ns3/constant-position-mobility-model.h"
#include "ns3/constant-velocity-mobility-model.h"
#include "ns3/mobility-helper.h"
#include "ns3/rectangle.h"
#include "ns3/lte-spectrum-value-helper.h"

#include "ns3/fatal-error.h"



#include <errno.h>
#include <set>

#include <unistd.h>

using namespace ns3;
using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::vector;
using std::set;
using std::queue;


NS_LOG_COMPONENT_DEFINE ("SoftSimuMptcpLteWifi");

FILE** pfp = NULL;

int nUE = 1;
int nonBack=1;



int isAddError =1;
int globalSpeed = 5;

const int INTERVAL=5;
int nPrimeChange =0;
int nOpen= 0;
int nClose= 0;

int nChange=0;

string oursLog = "data/largerPredict/ours.log";
string sinrLog = "data/largerPredict/sinr.log";

string sinrRealLog = "data/largerPredict/sinr.log";
FILE* oursFp;

const int IPERF_RANDOM_SART = 5; 
int lteInter = 120;

const int HIST_Q_LEN =3;
const double SMALL_RATE = 0.000001;

int nAP =0;
int schedulingMethod =0;

void setPos (Ptr<Node> n, int x, int y, int z)
{
  Ptr<ConstantPositionMobilityModel> loc = CreateObject<ConstantPositionMobilityModel> ();
  n->AggregateObject (loc);
  Vector locVec2 (x, y, z);
  loc->SetPosition (locVec2);
}

void setPosLater (Ptr<Node> n, int x, int y, int z)
{
  Ptr<MobilityModel> loc = n->GetObject<MobilityModel> ();
  //cout<<"inside pos later"<<endl;
  //n->AggregateObject (loc);
  Vector locVec2 (x, y, z);
  loc->SetPosition (locVec2);
}


void setPosSpeed (Ptr<Node> n, int x, int y, int z, Ptr<Vector> speed)
{
  Ptr<ConstantVelocityMobilityModel> loc = CreateObject<ConstantVelocityMobilityModel> ();
  n->AggregateObject (loc);
  Vector locVec2 (x, y, z);
  loc->SetPosition (locVec2);

 //Vector locVec2 (x, y, z);

  //loc->SetVelocity (speed);
}



/*

void PrintLoc(const NodeContainer* pnodes, double next)
{
    const  NodeContainer& nodes = *pnodes;

    for(int i=0; i< nodes.GetN(); i++)
    {
        Ptr<MobilityModel> mob = nodes.Get(i)->GetObject<MobilityModel>();
        Vector pos = mob->GetPosition ();
        //std::cout << "POS: x=" << pos.x << ", y=" << pos.y << std::endl;
        float dist = sqrt(pos.x * pos.x + pos.y * pos.y);

        float sinr=-1;
        float sinrSq =-1;

        float sinrW=-1;
        float sinrSqW =-1;

        float sinrL=-1;
        float sinrSqL =-1;

    float predict =-1;
    float predictSq =-1;

 float predictW =0;
    float predictSqW =0;

 float predictL =0;
    float predictSqL =0;


    if(disWifi || (!disWifi && !disLte))
    {

        if(dist >1)
        {
            sinrL = 7e12 * 1/pow(dist, 3.5);
            sinrSqL= 7e12 * 1/pow(dist, 2);
        }
        else
        {
            sinrL  = 7e12;
            sinrSqL  = 7e12;
        }
        predictL = (0.57* 5* log2(1+sinrL)) /20;
        predictSqL= (0.57* 5* log2(1+sinrSqL)) /20;
    }
   
    if(disLte || (!disWifi && !disLte))
    {
        if(dist >1)
        {
            sinrW= 800 * 1/pow(dist, 3.5);
            sinrSqW = 800 * 1/pow(dist, 2);
        }
        else
        {
            sinrW  = 800;
            sinrSqW  = 800;
        }
        predictW = (0.33 * 20* log2(1+sinrW)) /20;
        predictSqW = (0.33 * 20* log2(1+sinrSqW)) /20;

    }

    predict= predictL + predictW;
    predictSq = predictSqL + predictSqW;

    fprintf(pfp[i], "%.1f  %.1f  %.1f  %.5f  %.6f  %.5f  %.6f  %.5f  %.6f  %.5f  %.6f  %.5f  %.6f\n", pos.x, pos.y, dist, sinrW , predictW, sinrSqW, predictSqW, sinrL , predictL, sinrSqL, predictSqL, predict, predictSq);   

    }


//change to another loc, need to change 6 times
        if(isControlLoc)
    {
        if(disLte || (!disLte && !disWifi) )
        {
            node0X -= wifiInter;
        }
        else
        {

            node0X -= lteInter;
        }

            //Vector Vspeed (-globalSpeed, 0, 0);
        //setPosSpeed ((nodes.Get(0)), node0X, 0, 0, Ptr<Vector>(&Vspeed));
        setPosLater ((nodes.Get(0)), node0X, 0, 0);

    }

   Simulator::Schedule (Seconds (5), &PrintLoc, &nodes, next);


}


*/
/*
class RAT
{

}

struct wifiAP
{
    Vector2D pos;

}
*/


struct UE
{
    float alpha;
    int start;
    int end;
    //Vector2D pos;
   // RAT* ratList;
   //
   //0 LTE; 1 Wifi
   //this is the groud truth
   vector<float> rates;
   vector<float> Rrates;
   vector<float> histRates;
   //vector<float> sinrBiasedRates;


   queue<float> wifiHistR;
   queue<float> lteHistR;

   int wifiIndex;
   //the results of the last time
   vector<bool> re;
   char primary;
};

struct Plan
{
    char lw[2];
};

int findNearestAP(const Vector2D& toFind, const vector<Vector2D>& wifiAPs, double* pdist)
{
    //Vector2D minP;
    int minP=-1;
    double min = 1e20;
    int i =0;
    for(vector<Vector2D>::const_iterator it=wifiAPs.begin(); it != wifiAPs.end(); it++)
    {
        double dist = CalculateDistance(toFind, *it);
        if(dist < min)
        {
            //minP = *it
            minP = i;
            min = dist;
        }
        i++;
    }

   *pdist = min;
   return minP;


}


 void allocateAPPos(vector<Vector2D>& wifiAPs, int nRows, int cellLen)
{
    Vector2D curr;
    int nLeft = (nRows -1)/2;
  
    for (int j = nLeft; j>= -nLeft; j--)
       for (int i = -nLeft; i<= nLeft; i++)
      {
          curr.x =  i * cellLen;
        curr.y =  j * cellLen;
        wifiAPs.push_back(curr);
      }
}

inline double calcLte(double dist)
{
    double sinrSqL;
    if(dist >1)

        sinrSqL= 7e12 * 1/pow(dist, 2);
    else
        sinrSqL = 7e12;


        double predictL = (0.57* 5* log2(1+sinrSqL)) ;
    return predictL;
}

inline double calcWifi(double dist)
{

double sinrSqL;
if(dist >1)
    sinrSqL= 1000 * 1/pow(dist, 2);
else
    sinrSqL = 1000;

        double predictL = (0.3* 20* log2(1+sinrSqL));
    return predictL;
}

bool isStable(const queue<float>& histQ)
{
    //return histQ[0] !=0;
    queue<float> temp = histQ;
    while(!temp.empty())
    {

    //for(queue<float>::const_iterator it = histQ.begin(); it != histQ.end(); it++)
       double front = temp.front();
        if(front < 1e-8)
            return false;
        temp.pop();
    }
    return true;

}

double avgQ(const queue<float>& histQ)
{
    queue<float> temp = histQ;
    double newW = 0.7;
    double sum =0;
    while(!temp.empty())
    {

       double front = temp.front();
        if(sum==0)
            sum = front;
        else
         sum = sum*0.3 + front*0.7;
        temp.pop();
    }
    return sum;

}

double calcU(const vector<UE>& ueV, const vector<int>& runningV, const vector<Plan>& plan, double& through)
{
    double sumU = 0;
    int nLte =0 ;
    vector<int>nWifi;
    nWifi.resize(nAP);

    through = 0;

    for (int i=0; i < runningV.size(); i++)
    {

        if(plan[i].lw[0] ==1)
        {
            nLte++;
        }
        if(plan[i].lw[1] ==1)
        {
            (nWifi[ueV[runningV[i]].wifiIndex])++;
        }
    }

    for (int i=0; i < runningV.size(); i++)
    {
        if(ueV[runningV[i]].rates[0] >0 && plan[i].lw[0] ==1)
        {
            sumU += log(ueV[runningV[i]].rates[0] /nLte *1000000);
            through += ueV[runningV[i]].rates[0] /nLte;
        }
        if(ueV[runningV[i]].rates[1] >0 && plan[i].lw[1] ==1)
        {
            sumU += log(1000000 *ueV[runningV[i]].rates[1] /(nWifi[ueV[runningV[i]].wifiIndex]));
            through += ueV[runningV[i]].rates[1] /(nWifi[ueV[runningV[i]].wifiIndex]);
        }
    }
    return sumU;
}


double calcUNoError(const vector<UE>& ueV, const vector<int>& runningV, const vector<Plan>& plan)
{
    double sumU = 0;
    int nLte =0 ;
    vector<int>nWifi;
    nWifi.resize(nAP);

   double  through = 0;

    for (int i=0; i < runningV.size(); i++)
    {

        if(plan[i].lw[0] ==1)
        {
            nLte++;
        }
        if(plan[i].lw[1] ==1)
        {
            (nWifi[ueV[runningV[i]].wifiIndex])++;
        }
    }

    for (int i=0; i < runningV.size(); i++)
    {
        if(ueV[runningV[i]].Rrates[0] >0 && plan[i].lw[0] ==1)
        {
            sumU += log(ueV[runningV[i]].Rrates[0] /nLte *1000000);
            through += ueV[runningV[i]].Rrates[0] /nLte;
        }
        if(ueV[runningV[i]].Rrates[1] >0 && plan[i].lw[1] ==1)
        {
            sumU += log(1000000 *ueV[runningV[i]].Rrates[1] /(nWifi[ueV[runningV[i]].wifiIndex]));
            through += ueV[runningV[i]].Rrates[1] /(nWifi[ueV[runningV[i]].wifiIndex]);
        }
    }
    return sumU;
}

double calcUOurs(const vector<UE>& ueV, const vector<int>& runningV, const vector<Plan>& plan)
{
    double sumU = 0;
    int nLte =0;
    vector<int>nWifi;
    for(int i=0; i< nWifi.size(); i++)
        nWifi[i]  =0;
    nWifi.resize(nAP);

    for (int i=0; i < runningV.size(); i++)
    {

        if(plan[i].lw[0] ==1)
        {
            nLte ++;
        }
        if(plan[i].lw[1] ==1)
        {
            (nWifi[ueV[runningV[i]].wifiIndex])++;
        }
    }

    for (int i=0; i < runningV.size(); i++)
    {
        if(ueV[runningV[i]].histRates[0] >0 && plan[i].lw[0] ==1)
            //sumU += log(1000000 *ueV[runningV[i]].histRates[0] /nLte);
            sumU += log(ueV[runningV[i]].histRates[0] /nLte);
        if(ueV[runningV[i]].histRates[1] >0 && plan[i].lw[1] ==1)
            //sumU += log(1000000 *ueV[runningV[i]].histRates[1] /(nWifi[ueV[runningV[i]].wifiIndex]));
            sumU += log(ueV[runningV[i]].histRates[1] /(nWifi[ueV[runningV[i]].wifiIndex]));
    }
    return sumU;


}

double calcUSINR(const vector<UE>& ueV, const vector<int>& runningV, const vector<Plan>& plan)
{
    double sumU = 0;
    int nLte =0;
    vector<int>nWifi;
    for(int i=0; i< nWifi.size(); i++)
        nWifi[i]  =0;
    nWifi.resize(nAP);

    for (int i=0; i < runningV.size(); i++)
    {

        if(plan[i].lw[0] ==1)
        {
            nLte ++;
        }
        if(plan[i].lw[1] ==1)
        {
            (nWifi[ueV[runningV[i]].wifiIndex])++;
        }
    }

    for (int i=0; i < runningV.size(); i++)
    {
        if(ueV[runningV[i]].rates[0] >0 && plan[i].lw[0] ==1)
            //sumU += log(1000000 *ueV[runningV[i]].histRates[0] /nLte);
            sumU += log(ueV[runningV[i]].rates[0] /nLte);
        if(ueV[runningV[i]].rates[1] >0 && plan[i].lw[1] ==1)
            //sumU += log(1000000 *ueV[runningV[i]].histRates[1] /(nWifi[ueV[runningV[i]].wifiIndex]));
            sumU += log(ueV[runningV[i]].rates[1] /(nWifi[ueV[runningV[i]].wifiIndex]));
    }
    return sumU;


}



void doOurs(vector<UE>& ueV, const vector<int>& runningV,  vector<Plan>& plan)
{
    double avgWifi, avgLte;
    vector<Plan> tempPlan = plan;
    //primary
    for (int i=0; i< runningV.size(); i++)
    {

    if(ueV[runningV[i]].wifiHistR.size() < HIST_Q_LEN || ueV[runningV[i]].lteHistR.size()  < HIST_Q_LEN)
        return;

    if(isStable(ueV[runningV[i]].wifiHistR))
    {
        
        //check wifi rate and lte rate
        avgWifi = avgQ(ueV[runningV[i]].wifiHistR);
        ueV[runningV[i]].histRates[1] = avgWifi;
        avgLte = avgQ(ueV[runningV[i]].lteHistR);
        ueV[runningV[i]].histRates[0] = avgLte;

       // cout<<avgWifi<<"  "<<avgLte<<endl;

        if(avgWifi > avgLte)
        {
            ueV[runningV[i]].primary = 1;
            tempPlan[i].lw[ueV[runningV[i]].primary] = 1;
            //cout<<"##primary changed"<<endl;
            nPrimeChange ++;
        }
        //if change the primary, we will not try to close the secondary?
        //the later part will try to close it based on utility
        else

            ueV[runningV[i]].primary = 0;
    }
    else
    {

            ueV[runningV[i]].primary = 0;
    }
    }

    double currentU = calcUOurs(ueV, runningV, tempPlan);
    //cout<<"currentU"<<currentU<<endl;

    double newU =0;

    //avgWifi from small to large
    for (int i=0; i< runningV.size(); i++)
    {
        int second = !(ueV[runningV[i]].primary) ;
        if(tempPlan[i].lw[second] ==1)
        {
            tempPlan[i].lw[second] = 0; 
            newU = calcUOurs(ueV, runningV, tempPlan);
            //cout<<"newU "<<newU;
            if(newU > currentU)
            {
                currentU =  newU;
                cout<<"###closed one"<<endl;
                nClose++;
            }
            else
            {
                tempPlan[i].lw[second] = 1; 
            }
        }
    }

    //exit(0);



   // cout<<"currentU 2 :"<<currentU<<endl;
    for (int i=0; i< runningV.size(); i++)
    {
        int second = !(ueV[runningV[i]].primary);
        if(tempPlan[i].lw[second] ==0)
        {

            tempPlan[i].lw[second] = 1;
            newU = calcUOurs(ueV, runningV, tempPlan);
           // cout<<"openU "<<newU;
            if(newU > currentU)
            {

                cout<<"@@@open one"<<endl;
                nOpen++;
                currentU =  newU;
            }
            else
            {
                tempPlan[i].lw[second] = 0; 
            }

        }

    }

    plan= tempPlan;
    //exit(0);

}

void doSecondRandom(vector<UE>& ueV, const vector<int>& runningV,  vector<Plan>& plan)
{
    vector<Plan>tempPlan = plan;
    int nFirst =0;
    int nSecond =0;
    for(int i=0; i< runningV.size(); i++)
    {

        if(ueV[runningV[i]].rates[1] > ueV[runningV[i]].rates[0])
        {
            ueV[runningV[i]].primary = 1;
            tempPlan[i].lw[ueV[runningV[i]].primary] = 1;
        }
        else
        {ueV[runningV[i]].primary = 0;
            tempPlan[i].lw[ueV[runningV[i]].primary] = 1;
        }

        int second = !(ueV[runningV[i]].primary);

        
        Ptr<UniformRandomVariable> changeFactor = CreateObject<UniformRandomVariable> ();
  changeFactor->SetAttribute ("Min", DoubleValue (-20000000));
  changeFactor->SetAttribute ("Max", DoubleValue (20000000));
   
  if(changeFactor->GetValue() >0)
  {
       tempPlan[i].lw[second] = 1;
       nFirst++;
  }
  else
  {
       tempPlan[i].lw[second] = 0;
       nSecond++;
  }




    }

    //cout<<"n First second"<<nFirst<<" "<<nSecond<<endl;
    plan = tempPlan;
         

}


void doSINR(const vector<UE>& ueV, const vector<int>& runningV,  vector<Plan>& plan)
{
    double delta =200;
    int loops =0;
    vector<Plan> tempPlan = plan;


    //double currentU = calcUSINR(ueV, runningV, tempPlan);
    double currentU = (isAddError ==1) ? calcUSINR(ueV, runningV, tempPlan) : calcUNoError(ueV, runningV, tempPlan);
    double newU = 0;

    while (delta > 0 && loops < 100)
    {
        delta =0;
        for(int i=0; i< runningV.size(); i++)
        {
            //case 1
            tempPlan[i].lw[0] = !(plan[i].lw[0]);
            //newU = calcUSINR(ueV, runningV, tempPlan);
            newU = (isAddError ==1) ? calcUSINR(ueV, runningV, tempPlan) : calcUNoError(ueV, runningV, tempPlan);
            if(newU > currentU)
            {
                currentU = newU;
                delta = newU - currentU;
            }
            else
            tempPlan[i].lw[0] = plan[i].lw[0];

            //case 2
            tempPlan[i].lw[1] = !(plan[i].lw[1]);
            newU = (isAddError ==1) ? calcUSINR(ueV, runningV, tempPlan) : calcUNoError(ueV, runningV, tempPlan);
            if(newU > currentU)
            {
                currentU = newU;
                delta = newU - currentU;
            }
            else
            tempPlan[i].lw[1] = plan[i].lw[1];

            //case 3
                        //case 2
            tempPlan[i].lw[0] = !(plan[i].lw[0]);
            tempPlan[i].lw[1] = !(plan[i].lw[1]);
            newU = (isAddError ==1) ? calcUSINR(ueV, runningV, tempPlan) : calcUNoError(ueV, runningV, tempPlan);
            if(newU > currentU)
            {
                currentU = newU;
                delta = newU - currentU;
            }
            else
            {
            tempPlan[i].lw[0] = plan[i].lw[0];
            tempPlan[i].lw[1] = plan[i].lw[1];
            }
        }
        loops++;
        

    }
    plan = tempPlan;
}

/*
void doSINR(const vector<UE>& ueV, const vector<int>& runningV,  vector<Plan>& plan)
{
    double delta =200;
    int loops =0;
    vector<Plan> tempPlan = plan;


    double currentU = calcUOurs(ueV, runningV, tempPlan);
    double newU = 0;

    while (delta > 0 && loops < 100)
    {
        delta =0;
        for(int i=0; i< runningV.size(); i++)
        {
            //case 1
            tempPlan[i].lw[0] = !(plan[i].lw[0]);
            newU = calcUOurs(ueV, runningV, tempPlan);
            if(newU > currentU)
            {
                currentU = newU;
                delta = newU - currentU;
                nChange++;
            }
            else
            tempPlan[i].lw[0] = plan[i].lw[0];

            //case 2
            tempPlan[i].lw[1] = !(plan[i].lw[1]);
            newU = calcUOurs(ueV, runningV, tempPlan);
            if(newU > currentU)
            {
                currentU = newU;
                delta = newU - currentU;
                nChange++;
            }
            else
            tempPlan[i].lw[1] = plan[i].lw[1];

            //case 3
                        //case 2
            tempPlan[i].lw[0] = !(plan[i].lw[0]);
            tempPlan[i].lw[1] = !(plan[i].lw[1]);
            newU = calcUOurs(ueV, runningV, tempPlan);
            if(newU > currentU)
            {
                currentU = newU;
                delta = newU - currentU;
                nChange++;
            }
            else
            {
            tempPlan[i].lw[0] = plan[i].lw[0];
            tempPlan[i].lw[1] = plan[i].lw[1];
            }
        }
        loops++;
        

    }
    plan = tempPlan;
}

*/





void init(vector<UE>& ueV)
{
    for(int i=0; i< ueV.size(); i++)
    {
      ueV[i].rates.resize(2);
      ueV[i].Rrates.resize(2);
      ueV[i].histRates.resize(2);
      ueV[i].re.resize(2);
      ueV[i].re[0] = 1;
      ueV[i].re[1] = 1;
      ueV[i].primary = 0;


    }

}

double doBiasWifi(double rate, double dist)
{
    if(dist <20)
        rate /=4;
    else if(dist >20 && dist <40)
        rate /=2.5;
    else if(dist >60 )
        rate *=1.5;
    return rate;
}

double doBiasLte(double rate, double dist)
{
    if(dist <=210)
        rate /=20;
    else if(dist >210 && dist <=500)
        rate /=10;
   else if(dist >500 && dist <=4000)
        rate /=4;
    else if(dist >4000 )
        rate *=1.3;
    return rate;
}



void doSchedule(vector<UE>& ueV, set<int>& running, double sTimer, const vector<Vector2D>& wifiAPs, const NodeContainer* pnodes)
{

    const  NodeContainer& nodes = *pnodes;
  double time_tp = Simulator::Now ().GetSeconds ();

  for(int i=0; i< ueV.size(); i++)
  {
      if(time_tp >= ueV[i].start)
      {
          running.insert(i);

      }
      if(time_tp>= ueV[i].end)
      {
          running.erase(i);
      }
  }

  vector<int> runningV;

  for(set<int>::iterator it = running.begin(); it != running.end(); it++)
      runningV.push_back(*it);

  vector<Plan> plan;
  plan.resize(runningV.size());
  //cout<<"running size"<<runningV.size()<<endl;

  //prepare info for two scheduling algorithms
  for (int i =0; i< runningV.size(); i++)
  {
      Ptr<MobilityModel> mob = nodes.Get(runningV[i])->GetObject<MobilityModel>();
        Vector pos = mob->GetPosition ();

      Vector2D pos2D (pos.x, pos.y);

      double dist = CalculateDistance(pos2D, Vector2D(0, 0));
      double rateLte = calcLte(dist);

      ueV[runningV[i]].rates[0] = rateLte;

      double realRatelte = doBiasLte(rateLte, dist) * ueV[runningV[i]].alpha;

      ueV[runningV[i]].Rrates[0] = realRatelte ;
      if(realRatelte < SMALL_RATE)
          realRatelte = 0;

    //cout<<"rea lte "<<realRatelte<<"  ";

        if (!ueV[runningV[i]].lteHistR.empty() && ueV[runningV[i]].lteHistR.size()== HIST_Q_LEN)
        {
            ueV[runningV[i]].lteHistR.pop();
        }
            ueV[runningV[i]].lteHistR.push(realRatelte);

   

      //get wifi

      double wifiDist;

      int apIndex = findNearestAP(pos2D, wifiAPs, &wifiDist);
      double rateWifi = calcWifi(wifiDist);


      ueV[runningV[i]].rates[1] = rateWifi;
      ueV[runningV[i]].wifiIndex = apIndex;

      //do biasing
      //
   double realRatewifi = doBiasWifi(rateWifi, wifiDist) * ueV[runningV[i]].alpha;

      //cout<<"rea wifi "<<realRatewifi<<"  ";
      ueV[runningV[i]].Rrates[1] = realRatewifi ;
    if(realRatewifi < SMALL_RATE)
          realRatewifi = 0;

            if (!ueV[runningV[i]].wifiHistR.empty() && ueV[runningV[i]].wifiHistR.size()== HIST_Q_LEN)
        {
            ueV[runningV[i]].wifiHistR.pop();
        }
            ueV[runningV[i]].wifiHistR.push(realRatewifi);


      
//copy the current plan
        plan[i].lw[0] = ueV[runningV[i]].re[0];
        plan[i].lw[1] = ueV[runningV[i]].re[1];
      
  }

  
  
  //if UE not sent in this period, put 0 in history
  for(int j = 0; j< ueV.size(); j++)
  {
      vector<int>::iterator ii = find(runningV.begin(), runningV.end(), j);
      if(ii == runningV.end())
      {


          if (!ueV[j].wifiHistR.empty() && ueV[j].wifiHistR.size()== HIST_Q_LEN)
          {
              ueV[j].wifiHistR.pop();
          }
          ueV[j].wifiHistR.push(0);

          if (!ueV[j].lteHistR.empty() && ueV[j].lteHistR.size()== HIST_Q_LEN)
          {
              ueV[j].lteHistR.pop();
          }
          ueV[j].lteHistR.push(0);
      }

  }



  if(schedulingMethod == 0)
  {
      doOurs(ueV, runningV, plan);
  }else if(schedulingMethod == 1)
  {
      doSINR(ueV,runningV, plan);
  }
  else if (schedulingMethod == 2)
  {
      doSecondRandom(ueV,runningV, plan);
  }

  double through;

  double realU  = calcU(ueV, runningV, plan, through);

  fprintf(oursFp,"%.5f %.5f\n", realU, through);
  
  

for(int i=0; i< runningV.size(); i++)
{
  ueV[runningV[i]].re[0] = plan[i].lw[0];
  ueV[runningV[i]].re[1] = plan[i].lw[1];
}

  cout<<"realU ="<<realU<< "  th  "<<through <<endl;



 Simulator::Schedule(Seconds(sTimer), doSchedule, ueV, running,  sTimer, wifiAPs, pnodes);
}


void changeAlpha(vector<UE>& ueV, double sTimer)
{
    Ptr<UniformRandomVariable> changeFactor = CreateObject<UniformRandomVariable> ();
  changeFactor->SetAttribute ("Min", DoubleValue (0.33));
  changeFactor->SetAttribute ("Max", DoubleValue (3));

    for(int i=0; i<ueV.size(); i++)
    {

        ueV[i].alpha *= changeFactor->GetValue();
        if(ueV[i].alpha >1)
            ueV[i].alpha  = 1;
    }

 Simulator::Schedule(Seconds(4*sTimer), changeAlpha, ueV, sTimer);
}

void initAlpha(vector<UE>& ueV)
{
    Ptr<UniformRandomVariable> changeFactor = CreateObject<UniformRandomVariable> ();
  changeFactor->SetAttribute ("Min", DoubleValue (0.5));
  changeFactor->SetAttribute ("Max", DoubleValue (1));

    for(int i=0; i<ueV.size(); i++)
    {

        ueV[i].alpha = changeFactor->GetValue();
    }

}

void initAlphaBacklog(vector<UE>& ueV)
{
       for(int i=0; i<ueV.size(); i++)
    {

        ueV[i].alpha = 1;
    }

}

int main (int argc, char *argv[])
{
  LogComponentEnable ("SoftSimuMptcpLteWifi", LOG_LEVEL_ALL);
  std::string bufSize = "";

  bool isDownlink = true;
  double stopTime = 1520.0;
  int cellLen = 120;
  int nRows =5;
  int wifiCoverage =80;

  int uePerAp =20;


  double sTimer = 15.0;



  std::ostringstream cmd_oss;

  

  CommandLine cmd;
 // cmd.AddValue ("alpha", "traffic back log rate", alpha);
  cmd.AddValue ("cellLen", "len of cell", cellLen);
  cmd.AddValue ("isAddError", "do we add error", isAddError);
  cmd.AddValue ("nUE", "the number of UEs.", nUE);
  cmd.AddValue ("uePerAp", "the number of UEs per AP.", uePerAp);
  cmd.AddValue ("globalSpeed", "the global spedd.", globalSpeed);
  cmd.AddValue ("nRows", "nxn grid.", nRows);
  cmd.AddValue ("stopTime", "StopTime of simulatino.", stopTime);
  cmd.AddValue ("schedulingMethod", "method of simulation.", schedulingMethod);
  cmd.AddValue ("nonBack", "whether using alpha", nonBack);

  cmd.Parse (argc, argv);

  
  if(schedulingMethod ==1)
  {
      oursFp = fopen(sinrLog.c_str(), "w");
  }
  else
  {
      oursFp = fopen(oursLog.c_str(), "w");
  }

  if(oursFp ==NULL)
  {
      cerr<<"open file er"<<endl;
      exit(0);
  }



   
  int areaLen= 120* (nRows -1) + wifiCoverage *2 +20;
  double radius = areaLen *  (1.41421356 /2);
cout<<"lte freq. "<< LteSpectrumValueHelper::GetDownlinkCarrierFrequency(100)<<endl;

  cout<<"areaLen ="<<areaLen<<endl;
  cout<<"radisu ="<<radius<<endl;
  cout<<"sche method"<<schedulingMethod<<endl;
  cout<<"isAddError="<<isAddError<<endl;
  cout<<"nonBack="<<nonBack<<endl;


   nAP = nRows * nRows;

  vector<Vector2D> wifiAPs;
  allocateAPPos(wifiAPs, nRows, cellLen);

  nUE = nAP *uePerAp;

  vector<UE> uesV;
  uesV.resize(nUE);

  init(uesV);

   Ptr<UniformRandomVariable> flowStartSeconds = CreateObject<UniformRandomVariable> ();
  flowStartSeconds->SetAttribute ("Min", DoubleValue (0.2));
  flowStartSeconds->SetAttribute ("Max", DoubleValue (stopTime/2));


  Ptr<UniformRandomVariable> flowLenSeconds = CreateObject<UniformRandomVariable> ();
  flowLenSeconds->SetAttribute ("Min", DoubleValue (80));
  flowLenSeconds->SetAttribute ("Max", DoubleValue (stopTime));

  //init ue traffic
  for(int i=0; i< uesV.size(); i++)
  {
     // uesV[i].start = flowStartSeconds->GetValue();
      uesV[i].start = 0;
      //uesV[i].end = uesV[i].start + flowLenSeconds->GetValue() ;
      uesV[i].end = stopTime ;
      //cout<<uesV[i].start << "  "<<uesV[i].end<<endl;
  }

  set<int> running;


     MobilityHelper mobility;
/*

mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                  "MinX", DoubleValue (1.0),
                                  "MinY", DoubleValue (1.0),
                                  "DeltaX", DoubleValue (5.0),
                                  "DeltaY", DoubleValue (5.0),
                                  "GridWidth", UintegerValue (3),
                                  "LayoutType", StringValue ("RowFirst"));
                                  */
     cmd_oss.str("");
     cmd_oss<<radius;

     /*

Ptr<UniformRandomVariable> var = CreateObject<UniformRandomVariable> ();
   mobility.SetPositionAllocator ("ns3::UniformDiscPositionAllocator",
                              "X", StringValue ("0.0"),
                                 "Y", StringValue ("0.0"),
                                 //"Rho", StringValue (string("ns3::UniformRandomVariable[Min=0|Max=")+cmd_oss.str()+string("]")));
                                 "Rho",DoubleValue (var->GetValue(0, radius) ));
                                 */

   mobility.SetPositionAllocator ("ns3::RandomDiscPositionAllocator",
                              "X", StringValue ("0.0"),
                                 "Y", StringValue ("0.0"),
                                 "Rho", StringValue (string("ns3::UniformRandomVariable[Min=0|Max=")+cmd_oss.str()+string("]")));
                                 //"Rho",DoubleValue (var->GetValue(0, radius) ));

 cmd_oss.str("");
     cmd_oss<<globalSpeed;



   mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                              "Mode", StringValue ("Time"),
                              "Time", StringValue ("2s"),
                              "Speed", StringValue (string("ns3::ConstantRandomVariable[Constant=") +cmd_oss.str()+string("]")),
                              "Bounds", RectangleValue (Rectangle (-(radius+1), radius+1, -(radius+1), radius+1)));

  



   pfp = (FILE**) malloc(sizeof(FILE*) * nUE);
   for(int i=0; i<nUE; i++)
   {
       cmd_oss.str("");
       cmd_oss<<"data/largerPredict/s"<<i<<".txt";
       pfp[i]=fopen(cmd_oss.str().c_str(), "w");
       fprintf(pfp[i], "\n\n");
   }


  NodeContainer nodes;
  
  //node 0-9
  nodes.Create (nUE);

   mobility.Install (nodes);

if(nonBack)
   initAlpha(uesV);
else
   initAlphaBacklog(uesV);

   if(nonBack)
 Simulator::Schedule(Seconds(4*sTimer -0.1), changeAlpha, uesV, sTimer);

 Simulator::Schedule(Seconds(sTimer), doSchedule, uesV, running,  sTimer,wifiAPs, &nodes);
  

//Simulator::Schedule (Seconds (INTERVAL/2.0f), &Throughput);

//Simulator::Schedule (Seconds (5), &ProcGet);
//Simulator::Schedule (Seconds (5), &PrintLoc, &nodes, printTimer);


  Simulator::Stop (Seconds (stopTime+6));
  Simulator::Run ();

  //flowMonitor->SerializeToXmlFile("allFlow.xml", true, true);
  //sleep(1);

  cout<<"stat"<<nPrimeChange<<" "<<nOpen<< " "<<nClose<< " "<<nChange<<endl;

  for(int i=0; i<nodes.GetN(); i++)
    fclose(pfp[i]);

  fclose(oursFp);

  free(pfp);
  Simulator::Destroy ();

  return 0;
}
