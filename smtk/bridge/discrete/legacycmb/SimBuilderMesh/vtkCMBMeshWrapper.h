//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkCMBMeshWrapper - Wrapper to get the vtkCMBMesh on the server.
// .SECTION Description
// Wrapper to get the vtkCMBMesh on the server.  A vtkSMProxyProperty is
// created on the client for this purpose.

#ifndef __vtkCMBMeshWrapper_h
#define __vtkCMBMeshWrapper_h

#include "vtkObject.h"
#include "cmbSystemConfig.h"

class vtkCMBMeshServer;
class vtkDiscreteModelWrapper;

class VTK_EXPORT vtkCMBMeshWrapper : public vtkObject
{
public:
  static vtkCMBMeshWrapper* New();
  vtkTypeMacro(vtkCMBMeshWrapper, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Initialize vtkCMBMeshServer.
  void SetModelWrapper(vtkDiscreteModelWrapper*);

  vtkCMBMeshServer* GetMesh();

  // Description:
  // Pass to actual mesh
  virtual void SetGlobalLength(double length);
  virtual void SetGlobalMinimumAngle(double angle);

protected:
  vtkCMBMeshWrapper();
  ~vtkCMBMeshWrapper();

private:
  vtkCMBMeshWrapper(const vtkCMBMeshWrapper&); // Not implemented
  void operator=(const vtkCMBMeshWrapper&); // Not implemented
  vtkCMBMeshServer* Mesh;

};

#endif
