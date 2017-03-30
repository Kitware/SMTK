//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkCMBModelFaceMeshClient - Mesh representation for a vtkModelFace
// .SECTION Description
// Mesh representation for a vtkModelFace.  The smaller value for
// maximum area and larger value for minimum angle wins for local
// vs. global comparisons.  Absolute values are used for area.

#ifndef __vtkCMBModelFaceMeshClient_h
#define __vtkCMBModelFaceMeshClient_h

#include "cmbSystemConfig.h"
#include "vtkCMBModelFaceMesh.h"

class VTK_EXPORT vtkCMBModelFaceMeshClient : public vtkCMBModelFaceMesh
{
public:
  static vtkCMBModelFaceMeshClient* New();
  vtkTypeMacro(vtkCMBModelFaceMeshClient, vtkCMBModelFaceMesh);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set the local mesh length, min angle
  virtual bool SetLocalLength(double length);
  virtual bool SetLocalMinimumAngle(double angle);

protected:
  vtkCMBModelFaceMeshClient();
  virtual ~vtkCMBModelFaceMeshClient();
  bool SendLengthAndAngleToServer();

  // Description:
  // This method builds the model entity's mesh without checking
  // the parameters.  Returns true if the operation succeeded as
  // desired.  This includes deleting the mesh if the mesh
  // parameters go from valid to invalid values (i.e. a parameter set to 0).
  bool BuildMesh(bool meshHigherDimensionalEntities);

private:
  vtkCMBModelFaceMeshClient(const vtkCMBModelFaceMeshClient&); // Not implemented.
  void operator=(const vtkCMBModelFaceMeshClient&);            // Not implemented.
};

#endif
