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
