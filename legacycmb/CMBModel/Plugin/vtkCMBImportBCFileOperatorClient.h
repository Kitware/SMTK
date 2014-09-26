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
// .NAME vtkCMBImportBCFileOperatorClient - Imports the BC file info.
// .SECTION Description
// Operator that reads in a BC file on the server from the client and creates a
// vtkCmbBCGridRepresentation to be used by the model.  Currently
// the BC file only has enough information to work properly for the
// boundary groups that existed when the volumetric mesh was generated.

#ifndef __vtkCMBImportBCFileOperatorClient_h
#define __vtkCMBImportBCFileOperatorClient_h

#include "vtkObject.h"
#include "cmbSystemConfig.h"

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
