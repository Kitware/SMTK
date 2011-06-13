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
// .NAME vtkCmbMeshGridRepresentationClient
// .SECTION Description
// Operator that reads in the simulation mesh on the server and creates a
// vtkCmbMeshGridRepresentation to be used by the model.


#ifndef __vtkCmbMeshGridRepresentationClient_h
#define __vtkCmbMeshGridRepresentationClient_h

#include "vtkObject.h"

class vtkCMBModel;
class vtkSMProxy;

class VTK_EXPORT vtkCmbMeshGridRepresentationClient : public vtkObject
{
public:
  static vtkCmbMeshGridRepresentationClient * New();
  vtkTypeRevisionMacro(vtkCmbMeshGridRepresentationClient,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Reads in the file on the server. Returns true if the operation was successful.
  bool Operate(vtkSMProxy* serverMeshProxy);

protected:
  vtkCmbMeshGridRepresentationClient();
  virtual ~vtkCmbMeshGridRepresentationClient();

private:
  vtkCmbMeshGridRepresentationClient(const vtkCmbMeshGridRepresentationClient&);  // Not implemented.
  void operator=(const vtkCmbMeshGridRepresentationClient&);  // Not implemented.
};

#endif
