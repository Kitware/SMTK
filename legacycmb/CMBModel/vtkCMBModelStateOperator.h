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
// .NAME vtkCMBModelStateOperator - The CMB model state operator class
// .SECTION Description
//  This class handles saving and reloading of the state of a CMB model.
//  The saving of the model will serialize the model into a string,
//  and reloading of the model involves deserialize the string back into
//  the model.

#ifndef __vtkCMBModelStateOperator_h
#define __vtkCMBModelStateOperator_h

#include "vtkCmbDiscreteModelModule.h" // For export macro
#include "vtkCMBModelStateOperatorBase.h"
#include "vtkSmartPointer.h"
#include "cmbSystemConfig.h"

class vtkStringArray;
class vtkIdList;
class vtkDiscreteModelWrapper;
class vtkProperty;

class VTKCMBDISCRETEMODEL_EXPORT vtkCMBModelStateOperator : public vtkCMBModelStateOperatorBase
{
public:
  static vtkCMBModelStateOperator *New();
  vtkTypeMacro(vtkCMBModelStateOperator,vtkCMBModelStateOperatorBase);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Common API of model operator classes.
  virtual void Operate(vtkDiscreteModelWrapper *cmbModelWrapper);

  // Description:
  // Macro to set/get OperatorMode
  // 0, Save model state
  // 1, Load model state
  vtkSetClampMacro(OperatorMode, int, 0, 1);
  vtkGetMacro(OperatorMode, int);

  // Description:
  // Returns success (1) or failue (0) for Operation.
  vtkGetMacro(OperateSucceeded, int);

//BTX
protected:
  vtkCMBModelStateOperator();
  ~vtkCMBModelStateOperator();

  using Superclass::AbleToOperate;

  // Description:
  // Check to see if everything is properly set for the operator.
  virtual bool AbleToOperate(vtkDiscreteModelWrapper* ModelWrapper);

  // Description:
  // Save or reload the state for the model
  int SaveState(vtkDiscreteModelWrapper *modelWrapper);
  int LoadSavedState(vtkDiscreteModelWrapper *modelWrapper);

  // Description:
  // Flag to indicate that the operation on the model succeeded (1) or not (0).
  int OperateSucceeded;

  // Description:
  // Model Face and Model Edge mapping
  std::map<vtkIdType, vtkSmartPointer<vtkIdList> > FaceToIds;
  std::map<vtkIdType, vtkSmartPointer<vtkIdList> > EdgeToIds;
  // The Vertex for 2D
  std::map<vtkIdType, vtkIdType > VertexToIds;
  // The Display property, because it is not a serializable object
  std::map<vtkIdType, vtkSmartPointer<vtkProperty> > EntityToProperties;

  int OperatorMode; // 0, saveMode; 1; reload state

private:
  vtkCMBModelStateOperator(const vtkCMBModelStateOperator&);  // Not implemented.
  void operator=(const vtkCMBModelStateOperator&);  // Not implemented.

//ETX
};

#endif
