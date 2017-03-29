//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkCMBImportBCFileOperatorClient - Imports the BC file info.
// .SECTION Description
// Operator that reads in a BC file on the server from the client and creates a
// vtkCmbBCGridRepresentation to be used by the model.  Currently
// the BC file only has enough information to work properly for the
// boundary groups that existed when the volumetric mesh was generated.

#ifndef __vtkCMBImportBCFileOperatorClient_h
#define __vtkCMBImportBCFileOperatorClient_h

#include "cmbSystemConfig.h"
#include "vtkObject.h"

class vtkDiscreteModel;
class vtkSMProxy;

class VTK_EXPORT vtkCMBImportBCFileOperatorClient : public vtkObject
{
public:
  static vtkCMBImportBCFileOperatorClient * New();
  vtkTypeMacro(vtkCMBImportBCFileOperatorClient,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Get/Set the name of the file to be imported from.
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

  // Description:
  // Reads in the file on the server. Returns true if the operation was successful.
  bool Operate(vtkDiscreteModel* model, vtkSMProxy* serverModelProxy);

protected:
  vtkCMBImportBCFileOperatorClient();
  virtual ~vtkCMBImportBCFileOperatorClient();

  // Description:
  // Check to see if everything is properly set for the operator.
  virtual bool AbleToOperate(vtkDiscreteModel* model);

private:
  // Description:
  // The name of the file to be read.
  char* FileName;

  vtkCMBImportBCFileOperatorClient(const vtkCMBImportBCFileOperatorClient&);  // Not implemented.
  void operator=(const vtkCMBImportBCFileOperatorClient&);  // Not implemented.
};

#endif
