#include "folder-create-helper.h"
#include "dce-application.h"
#include "ns3/log.h"
#include "utils.h"
//#include <fstream>
#include <stdlib.h>
#include <unistd.h>

NS_LOG_COMPONENT_DEFINE ("FolderCreateHelper");

namespace ns3 {

FolderCreateHelper::FolderCreateHelper ()
{
}

FolderCreateHelper::~FolderCreateHelper ()
{
    /*
  std::stringstream oss;
  oss << "rm -rf /tmp/iperf3*";
  ::system (oss.str ().c_str ());
  */
}
  
ApplicationContainer
FolderCreateHelper::Install (NodeContainer c)
{
  NS_LOG_FUNCTION (this);
  ApplicationContainer apps;
  for (NodeContainer::Iterator j = c.Begin (); j != c.End (); ++j)
    {
      int nodeId = (*j)->GetId ();
      std::stringstream oss;

      oss << "/tmp";
       UtilsEnsureDirectoryExists (UtilsGetAbsRealFilePath (nodeId, "/tmp"));
       UtilsEnsureDirectoryExists (UtilsGetAbsRealFilePath (nodeId, "/data"));
      //UtilsEnsureDirectoryExists (oss.str ());


    }
  return DceApplicationHelper::Install (c);
}

ApplicationContainer 
FolderCreateHelper::InstallInNode (Ptr<Node> node)
{
  NS_LOG_FUNCTION (this);
  ApplicationContainer apps;
      int nodeId = node->GetId ();
      std::stringstream oss;

      oss << "/tmp";
      UtilsEnsureDirectoryExists (oss.str ());

  return DceApplicationHelper::InstallInNode (node);
}
}
