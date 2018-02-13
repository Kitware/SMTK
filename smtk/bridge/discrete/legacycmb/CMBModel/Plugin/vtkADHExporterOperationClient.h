//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkADHExporterOperationClient - Write the ADH input on the server.
// .SECTION Description
// Operation to write all of the information for
// the ADH file that is getting exported from SimBuilder.  This operator
// gets text from the client that needs to get written out (this
// information is stored in ClientText).  It then writes out that
// information on the server and then adds in the nodal (NDS) and face
// (FCS) boundary condition cards to the file for each node and face
//  in the grid.

#ifndef __vtkADHExporterOperationClient_h
#define __vtkADHExporterOperationClient_h

#include "cmbSystemConfig.h"
#include "vtkADHExporterOperationBase.h"

class vtkSMProxy;

class VTK_EXPORT vtkADHExporterOperationClient : public vtkADHExporterOperationBase
{
public:
  static vtkADHExporterOperationClient* New();
  vtkTypeMacro(vtkADHExporterOperationClient, vtkADHExporterOperationBase);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Get/Set the text that the client generated
  vtkSetStringMacro(ClientText);
  vtkGetStringMacro(ClientText);

  using Superclass::Operate;

  // Description:
  virtual bool Operate(vtkDiscreteModel* model, vtkSMProxy* serverModelProxy);

protected:
  vtkADHExporterOperationClient();
  virtual ~vtkADHExporterOperationClient();

  char* ClientText;

private:
  vtkADHExporterOperationClient(const vtkADHExporterOperationClient&); // Not implemented.
  void operator=(const vtkADHExporterOperationClient&);                // Not implemented.
};

#endif
