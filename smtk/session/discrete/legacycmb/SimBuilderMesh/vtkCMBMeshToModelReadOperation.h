//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkCMBMeshToModelReadOperation -
// .SECTION Description
// Front end for the readers.  Reads in a "m2m" file and load the meshing
// info to the model with vtkCMBMeshToModelReader

#ifndef __vtkCMBMeshToModelReadOperation_h
#define __vtkCMBMeshToModelReadOperation_h

#include "cmbSystemConfig.h"
#include "vtkObject.h"

class vtkCMBParserBase;
class vtkDiscreteModelWrapper;
class vtkPolyData;
class vtkDiscreteModel;

class VTK_EXPORT vtkCMBMeshToModelReadOperation : public vtkObject
{
public:
  static vtkCMBMeshToModelReadOperation* New();
  vtkTypeMacro(vtkCMBMeshToModelReadOperation, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;

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
  vtkCMBMeshToModelReadOperation();
  virtual ~vtkCMBMeshToModelReadOperation();

private:
  // Description:
  // The name of the file to be read in.
  char* FileName;

  vtkCMBMeshToModelReadOperation(const vtkCMBMeshToModelReadOperation&); // Not implemented.
  void operator=(const vtkCMBMeshToModelReadOperation&);                 // Not implemented.

  // Description:
  // Flag to indicate that the operation on the model succeeded (1) or not (0).
  int OperateSucceeded;
};

#endif
