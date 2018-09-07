//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkADHExporterOperation - Write the nodal and face boundary conditions.
// .SECTION Description
// Operation to append the nodal and face boundary conditions for
// the ADH file that is getting exported from SimBuilder.  This operator
// only appends the nodal (NDS) and face (FCS) boundary condition
// cards to the file for each node and face in the grid.  Objects of this
// class should only be created on the server.

#ifndef __vtkADHExporterOperation_h
#define __vtkADHExporterOperation_h

#include "cmbSystemConfig.h"
#include "vtkADHExporterOperationBase.h"
#include "vtkCmbDiscreteModelModule.h" // For export macro

class vtkDiscreteModelWrapper;

class VTKCMBDISCRETEMODEL_EXPORT vtkADHExporterOperation : public vtkADHExporterOperationBase
{
public:
  static vtkADHExporterOperation* New();
  vtkTypeMacro(vtkADHExporterOperation, vtkADHExporterOperationBase);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  using Superclass::Operate;

  virtual void Operate(vtkDiscreteModelWrapper* modelWrapper);

  // Description:
  // Returns success (1) or failue (0) for Operation.
  vtkGetMacro(OperateSucceeded, int);

  // Description:
  // Get/Set the text that the client generated
  vtkSetStringMacro(ClientText);
  vtkGetStringMacro(ClientText);

protected:
  vtkADHExporterOperation();
  virtual ~vtkADHExporterOperation();

  using Superclass::AbleToOperate;

  // Description:
  // Check to see if everything is properly set for the operator.
  virtual bool AbleToOperate(vtkDiscreteModelWrapper* ModelWrapper);

  char* ClientText;

private:
  vtkADHExporterOperation(const vtkADHExporterOperation&); // Not implemented.
  void operator=(const vtkADHExporterOperation&);          // Not implemented.

  // Description:
  // Flag to indicate that the operation on the model succeeded (1) or not (0).
  int OperateSucceeded;
};

#endif
