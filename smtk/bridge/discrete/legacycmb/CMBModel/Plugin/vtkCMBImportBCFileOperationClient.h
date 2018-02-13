//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkCMBImportBCFileOperationClient - Imports the BC file info.
// .SECTION Description
// Operation that reads in a BC file on the server from the client and creates a
// vtkCmbBCGridRepresentation to be used by the model.  Currently
// the BC file only has enough information to work properly for the
// boundary groups that existed when the volumetric mesh was generated.

#ifndef __vtkCMBImportBCFileOperationClient_h
#define __vtkCMBImportBCFileOperationClient_h

#include "cmbSystemConfig.h"
#include "vtkObject.h"

class vtkDiscreteModel;
class vtkSMProxy;

class VTK_EXPORT vtkCMBImportBCFileOperationClient : public vtkObject
{
public:
  static vtkCMBImportBCFileOperationClient* New();
  vtkTypeMacro(vtkCMBImportBCFileOperationClient, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Get/Set the name of the file to be imported from.
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

  // Description:
  // Reads in the file on the server. Returns true if the operation was successful.
  bool Operate(vtkDiscreteModel* model, vtkSMProxy* serverModelProxy);

protected:
  vtkCMBImportBCFileOperationClient();
  virtual ~vtkCMBImportBCFileOperationClient();

  // Description:
  // Check to see if everything is properly set for the operator.
  virtual bool AbleToOperate(vtkDiscreteModel* model);

private:
  // Description:
  // The name of the file to be read.
  char* FileName;

  vtkCMBImportBCFileOperationClient(const vtkCMBImportBCFileOperationClient&); // Not implemented.
  void operator=(const vtkCMBImportBCFileOperationClient&);                    // Not implemented.
};

#endif
