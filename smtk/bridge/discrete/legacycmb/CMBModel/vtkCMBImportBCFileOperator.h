//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkCMBImportBCFileOperator -Imports the BC file info.
// .SECTION Description
// Operator that reads in a BC file on the server and creates a
// vtkModelBCGridRepresentation to be used by the model.  Currently
// the BC file only has enough information to work properly for the
// boundary groups that existed when the volumetric mesh was generated.

#ifndef __vtkCMBImportBCFileOperator_h
#define __vtkCMBImportBCFileOperator_h

#include "cmbSystemConfig.h"
#include "vtkCmbDiscreteModelModule.h" // For export macro
#include "vtkObject.h"

class vtkDiscreteModelWrapper;

class VTKCMBDISCRETEMODEL_EXPORT vtkCMBImportBCFileOperator : public vtkObject
{
public:
  static vtkCMBImportBCFileOperator * New();
  vtkTypeMacro(vtkCMBImportBCFileOperator,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Reads in the file assuming we're on the server.
  // Sets OperateSucceeded.
  void Operate(vtkDiscreteModelWrapper* modelWrapper);

  // Description:
  // Get/Set the name of the input file.
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

  // Description:
  // Returns success (1) or failue (0) for Operation.
  vtkGetMacro(OperateSucceeded, int);

protected:
  vtkCMBImportBCFileOperator();
  virtual ~vtkCMBImportBCFileOperator();

private:
  // Description:
  // The name of the file to be read.
  char* FileName;

  vtkCMBImportBCFileOperator(const vtkCMBImportBCFileOperator&);  // Not implemented.
  void operator=(const vtkCMBImportBCFileOperator&);  // Not implemented.

  // Description:
  // Flag to indicate that the operation on the model succeeded (1) or not (0).
  int OperateSucceeded;
};

#endif
