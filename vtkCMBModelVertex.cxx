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
#include "vtkCMBModelVertex.h"

#include "vtkDiscreteModel.h"
#include "vtkInformation.h"
#include "vtkInformationIdTypeKey.h"
#include "vtkModel.h"
#include "vtkModelItemIterator.h"
#include "vtkModelVertexUse.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"
#include "vtkProperty.h"

vtkCxxRevisionMacro(vtkCMBModelVertex, "");
vtkInformationKeyMacro(vtkCMBModelVertex, POINTID, IdType);

vtkCMBModelVertex* vtkCMBModelVertex::New()
{
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkCMBModelVertex");
  if(ret)
    {
    return static_cast<vtkCMBModelVertex*>(ret);
    }
  return new vtkCMBModelVertex;
}

vtkCMBModelVertex::vtkCMBModelVertex()
{
  this->GetProperties()->Set(POINTID(), -1);
}

vtkCMBModelVertex::~vtkCMBModelVertex()
{
}

bool vtkCMBModelVertex::GetPoint(double* xyz)
{
  if(vtkPolyData* masterPoly = vtkPolyData::SafeDownCast(
       vtkDiscreteModel::SafeDownCast(this->GetModel())->GetGeometry()))
    {
    vtkIdType pointId = this->GetPointId();
    if(pointId >= 0 && pointId < masterPoly->GetNumberOfPoints())
      {
      masterPoly->GetPoint(pointId, xyz);
      return true;
      }
    else
      {
      return false;
      }
    }
  return false;
}

vtkIdType vtkCMBModelVertex::GetPointId()
{
  return this->GetProperties()->Get(POINTID());
}

void vtkCMBModelVertex::SetPointId(vtkIdType pointId)
{
  if(pointId != this->GetPointId())
    {
    this->GetProperties()->Set(POINTID(), pointId);
    //this->CreateGeometry(pointId);
    }
}

void vtkCMBModelVertex::CreateGeometry()
{
  // if already has geometry, just return;
  if(this->GetGeometry())
    {
    return;
    }
  if(vtkPolyData* masterPoly = vtkPolyData::SafeDownCast(
    vtkDiscreteModel::SafeDownCast(this->GetModel())->GetGeometry()))
    {
    vtkIdType pointId = this->GetPointId();
    if(pointId >= 0 && pointId < masterPoly->GetNumberOfPoints())
      {
      vtkPolyData* poly = vtkPolyData::New();
      poly->SetPoints(masterPoly->GetPoints());
      poly->Allocate(1);
      poly->InsertNextCell(VTK_VERTEX, 1, &pointId);
      this->SetGeometry(poly);
      vtkProperty* displayProp = this->GetDisplayProperty();
      displayProp->SetPointSize(8.0);
      displayProp->SetColor(0.0, 1.0, 0.0);
      this->SetColor(0.0, 1.0, 0.0, 1.0);
      poly->Delete();
      }
    else
      {
      vtkWarningMacro("Bad point Id for model vertex.");
      }
    }
}

void vtkCMBModelVertex::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
