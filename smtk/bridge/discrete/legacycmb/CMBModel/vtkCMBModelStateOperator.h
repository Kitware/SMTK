//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

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

};

#endif
