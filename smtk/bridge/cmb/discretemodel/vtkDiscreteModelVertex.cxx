//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkDiscreteModelVertex.h"

#include "vtkDiscreteModel.h"
#include "vtkInformation.h"
#include "vtkInformationIdTypeKey.h"
#include "vtkModel.h"
#include "vtkModelItemIterator.h"
#include "vtkModelVertexUse.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"
#include "vtkProperty.h"

vtkInformationKeyMacro(vtkDiscreteModelVertex, POINTID, IdType);

vtkDiscreteModelVertex* vtkDiscreteModelVertex::New()
{
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkDiscreteModelVertex");
  if(ret)
    {
    return static_cast<vtkDiscreteModelVertex*>(ret);
    }
  return new vtkDiscreteModelVertex;
}

vtkDiscreteModelVertex::vtkDiscreteModelVertex()
{
  this->GetProperties()->Set(POINTID(), -1);
}

vtkDiscreteModelVertex::~vtkDiscreteModelVertex()
{
}

bool vtkDiscreteModelVertex::GetPoint(double* xyz)
{
  vtkDiscreteModel* model = vtkDiscreteModel::SafeDownCast(this->GetModel());
  if(model && model->HasValidMesh())
    {
    const DiscreteMesh& mesh = model->GetMesh();
    vtkIdType pointId = this->GetPointId();
    if(pointId >= 0 && pointId < mesh.GetNumberOfPoints())
      {
      mesh.GetPoint(pointId, xyz);
      return true;
      }
    else
      {
      return false;
      }
    }
  return false;
}

vtkIdType vtkDiscreteModelVertex::GetPointId()
{
  return this->GetProperties()->Get(POINTID());
}

void vtkDiscreteModelVertex::SetPointId(vtkIdType pointId)
{
  if(pointId != this->GetPointId())
    {
    this->GetProperties()->Set(POINTID(), pointId);
    //this->CreateGeometry(pointId);
    }
}

void vtkDiscreteModelVertex::CreateGeometry()
{
  // if already has geometry, just return;
  if(this->GetGeometry())
    {
    return;
    }
  vtkDiscreteModel* model  = vtkDiscreteModel::SafeDownCast(this->GetModel());
  if(model->HasValidMesh())
    {
    const DiscreteMesh& mesh = model->GetMesh();
    vtkIdType pointId = this->GetPointId();
    if(pointId >= 0 && pointId < mesh.GetNumberOfPoints())
      {
      vtkPolyData* poly = vtkPolyData::New();
      poly->SetPoints(mesh.SharePointsPtr());
      poly->Allocate(1);
      poly->InsertNextCell(VTK_VERTEX, 1, &pointId);

      this->SetGeometry(poly);

      if(!this->GetDisplayProperty())
        {
        this->InitDefaultDisplayProperty();
        }
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

void vtkDiscreteModelVertex::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
