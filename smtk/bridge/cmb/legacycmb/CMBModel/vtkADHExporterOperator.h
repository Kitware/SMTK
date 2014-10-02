//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkADHExporterOperator - Write the nodal and face boundary conditions.
// .SECTION Description
// Operator to append the nodal and face boundary conditions for
// the ADH file that is getting exported from SimBuilder.  This operator
// only appends the nodal (NDS) and face (FCS) boundary condition
// cards to the file for each node and face in the grid.  Objects of this
// class should only be created on the server.

#ifndef __vtkADHExporterOperator_h
#define __vtkADHExporterOperator_h

#include "vtkCmbDiscreteModelModule.h" // For export macro
#include "vtkADHExporterOperatorBase.h"
#include "cmbSystemConfig.h"

class vtkDiscreteModelWrapper;

class VTKCMBDISCRETEMODEL_EXPORT vtkADHExporterOperator : public vtkADHExporterOperatorBase
{
public:
  static vtkADHExporterOperator * New();
  vtkTypeMacro(vtkADHExporterOperator,vtkADHExporterOperatorBase);
  void PrintSelf(ostream& os, vtkIndent indent);

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
  vtkADHExporterOperator();
  virtual ~vtkADHExporterOperator();

  using Superclass::AbleToOperate;

  // Description:
  // Check to see if everything is properly set for the operator.
  virtual bool AbleToOperate(vtkDiscreteModelWrapper* ModelWrapper);

  char *ClientText;

private:
  vtkADHExporterOperator(const vtkADHExporterOperator&);  // Not implemented.
  void operator=(const vtkADHExporterOperator&);  // Not implemented.

  // Description:
  // Flag to indicate that the operation on the model succeeded (1) or not (0).
  int OperateSucceeded;

};

#endif
