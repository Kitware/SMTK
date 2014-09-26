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
// .NAME vtkGenerateSimpleModelOperator - create a simple model from polydata
// .SECTION Description
// Create a 2D model from the output from a filter that produces
// polydata. By default we assume that the model may have quads and/or
// bad topology (i.e. coincident points) so we use the triangle
// and clean polydata filter to "fix" it. The user can set CleanInput
// to 0 to avoid this overhead if the incoming polydata is "good".

#ifndef __vtkGenerateSimpleModelOperator_h
#define __vtkGenerateSimpleModelOperator_h

#include "vtkCmbDiscreteModelModule.h" // For export macro
#include "vtkObject.h"
#include "cmbSystemConfig.h"

class vtkAlgorithm;
class vtkDiscreteModelWrapper;

class VTKCMBDISCRETEMODEL_EXPORT vtkGenerateSimpleModelOperator : public vtkObject
{
public:
  static vtkGenerateSimpleModelOperator * New();
  vtkTypeMacro(vtkGenerateSimpleModelOperator,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Create a model from the output from inputFilter. If cleanInput is
  // true then we also triangulate and remove coincident points.
  void Operate(vtkDiscreteModelWrapper* modelWrapper, vtkAlgorithm* inputFilter,
               int cleanInput);

  // Description:
  // Returns success (1) or failue (0) for Operation.
  vtkGetMacro(OperateSucceeded, int);

protected:
  vtkGenerateSimpleModelOperator();
  virtual ~vtkGenerateSimpleModelOperator();

private:
  vtkGenerateSimpleModelOperator(const vtkGenerateSimpleModelOperator&);  // Not implemented.
  void operator=(const vtkGenerateSimpleModelOperator&);  // Not implemented.

  // Description:
  // Flag to indicate that the operation on the model succeeded (1) or not (0).
  int OperateSucceeded;
};

#endif
