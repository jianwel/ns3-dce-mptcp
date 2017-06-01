#ifndef FOLDER_CREATE_HELPER_H
#define FOLDER_CREATE_HELPER_H
#include "dce-application-helper.h"
//#include <fstream>
//#include <vector>


namespace ns3 {

class FolderCreateHelper : public DceApplicationHelper
{
public:
  FolderCreateHelper ();
  ~FolderCreateHelper ();
  virtual ApplicationContainer Install (NodeContainer c);
  virtual ApplicationContainer InstallInNode (Ptr<Node> node);


  /**
   * Reset environmental variables for the main binary for this application.
   */
  //void ResetEnvironment (void);


private:
};

}
#endif // CCN_HELPER_H
