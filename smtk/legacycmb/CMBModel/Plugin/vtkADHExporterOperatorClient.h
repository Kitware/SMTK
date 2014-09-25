/*=========================================================================

Copyright (c) 1998-2005 Kitware Inc. 28 Corporate Drive, Suite 204,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced,
distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO
PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

=========================================================================*/
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

#include "vtkADHExporterOperatorBase.h"
#include "cmbSystemConfig.h"

class vtkSMProxy;

class VTK_EXPORT vtkADHExporterOperatorClient : public vtkADHExporterOperatorBase
{
public:
  static vtkADHExporterOperatorClient * New();
  vtkTypeMacro(vtkADHExporterOperatorClient,vtkADHExporterOperatorBase);
  void PrintSelf(ostream& os, vtkIndent indent);


  // Description:
  // Get/Set the text that the client generated
  vtkSetStringMacro(ClientText);
  vtkGetStringMacro(ClientText);

  using Superclass::Operate;
//BTX
  // Description:
  virtual bool Operate(vtkDiscreteModel* model, vtkSMProxy* serverModelProxy);
//ETX

protected:
  vtkADHExporterOperatorClient();
  virtual ~vtkADHExporterOperatorClient();

  char *ClientText;

private:
  vtkADHExporterOperatorClient(const vtkADHExporterOperatorClient&);  // Not implemented.
  void operator=(const vtkADHExporterOperatorClient&);  // Not implemented.
};

#endif
