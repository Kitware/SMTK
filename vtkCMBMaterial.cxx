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
#include "vtkCMBMaterial.h"

#include "vtkDiscreteModel.h"
#include "vtkCMBModelFace.h"
#include "vtkCMBModelRegion.h"
#include "vtkInformation.h"
#include "vtkInformationDoubleVectorKey.h"
#include "vtkObjectFactory.h"
#include "vtkSerializer.h"

vtkCxxRevisionMacro(vtkCMBMaterial, "");
vtkInformationKeyRestrictedMacro(vtkCMBMaterial, WAREHOUSEID, DoubleVector, 2);

vtkCMBMaterial* vtkCMBMaterial::New()
{
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkCMBMaterial"); 
  if(ret) 
    {                                    
    return static_cast<vtkCMBMaterial*>(ret);
    } 
  return new vtkCMBMaterial;
}

vtkCMBMaterial::vtkCMBMaterial()
{
}

vtkCMBMaterial::~vtkCMBMaterial()
{
}

bool vtkCMBMaterial::IsDestroyable()
{
  if(this->GetNumberOfModelGeometricEntities())
    {
    return false;
    }
  return true;
}

bool vtkCMBMaterial::Destroy()
{
  this->RemoveAllAssociations(vtkModelVertexType);
  this->RemoveAllAssociations(vtkModelEdgeType);
  this->RemoveAllAssociations(vtkModelFaceType);
  this->RemoveAllAssociations(vtkModelRegionType);

  this->Modified();  
  return true;
}

int vtkCMBMaterial::GetType()
{
  return vtkCMBMaterialType;
}

bool vtkCMBMaterial::SetWarehouseId(double* uuid)
{
  this->GetProperties()->Set(WAREHOUSEID(),uuid,2);
  this->Modified();
  return 1;
}

double* vtkCMBMaterial::GetWarehouseId()
{
  return this->GetProperties()->Get(WAREHOUSEID());
}

void vtkCMBMaterial::AddModelGeometricEntity(
  vtkModelGeometricEntity* GeometricEntity)
{
  // first remove the GeometricEntities association with any other material
  vtkCMBModelGeometricEntity* CMBGeometricEntity = 
    vtkCMBModelRegion::SafeDownCast(GeometricEntity);
  if(!CMBGeometricEntity)
    {
    CMBGeometricEntity = vtkCMBModelFace::SafeDownCast(GeometricEntity);
    }
  if(CMBGeometricEntity)
    {
    vtkCMBMaterial* PreviousMaterial = CMBGeometricEntity->GetMaterial();
    if(PreviousMaterial)
      {
      PreviousMaterial->RemoveModelGeometricEntity(GeometricEntity);
      }
    }
  this->AddAssociation(GeometricEntity->GetType(), GeometricEntity);
  this->Modified();
}

int vtkCMBMaterial::GetNumberOfModelGeometricEntities()
{
  int number = this->GetNumberOfAssociations(vtkModelVertexType);
  number += this->GetNumberOfAssociations(vtkModelEdgeType);
  number += this->GetNumberOfAssociations(vtkModelFaceType);
  number += this->GetNumberOfAssociations(vtkModelRegionType);
  return number;
}

bool vtkCMBMaterial::RemoveModelGeometricEntity(
  vtkModelGeometricEntity* GeometricEntity)
{
  this->RemoveAssociation(GeometricEntity->GetType(), 
                          GeometricEntity);
  this->Modified();
  return 1;
}

void vtkCMBMaterial::Serialize(vtkSerializer* ser)
{
  this->Superclass::Serialize(ser);
}

void vtkCMBMaterial::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

