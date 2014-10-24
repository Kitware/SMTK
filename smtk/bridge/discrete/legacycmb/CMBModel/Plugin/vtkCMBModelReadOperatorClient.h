//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkCMBModelReadOperatorClient - Read in a CMB file.
// .SECTION Description
// Operator that reads in a CMB file on the server and then
// serializes it onto the client.

#ifndef __vtkCMBModelReadOperatorClient_h
#define __vtkCMBModelReadOperatorClient_h

#include "vtkObject.h"
#include "cmbSystemConfig.h"

class vtkDiscreteModel;
class vtkSMProxy;

class VTK_EXPORT vtkCMBModelReadOperatorClient : public vtkObject
{
public:
  static vtkCMBModelReadOperatorClient * New();
  vtkTypeMacro(vtkCMBModelReadOperatorClient,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Get/Set the name of the file to be read in.
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

  // Description:
  // Reads in the file on the server and serializes the model
  // to the client.  Returns true if the operation was successful.
  bool Operate(vtkDiscreteModel* Model, vtkSMProxy* ServerModelProxy);

protected:
  vtkCMBModelReadOperatorClient();
  virtual ~vtkCMBModelReadOperatorClient();

  // Description:
  // Check to see if everything is properly set for the operator.
  virtual bool AbleToOperate(vtkDiscreteModel* Model);

private:
  // Description:
  // The name of the file to be read.
  char* FileName;

  vtkCMBModelReadOperatorClient(const vtkCMBModelReadOperatorClient&);  // Not implemented.
  void operator=(const vtkCMBModelReadOperatorClient&);  // Not implemented.
};

#endif
