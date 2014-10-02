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
// .NAME vtkEdgeSplitOperatorBase - Split a model edge
// .SECTION Description
// Operator to split a model edge given a point id for the grid.  Note
// that the master polydata and the model edge polydata share the
// same point set so there is no concern about getting the point Ids
// confused.

#ifndef __vtkEdgeSplitOperatorBase_h
#define __vtkEdgeSplitOperatorBase_h

#include "vtkCmbDiscreteModelModule.h" // For export macro
#include "vtkObject.h"
#include "cmbSystemConfig.h"

class vtkDiscreteModel;
class vtkModelEntity;

class VTKCMBDISCRETEMODEL_EXPORT vtkEdgeSplitOperatorBase : public vtkObject
{
public:
  static vtkEdgeSplitOperatorBase * New();
  vtkTypeMacro(vtkEdgeSplitOperatorBase,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set/get the model entity unique persistent id to operate on.
  // This is required to be set before calling Operate.
  vtkGetMacro(IsEdgeIdSet, int);
  vtkGetMacro(EdgeId, vtkIdType);
  void SetEdgeId(vtkIdType id);

  // Description:
  // Set/get the point Id of where the model edge should be split at.
  // This is required to be set before calling Operate.
  vtkGetMacro(IsPointIdSet, int);
  vtkGetMacro(PointId, vtkIdType);
  void SetPointId(vtkIdType id);

//BTX
  // Description:
  // Return the model entity.
  vtkModelEntity* GetModelEntity(vtkDiscreteModel* Model);
//ETX

  // Description:
  // Get the Id of the created model edge during a split operation.
  vtkGetMacro(CreatedModelEdgeId, vtkIdType);

  // Description:
  // Get the Id of the created model vertex during a split operation.
  vtkGetMacro(CreatedModelVertexId, vtkIdType);

protected:
  vtkEdgeSplitOperatorBase();
  virtual ~vtkEdgeSplitOperatorBase();

  // Description:
  // Set function for the created model edge id.  It is protected
  // since only the derived class should modify it.
  vtkSetMacro(CreatedModelEdgeId, vtkIdType);

  // Description:
  // Set function for the created model vertex id.  It is protected
  // since only the derived class should modify it.
  vtkSetMacro(CreatedModelVertexId, vtkIdType);

  // Description:
  // Check to see if everything is properly set for the operator.
  virtual bool AbleToOperate(vtkDiscreteModel* Model);

private:
  // Description:
  // The unique persistent Id of the model edge created (-1 if
  // no model edge was created).
  vtkIdType CreatedModelEdgeId;

  // Description:
  // The unique persistent Id of the model vertex created (-1 if
  // no model vertex was created).
  vtkIdType CreatedModelVertexId;

  // Description:
  // The unique persistent id of the model entity to be operated on.
  // This is required to be set before calling Operate.
  vtkIdType EdgeId;
  int IsEdgeIdSet;

  // Description:
  // The point id of the polydata where the model edge will be split.
  // This is required to be set before calling Operate.
  vtkIdType PointId;
  int IsPointIdSet;

  vtkEdgeSplitOperatorBase(const vtkEdgeSplitOperatorBase&);  // Not implemented.
  void operator=(const vtkEdgeSplitOperatorBase&);  // Not implemented.
};

#endif
