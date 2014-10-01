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
// .NAME vtkCMBModelReadOperator -
// .SECTION Description
// Front end for the readers.  Reads in a vtkPolyData and then figures
// out how to parse that vtkPolyData.

#ifndef __smtkcmb_vtkCMBModelReadOperator_h
#define __smtkcmb_vtkCMBModelReadOperator_h

#include "vtkSMTKCMBModelModule.h" // For export macro
#include "vtkObject.h"


class vtkCMBParserBase;
class vtkDiscreteModelWrapper;
class vtkPolyData;
class vtkDiscreteModel;

class VTKSMTKCMBMODEL_EXPORT vtkCMBModelReadOperator : public vtkObject
{
public:
  static vtkCMBModelReadOperator * New();
  vtkTypeMacro(vtkCMBModelReadOperator,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Load the file into Model.
  void Operate(vtkDiscreteModelWrapper* ModelWrapper);

//BTX
  // Description:
  // Load the file into Model.
  void Read(vtkDiscreteModel* model);
//ETX

  // Description:
  // Get/Set the name of the input file.
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

  // Description:
  // Returns success (1) or failue (0) for Operation.
  vtkGetMacro(OperateSucceeded, int);

  // Description:
  // Get the string used to reference the field data array that the
  // file version is stored in.
  static const char* GetCMBFileVersionString();

protected:
  vtkCMBModelReadOperator();
  virtual ~vtkCMBModelReadOperator();

  vtkCMBParserBase* NewParser(vtkPolyData* MasterPoly);

private:
  // Description:
  // The name of the file to be read in.
  char* FileName;

  vtkCMBModelReadOperator(const vtkCMBModelReadOperator&);  // Not implemented.
  void operator=(const vtkCMBModelReadOperator&);  // Not implemented.

  // Description:
  // Flag to indicate that the operation on the model succeeded (1) or not (0).
  int OperateSucceeded;
};

#endif
