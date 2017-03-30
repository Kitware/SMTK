//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkCMBModelOmicronMeshInputWriterBase -
// .SECTION Description

#ifndef __vtkCMBModelOmicronMeshInputWriterBase_h
#define __vtkCMBModelOmicronMeshInputWriterBase_h

#include "cmbSystemConfig.h"
#include "vtkCmbDiscreteModelModule.h" // For export macro
#include "vtkObject.h"

//class vtkDiscreteModel;
//class vtkDiscreteModelGeometricEntity;
class vtkDiscreteModelWrapper;

class VTKCMBDISCRETEMODEL_EXPORT vtkCMBModelOmicronMeshInputWriterBase : public vtkObject
{
public:
  static vtkCMBModelOmicronMeshInputWriterBase* New();
  vtkTypeMacro(vtkCMBModelOmicronMeshInputWriterBase, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Write the CMB file out.
  void Operate(vtkDiscreteModelWrapper* ModelWrapper);

  // Description:
  // Get/Set the name of the output file.
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

  // Description:
  // Get/Set the name of the geometry file.
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

  // Description:
  // Returns success (1) or failue (0) for Operation.
  vtkGetMacro(OperateSucceeded, int);

protected:
  vtkCMBModelOmicronMeshInputWriterBase();
  virtual ~vtkCMBModelOmicronMeshInputWriterBase();

private:
  vtkCMBModelOmicronMeshInputWriterBase(
    const vtkCMBModelOmicronMeshInputWriterBase&);              // Not implemented.
  void operator=(const vtkCMBModelOmicronMeshInputWriterBase&); // Not implemented.

  // Description:
  // The name of the file to be written.
  char* FileName;

  // Description:
  // The name of the geoemtry file to be listed in the output file.
  char* GeometryFileName;

  // Description:
  // The name of the TetGen Options to be given to the mesher.
  char* TetGenOptions;

  // Description:
  // The volume constraint to pass to the mesher.
  double VolumeConstraint;

  // Description:
  // Flag to indicate that the operation on the model succeeded (1) or not (0).
  int OperateSucceeded;
};

#endif
