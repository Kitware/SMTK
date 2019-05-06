//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkModelEntityOperationBase.h"

#include "smtk/session/discrete/kernel/Model/vtkModelGeometricEntity.h"
#include "vtkDiscreteModel.h"
#include "vtkModelUserName.h"
#include "vtkObjectFactory.h"
#include "vtkProperty.h"
#include "vtkSmartPointer.h"

vtkStandardNewMacro(vtkModelEntityOperationBase);

vtkModelEntityOperationBase::vtkModelEntityOperationBase()
{
  this->Id = -1000;
  this->IsIdSet = 0;
  this->ItemType = -1;
  this->IsItemTypeSet = 0;
  this->Visibility = -1;
  this->IsVisibilitySet = 0;
  for (int i = 0; i < 4; i++)
  {
    this->RGBA[i] = -10.0;
  }
  this->IsRGBASet = 0;
  for (int i = 0; i < 4; i++)
  {
    this->RepresentationRGBA[i] = -10.0;
  }
  this->IsRepresentationRGBASet = 0;
  this->UserName = 0;
  this->IsPickableSet = 0;
  this->Pickable = -1;
  this->IsShowTextureSet = 0;
  this->ShowTexture = -1;
}

vtkModelEntityOperationBase::~vtkModelEntityOperationBase()
{
  this->SetUserName(0);
}

void vtkModelEntityOperationBase::SetId(vtkIdType id)
{
  if (id != this->Id)
  {
    this->Modified();
    this->Id = id;
    this->IsIdSet = 1;
  }
}

void vtkModelEntityOperationBase::SetItemType(int itemType)
{
  if (itemType != this->ItemType)
  {
    this->Modified();
    this->ItemType = itemType;
    this->IsItemTypeSet = 1;
  }
}

void vtkModelEntityOperationBase::SetVisibility(int visibility)
{
  if (this->Visibility != visibility)
  {
    this->Modified();
    this->Visibility = visibility;
    this->IsVisibilitySet = 1;
  }
}

void vtkModelEntityOperationBase::SetPickable(int pickable)
{
  if (this->Pickable != pickable)
  {
    this->Modified();
    this->Pickable = pickable;
    this->IsPickableSet = 1;
  }
}

void vtkModelEntityOperationBase::SetShowTexture(int show)
{
  if (this->ShowTexture != show)
  {
    this->Modified();
    this->ShowTexture = show;
    this->IsShowTextureSet = 1;
  }
}

void vtkModelEntityOperationBase::SetRGBA(double R, double G, double B, double A)
{
  double rgba[4] = { R, G, B, A };
  this->SetRGBA(rgba);
}

void vtkModelEntityOperationBase::SetRGBA(double* rgba)
{
  for (int i = 0; i < 4; i++)
  {
    if (rgba[i] != this->RGBA[i])
    {
      this->Modified();
      this->RGBA[i] = rgba[i];
      this->IsRGBASet = 1;
    }
  }
}
void vtkModelEntityOperationBase::SetRepresentationRGBA(double R, double G, double B, double A)
{
  double rgba[4] = { R, G, B, A };
  this->SetRepresentationRGBA(rgba);
}

void vtkModelEntityOperationBase::SetRepresentationRGBA(double* rgba)
{
  for (int i = 0; i < 4; i++)
  {
    if (rgba[i] != this->RepresentationRGBA[i])
    {
      this->RepresentationRGBA[i] = rgba[i];
      this->Modified();
      this->IsRepresentationRGBASet = 1;
    }
  }
}
vtkModelEntity* vtkModelEntityOperationBase::GetModelEntity(vtkDiscreteModel* Model)
{
  if (!Model || !this->GetIsIdSet())
  {
    return 0;
  }
  if (this->GetIsItemTypeSet())
  {
    return Model->GetModelEntity(this->GetItemType(), this->GetId());
  }
  else
  {
    return Model->GetModelEntity(this->GetId());
  }
  return 0;
}

bool vtkModelEntityOperationBase::AbleToOperate(vtkDiscreteModel* Model)
{
  if (!Model)
  {
    vtkErrorMacro("Passed in a null model.");
    return 0;
  }
  if (this->GetIsIdSet() == 0)
  {
    vtkErrorMacro("No entity id specified.");
    return 0;
  }

  if (!this->GetIsVisibilitySet() && !this->GetIsRGBASet() && !this->GetIsRepresentationRGBASet() &&
    !this->GetIsPickableSet() && this->GetUserName() == 0 && !this->GetIsShowTextureSet())
  {
    vtkWarningMacro("Did not set the visibility, user name or RGBA values.");
    return 0;
  }

  return 1;
}

bool vtkModelEntityOperationBase::Operate(vtkDiscreteModel* Model)
{
  vtkDebugMacro("Operating on a model.");

  if (!this->AbleToOperate(Model))
  {
    return 0;
  }

  vtkModelEntity* Entity = this->GetModelEntity(Model);
  if (!Entity)
  {
    vtkErrorMacro("Could not find entity with id " << this->GetId());
    return 0;
  }

  if (this->GetIsVisibilitySet())
  {
    Entity->SetVisibility(this->GetVisibility());
  }

  if (this->GetIsPickableSet() && vtkModelGeometricEntity::SafeDownCast(Entity))
  {
    vtkModelGeometricEntity::SafeDownCast(Entity)->SetPickable(this->GetPickable());
  }

  if (this->GetIsShowTextureSet() && vtkModelGeometricEntity::SafeDownCast(Entity))
  {
    vtkModelGeometricEntity::SafeDownCast(Entity)->SetShowTexture(this->GetShowTexture());
  }

  if (this->GetIsRGBASet())
  {
    double* rgba = this->GetRGBA();
    Entity->SetColor(rgba[0], rgba[1], rgba[2], rgba[3]);
  }

  if (this->GetIsRepresentationRGBASet() && vtkModelGeometricEntity::SafeDownCast(Entity))
  {
    if (vtkProperty* entityProp =
          vtkModelGeometricEntity::SafeDownCast(Entity)->GetDisplayProperty())
    {
      entityProp->SetColor(
        this->RepresentationRGBA[0], this->RepresentationRGBA[1], this->RepresentationRGBA[2]);
    }
  }

  if (this->GetUserName())
  {
    vtkModelUserName::SetUserName(Entity, this->GetUserName());
  }

  this->IsIdSet = 0;
  this->IsItemTypeSet = 0;
  this->IsVisibilitySet = 0;
  this->IsRGBASet = 0;
  this->IsRepresentationRGBASet = 0;
  this->IsPickableSet = 0;
  this->IsShowTextureSet = 0;

  vtkDebugMacro("Finished operating on a model.");
  return 1;
}

void vtkModelEntityOperationBase::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "Id: " << this->Id << endl;
  os << indent << "IsIdSet: " << this->IsIdSet << endl;
  os << indent << "ItemType: " << this->ItemType << endl;
  os << indent << "IsItemTypeSet: " << this->IsItemTypeSet << endl;
  os << indent << "Visibility: " << this->Visibility << endl;
  os << indent << "IsVisibilitySet: " << this->IsVisibilitySet << endl;
  os << indent << "IsPickableSet: " << this->IsPickableSet << endl;
  os << indent << "IsShowTextureSet: " << this->IsShowTextureSet << endl;
  os << indent << "RGBA: " << this->RGBA[0] << " " << this->RGBA[1] << " " << this->RGBA[2] << " "
     << this->RGBA[3] << endl;
  os << indent << "IsRGBASet: " << this->IsRGBASet << endl;
  os << indent << "RepresentationRGBA: " << this->RepresentationRGBA[0] << " "
     << this->RepresentationRGBA[1] << " " << this->RepresentationRGBA[2] << " "
     << this->RepresentationRGBA[3] << endl;
  os << indent << "IsRepresentationRGBASet: " << this->IsRepresentationRGBASet << endl;
}
