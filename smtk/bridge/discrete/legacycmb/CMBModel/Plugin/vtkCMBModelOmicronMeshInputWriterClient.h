//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkCMBModelOmicronMeshInputWriterClient -
// .SECTION Description


#ifndef __vtkCMBModelOmicronMeshInputWriterClient_h
#define __vtkCMBModelOmicronMeshInputWriterClient_h

#include "cmbSystemConfig.h"
#include "vtkObject.h"

class vtkDiscreteModel;
class vtkSMProxy;

class VTK_EXPORT vtkCMBModelOmicronMeshInputWriterClient : public vtkObject
{
public:
  static vtkCMBModelOmicronMeshInputWriterClient * New();
  vtkTypeMacro(vtkCMBModelOmicronMeshInputWriterClient,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  bool Operate(vtkDiscreteModel* model, vtkSMProxy* serverModelProxy);

  // Description:
  // Get/Set the name of the input file.
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

  // Description:
  // Get/Set the filename of the geometry file associated with this file.
  vtkSetStringMacro(GeometryFileName);
  vtkGetStringMacro(GeometryFileName);

  // Description:
  // Get/Set the TetGen Options.
  vtkSetStringMacro(TetGenOptions);
  vtkGetStringMacro(TetGenOptions);

  // Description:
  // Get/Set the volume constraint
  vtkSetMacro(VolumeConstraint, double);
  vtkGetMacro(VolumeConstraint, double);

protected:
  vtkCMBModelOmicronMeshInputWriterClient();
  virtual ~vtkCMBModelOmicronMeshInputWriterClient();

private:
  vtkCMBModelOmicronMeshInputWriterClient(const vtkCMBModelOmicronMeshInputWriterClient&);  // Not implemented.
  void operator=(const vtkCMBModelOmicronMeshInputWriterClient&);  // Not implemented.

  // Description:
  // The name of the file to be written.
  char* FileName;

  char* GeometryFileName;
  char* TetGenOptions;
  double VolumeConstraint;
};

#endif
