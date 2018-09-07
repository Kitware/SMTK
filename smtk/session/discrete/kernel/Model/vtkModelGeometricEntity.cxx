//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkModelGeometricEntity.h"

#include "vtkInformation.h"
#include "vtkInformationObjectBaseKey.h"
#include "vtkModel.h"
#include "vtkModelItemIterator.h"
#include "vtkOpenGLProperty.h"
#include "vtkPolyData.h"

vtkInformationKeyMacro(vtkModelGeometricEntity, GEOMETRY, ObjectBase);

vtkModelGeometricEntity::vtkModelGeometricEntity()
{
  this->InitDefaultDisplayProperty();
  this->SetPickable(1);
  this->SetShowTexture(1);
}

vtkModelGeometricEntity::~vtkModelGeometricEntity()
{
  // do we need to remove BoundaryRep from the informationobject?
  this->SetDisplayProperty(0);
}

void vtkModelGeometricEntity::SetGeometry(vtkObject* geometry)
{
  this->GetProperties()->Set(GEOMETRY(), geometry);
  this->Modified();
  this->GetModel()->InvokeModelGeometricEntityEvent(ModelEntityGeometrySet, this);
}

vtkObject* vtkModelGeometricEntity::GetGeometry()
{
  vtkObject* object = vtkObject::SafeDownCast(this->GetProperties()->Get(GEOMETRY()));
  return object;
}

void vtkModelGeometricEntity::InitDefaultDisplayProperty()
{
  vtkOpenGLProperty* prop = vtkOpenGLProperty::New();
  prop->SetBackfaceCulling(0);
  prop->SetFrontfaceCulling(0);
  this->SetDisplayProperty(prop);
  prop->Delete();
}

void vtkModelGeometricEntity::SetDisplayProperty(vtkProperty* prop)
{
  this->GetProperties()->Set(DISPLAY_PROPERTY(), prop);
  this->Modified();
}

vtkProperty* vtkModelGeometricEntity::GetDisplayProperty()
{
  vtkProperty* object = vtkProperty::SafeDownCast(this->GetProperties()->Get(DISPLAY_PROPERTY()));
  return object;
}

void vtkModelGeometricEntity::SetPickable(int pickable)
{
  if (this->GetProperties()->Has(PICKABLE()) && pickable == this->GetProperties()->Get(PICKABLE()))
  {
    return;
  }
  this->GetProperties()->Set(PICKABLE(), pickable);
  this->Modified();
}

int vtkModelGeometricEntity::GetPickable()
{
  if (this->GetProperties()->Has(PICKABLE()))
  {
    return this->GetProperties()->Get(PICKABLE());
  }
  return 1;
}

void vtkModelGeometricEntity::SetShowTexture(int show)
{
  if (this->GetProperties()->Has(SHOWTEXTURE()) &&
    show == this->GetProperties()->Get(SHOWTEXTURE()))
  {
    return;
  }
  this->GetProperties()->Set(SHOWTEXTURE(), show);
  this->Modified();
}

int vtkModelGeometricEntity::GetShowTexture()
{
  if (this->GetProperties()->Has(SHOWTEXTURE()))
  {
    return this->GetProperties()->Get(SHOWTEXTURE());
  }
  return 0;
}

bool vtkModelGeometricEntity::GetBounds(double bounds[6])
{
  if (vtkPolyData* entityPoly = vtkPolyData::SafeDownCast(this->GetGeometry()))
  {
    entityPoly->GetBounds(bounds);
    return true;
  }
  return false;
}

vtkModel* vtkModelGeometricEntity::GetModel()
{
  vtkModelItemIterator* iter = this->NewIterator(vtkModelType);
  iter->Begin();
  vtkModel* model = vtkModel::SafeDownCast(iter->GetCurrentItem());
  iter->Delete();
  return model;
}

void vtkModelGeometricEntity::Serialize(vtkSerializer* ser)
{
  this->Superclass::Serialize(ser);
}

void vtkModelGeometricEntity::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "Representation : " << this->GetGeometry() << "\n";
}
