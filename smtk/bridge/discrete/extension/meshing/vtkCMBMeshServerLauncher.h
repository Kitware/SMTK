//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkCMBMeshServerLauncher
// .SECTION Description
// Starts up a thread remus::server and reports back the host name
// and port that the server has bound to

#ifndef __smtkdiscrete_vtkCMBMeshServerLauncher_h
#define __smtkdiscrete_vtkCMBMeshServerLauncher_h

#include "smtk/bridge/discrete/extension/vtkSMTKDiscreteExtModule.h" // For export macro
#include "vtkObject.h"
#include "vtkStdString.h" //needed for the HostName

namespace remus { namespace server { class Server; } }

namespace smtk {
  namespace bridge {
    namespace discrete {

class VTKSMTKDISCRETEEXT_EXPORT vtkCMBMeshServerLauncher : public vtkObject
{
public:
  //construction of this class will spawn
  //the CMBMeshServer
  vtkTypeMacro(vtkCMBMeshServerLauncher,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);
  static vtkCMBMeshServerLauncher *New();

  //create a CMBMeshServer
  //returns if we launched a server, if a server already exists
  //this will return success
  int Launch();

  //returns if we have a CMBMeshServer running
  int IsAlive();

  //kills the current mesh server if it exists;
  int Terminate();

  //get the host name of the server we created
  const char* GetHostName() const { return HostName.c_str(); }

  //get the port of the server we created
  vtkGetMacro(PortNumber,int)

protected:
  vtkCMBMeshServerLauncher();
  ~vtkCMBMeshServerLauncher();

private:
  vtkCMBMeshServerLauncher(const vtkCMBMeshServerLauncher&);  // Not implemented.
  void operator=(const vtkCMBMeshServerLauncher&);  // Not implemented.

  vtkStdString HostName;
  int PortNumber;
  bool Alive;
  remus::server::Server* Implementation;
};

    } // namespace discrete
  } // namespace bridge
} // namespace smtk

#endif
