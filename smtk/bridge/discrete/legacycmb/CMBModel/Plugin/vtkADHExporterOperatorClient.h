//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkADHExporterOperatorClient - Write the ADH input on the server.
// .SECTION Description
// Operator to write all of the information for
// the ADH file that is getting exported from SimBuilder.  This operator
// gets text from the client that needs to get written out (this
// information is stored in ClientText).  It then writes out that
// information on the server and then adds in the nodal (NDS) and face
// (FCS) boundary condition cards to the file for each node and face
//  in the grid.

#ifndef __vtkADHExporterOperatorClient_h
#define __vtkADHExporterOperatorClient_h

#include "cmbSystemConfig.h"
#include "vtkADHExporterOperatorBase.h"

class vtkSMProxy;

class VTK_EXPORT vtkADHExporterOperatorClient : public vtkADHExporterOperatorBase
{
public:
  static vtkADHExporterOperatorClient* New();
  vtkTypeMacro(vtkADHExporterOperatorClient, vtkADHExporterOperatorBase);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Get/Set the text that the client generated
  vtkSetStringMacro(ClientText);
  vtkGetStringMacro(ClientText);

  using Superclass::Operate;

  // Description:
  virtual bool Operate(vtkDiscreteModel* model, vtkSMProxy* serverModelProxy);

protected:
  vtkADHExporterOperatorClient();
  virtual ~vtkADHExporterOperatorClient();

  char* ClientText;

private:
  vtkADHExporterOperatorClient(const vtkADHExporterOperatorClient&); // Not implemented.
  void operator=(const vtkADHExporterOperatorClient&);               // Not implemented.
};

#endif
