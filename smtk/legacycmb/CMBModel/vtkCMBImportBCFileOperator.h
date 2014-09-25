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
// .NAME vtkCMBImportBCFileOperator -Imports the BC file info.
// .SECTION Description
// Operator that reads in a BC file on the server and creates a
// vtkModelBCGridRepresentation to be used by the model.  Currently
// the BC file only has enough information to work properly for the
// boundary groups that existed when the volumetric mesh was generated.

#ifndef __vtkCMBImportBCFileOperator_h
#define __vtkCMBImportBCFileOperator_h

#include "vtkCmbDiscreteModelModule.h" // For export macro
#include "vtkObject.h"
#include "cmbSystemConfig.h"

class vtkDiscreteModelWrapper;

class VTKCMBDISCRETEMODEL_EXPORT vtkCMBImportBCFileOperator : public vtkObject
{
public:
  static vtkCMBImportBCFileOperator * New();
  vtkTypeMacro(vtkCMBImportBCFileOperator,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Reads in the file assuming we're on the server.
  // Sets OperateSucceeded.
  void Operate(vtkDiscreteModelWrapper* modelWrapper);

  // Description:
  // Get/Set the name of the input file.
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

  // Description:
  // Returns success (1) or failue (0) for Operation.
  vtkGetMacro(OperateSucceeded, int);

protected:
  vtkCMBImportBCFileOperator();
  virtual ~vtkCMBImportBCFileOperator();

private:
  // Description:
  // The name of the file to be read.
  char* FileName;

  vtkCMBImportBCFileOperator(const vtkCMBImportBCFileOperator&);  // Not implemented.
  void operator=(const vtkCMBImportBCFileOperator&);  // Not implemented.

  // Description:
  // Flag to indicate that the operation on the model succeeded (1) or not (0).
  int OperateSucceeded;
};

#endif
