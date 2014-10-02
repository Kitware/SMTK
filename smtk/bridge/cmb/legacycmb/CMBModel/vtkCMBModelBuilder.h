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
// .NAME vtkCMBModelBuilder - A CMB model builder object
// .SECTION Description
// This CMB Model builder takes a vtkPolyData as input, then parse and convert
// all the topology and geometry info from the input to fill in a CMB model.

#ifndef __vtkCMBModelBuilder_h
#define __vtkCMBModelBuilder_h

#include "vtkCmbDiscreteModelModule.h" // For export macro
#include "vtkObject.h"
#include "cmbSystemConfig.h"

class vtkCellLocator;
class vtkDiscreteModelRegion;
class vtkDiscreteModelWrapper;
class vtkPolyData;
class vtkAlgorithm;
class vtkIdList;
class vtkIntArray;

class VTKCMBDISCRETEMODEL_EXPORT vtkCMBModelBuilder : public vtkObject
{
public:
  static vtkCMBModelBuilder * New();
  vtkTypeMacro(vtkCMBModelBuilder,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Load the input polydata into Model.
  void Operate(vtkDiscreteModelWrapper* modelWrapper,
    vtkAlgorithm* inputPoly);

  // Description:
  // Returns success (1) or failue (0) for Operation.
  vtkGetMacro(OperateSucceeded, int);

protected:
  vtkCMBModelBuilder();
  virtual ~vtkCMBModelBuilder();

private:

  // Description:
  // Internal ivars.
  char* FileName;
  vtkCMBModelBuilder(const vtkCMBModelBuilder&);  // Not implemented.
  void operator=(const vtkCMBModelBuilder&);  // Not implemented.

  void ProcessAs2DMesh(vtkDiscreteModelWrapper* ModelWrapper, vtkPolyData *modelPolyData);

  void ComputePointInsideForRegion(vtkDiscreteModelRegion *region,
    vtkCellLocator *locator);

  // Description:
  // Flag to indicate that the operation on the model succeeded (1) or not (0).
  int OperateSucceeded;
};

#endif
