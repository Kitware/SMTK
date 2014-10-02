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
