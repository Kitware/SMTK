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
// .NAME vtkCmbMeshRepresentationOperator
// .SECTION Description
// Operator that reads in the simulation mesh on the server and creates a
// vtkCmbMeshGridRepresentation to be used by the model.

#ifndef __vtkCmbMeshRepresentationOperator_h
#define __vtkCmbMeshRepresentationOperator_h

#include "vtkObject.h"

class vtkCMBMeshWrapper;

class VTK_EXPORT vtkCmbMeshRepresentationOperator : public vtkObject
{
public:
  static vtkCmbMeshRepresentationOperator * New();
  vtkTypeRevisionMacro(vtkCmbMeshRepresentationOperator,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Sets OperateSucceeded.
  void Operate(vtkCMBMeshWrapper* meshWrapper);

  // Description:
  // Returns success (1) or failue (0) for Operation.
  vtkGetMacro(OperateSucceeded, int);

protected:
  vtkCmbMeshRepresentationOperator();
  virtual ~vtkCmbMeshRepresentationOperator();

private:

  vtkCmbMeshRepresentationOperator(const vtkCmbMeshRepresentationOperator&);  // Not implemented.
  void operator=(const vtkCmbMeshRepresentationOperator&);  // Not implemented.

  // Description:
  // Flag to indicate that the operation on the model succeeded (1) or not (0).
  int OperateSucceeded;
};

#endif
