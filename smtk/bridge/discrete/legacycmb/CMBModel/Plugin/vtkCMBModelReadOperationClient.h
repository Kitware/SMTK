//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkCMBModelReadOperationClient - Read in a CMB file.
// .SECTION Description
// Operation that reads in a CMB file on the server and then
// serializes it onto the client.

#ifndef __vtkCMBModelReadOperationClient_h
#define __vtkCMBModelReadOperationClient_h

#include "cmbSystemConfig.h"
#include "vtkObject.h"

class vtkDiscreteModel;
class vtkSMProxy;

class VTK_EXPORT vtkCMBModelReadOperationClient : public vtkObject
{
public:
  static vtkCMBModelReadOperationClient* New();
  vtkTypeMacro(vtkCMBModelReadOperationClient, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Get/Set the name of the file to be read in.
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

  // Description:
  // Reads in the file on the server and serializes the model
  // to the client.  Returns true if the operation was successful.
  bool Operate(vtkDiscreteModel* Model, vtkSMProxy* ServerModelProxy);

protected:
  vtkCMBModelReadOperationClient();
  virtual ~vtkCMBModelReadOperationClient();

  // Description:
  // Check to see if everything is properly set for the operator.
  virtual bool AbleToOperate(vtkDiscreteModel* Model);

private:
  // Description:
  // The name of the file to be read.
  char* FileName;

  vtkCMBModelReadOperationClient(const vtkCMBModelReadOperationClient&); // Not implemented.
  void operator=(const vtkCMBModelReadOperationClient&);                 // Not implemented.
};

#endif
