//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkCMBMeshToModelReadOperator -
// .SECTION Description
// Front end for the readers.  Reads in a "m2m" file and load the meshing
// info to the model with vtkCMBMeshToModelReader

#ifndef __vtkCMBMeshToModelReadOperator_h
#define __vtkCMBMeshToModelReadOperator_h

#include "vtkObject.h"
#include "cmbSystemConfig.h"

class vtkCMBParserBase;
class vtkDiscreteModelWrapper;
class vtkPolyData;
class vtkDiscreteModel;

class VTK_EXPORT vtkCMBMeshToModelReadOperator : public vtkObject
{
public:
  static vtkCMBMeshToModelReadOperator * New();
  vtkTypeMacro(vtkCMBMeshToModelReadOperator,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Load the file into Model.
  void Operate(vtkDiscreteModelWrapper* ModelWrapper);

  // Description:
  // Get/Set the name of the input file.
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

  // Description:
  // Returns success (1) or failue (0) for Operation.
  vtkGetMacro(OperateSucceeded, int);

protected:
  vtkCMBMeshToModelReadOperator();
  virtual ~vtkCMBMeshToModelReadOperator();

private:
  // Description:
  // The name of the file to be read in.
  char* FileName;

  vtkCMBMeshToModelReadOperator(const vtkCMBMeshToModelReadOperator&);  // Not implemented.
  void operator=(const vtkCMBMeshToModelReadOperator&);  // Not implemented.

  // Description:
  // Flag to indicate that the operation on the model succeeded (1) or not (0).
  int OperateSucceeded;
};

#endif
