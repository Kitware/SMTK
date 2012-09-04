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
#include "vtkModelGeometricEntity.h"

#include "vtkInformation.h"
#include "vtkInformationObjectBaseKey.h"
#include "vtkModel.h"
#include "vtkModelItemIterator.h"
#include "vtkPolyData.h"
#include "vtkOpenGLProperty.h"


vtkModelGeometricEntity::vtkModelGeometricEntity()
{
  this->InitDefaultDisplayProperty();
  this->SetPickable(1);
}

vtkModelGeometricEntity::~vtkModelGeometricEntity()
{
  // do we need to remove BoundaryRep from the informationobject?
  this->SetDisplayProperty(0);
}

void vtkModelGeometricEntity::SetGeometry(vtkObject* Geometry)
{
  this->GetProperties()->Set(GEOMETRY(), Geometry);
  this->Modified();
  this->GetModel()->InvokeModelGeometricEntityEvent(
    ModelEntityGeometrySet, this);
}

vtkObject* vtkModelGeometricEntity::GetGeometry()
{
  vtkObject* object = vtkObject::SafeDownCast(
    this->GetProperties()->Get(GEOMETRY()));
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
  vtkProperty* object = vtkProperty::SafeDownCast(
    this->GetProperties()->Get(DISPLAY_PROPERTY()));
  return object;
}

void vtkModelGeometricEntity::SetPickable(int pickable)
{
  if(this->GetProperties()->Has(PICKABLE()) &&
    pickable == this->GetProperties()->Get(PICKABLE()))
    {
    return;
    }
  this->GetProperties()->Set(PICKABLE(), pickable);
  this->Modified();
}

int vtkModelGeometricEntity::GetPickable()
{
  if(this->GetProperties()->Has(PICKABLE()))
    {
    return this->GetProperties()->Get(PICKABLE());
    }
  return 1;
}

bool vtkModelGeometricEntity::GetBounds(double bounds[6])
{
  if(vtkPolyData* entityPoly =
     vtkPolyData::SafeDownCast(this->GetGeometry()))
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
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Representation : " << this->GetGeometry() << "\n";
}
