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
#include "vtkModelUniqueNodalGroup.h"

#include "vtkBitArray.h"
#include "vtkCellArray.h"
#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelFace.h"
#include "vtkExtractEdges.h"
#include "vtkIdList.h"
#include "vtkModelItemIterator.h"
#include "vtkObjectFactory.h"
#include "vtkPointSet.h"
#include "vtkPolyData.h"
#include "vtkSerializer.h"

#include <set>

struct vtkModelUniqueNodalGroupInternals
{
  typedef std::set<vtkIdType> Set;
  typedef Set::iterator SetIterator;
  Set PointIds;
};

vtkCxxRevisionMacro(vtkModelUniqueNodalGroup, "");

vtkModelUniqueNodalGroup* vtkModelUniqueNodalGroup::New()
{
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkModelUniqueNodalGroup");
  if(ret)
    {
    return static_cast<vtkModelUniqueNodalGroup*>(ret);
    }
  return new vtkModelUniqueNodalGroup;
}

vtkModelUniqueNodalGroup::vtkModelUniqueNodalGroup()
{
}

vtkModelUniqueNodalGroup::~vtkModelUniqueNodalGroup()
{
}

void vtkModelUniqueNodalGroup::AddPointId(vtkIdType pointId)
{
  vtkDiscreteModel* model = this->GetModel();
  // DiscreteModel will remove pointId from the old unique nodal group
  model->SetPointUniqueNodalGroup(this, pointId);

  this->Superclass::AddPointId(pointId);
}

void vtkModelUniqueNodalGroup::RemovePointId(vtkIdType pointId)
{
  // DiscreteModel will remove pointId from the old unique nodal group
  vtkDiscreteModel* model = this->GetModel();
  model->SetPointUniqueNodalGroup(0, pointId);
}

void vtkModelUniqueNodalGroup::Serialize(vtkSerializer* ser)
{
  this->Superclass::Serialize(ser);
}

bool vtkModelUniqueNodalGroup::Destroy()
{
  // remove all points first from here so that the model doesn't have
  // anything in.
  vtkIdList* pointIds = vtkIdList::New();
  this->GetPointIds(pointIds);
  // if the performance of this is too slow we could get the model once
  // and just call Model->SetPointUniqueNodalGroup() for each id in pointIds.
  // I'm doing it this way for now to be clearer.
  this->RemovePointIds(pointIds);
  pointIds->Delete();

  return 1;
}

void vtkModelUniqueNodalGroup::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
