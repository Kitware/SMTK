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
// .NAME vtkModelEdgeMeshOperator - Operate on model edge mesh on server.
// .SECTION Description

#ifndef __vtkModelEdgeMeshOperator_h
#define __vtkModelEdgeMeshOperator_h

#include "vtkObject.h"

class vtkCMBModelWrapper;

class VTK_EXPORT vtkModelEdgeMeshOperator : public vtkObject
{
public:
  static vtkModelEdgeMeshOperator * New();
  vtkTypeRevisionMacro(vtkModelEdgeMeshOperator,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Modify the color, user name, and/or the visibility of an object.
  virtual void Operate(vtkCMBModelWrapper* modelWrapper);

  // Description:
  // Get/set the unique persistent id of the model edge.
  // 0 indicates it is not set.
  vtkSetMacro(UniquePersistentId, vtkIdType);
  vtkGetMacro(UniquePersistentId, vtkIdType);

  // Description:
  // The desired length of the model edge.  0 indicates not set.
  vtkGetMacro(Length, double);
  vtkSetMacro(Length, double);

  // Description:
  // Returns success (1) or failue (0) for Operation.
  vtkGetMacro(OperateSucceeded, int);

protected:
  vtkModelEdgeMeshOperator();
  virtual ~vtkModelEdgeMeshOperator();

private:
  vtkModelEdgeMeshOperator(const vtkModelEdgeMeshOperator&);  // Not implemented.
  void operator=(const vtkModelEdgeMeshOperator&);  // Not implemented.

  // Description:
  // Flag to indicate that the operation on the model succeeded (1) or not (0).
  int OperateSucceeded;

  vtkIdType UniquePersistentId;
  double Length;
};

#endif
