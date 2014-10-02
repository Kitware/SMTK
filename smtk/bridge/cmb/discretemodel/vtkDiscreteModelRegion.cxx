//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkDiscreteModelRegion.h"

#include "vtkDiscreteModelEntityGroup.h"
#include "vtkInformation.h"
#include "vtkInformationDoubleVectorKey.h"
#include "vtkInformationStringKey.h"
#include "vtkModelMaterial.h"
#include "vtkDiscreteModel.h"
#include "vtkModelShellUse.h"
#include "vtkObjectFactory.h"
#include "vtkSerializer.h"
#include "vtkSmartPointer.h"

vtkInformationKeyRestrictedMacro(vtkDiscreteModelRegion, POINTINSIDE, DoubleVector, 3);
vtkInformationKeyMacro(vtkDiscreteModelRegion, SOLIDFILENAME, String);

vtkDiscreteModelRegion* vtkDiscreteModelRegion::New()
{
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkDiscreteModelRegion");
  if(ret)
    {
    return static_cast<vtkDiscreteModelRegion*>(ret);
    }
  return new vtkDiscreteModelRegion;
}

vtkDiscreteModelRegion::vtkDiscreteModelRegion()
{
}

vtkDiscreteModelRegion::~vtkDiscreteModelRegion()
{
}

vtkModelEntity* vtkDiscreteModelRegion::GetThisModelEntity()
{
  return this;
}

bool vtkDiscreteModelRegion::Destroy()
{
  this->Superclass::Destroy();
  this->RemoveAllAssociations(vtkModelEdgeType);
  return 1;
}

void vtkDiscreteModelRegion::SetPointInside(double* point)
{
  this->GetProperties()->Set(POINTINSIDE(),point,3);
  this->Modified();
}

double* vtkDiscreteModelRegion::GetPointInside()
{
  return this->GetProperties()->Get(POINTINSIDE());
}
void vtkDiscreteModelRegion::SetSolidFileName(const char* filename)
{
  this->GetProperties()->Set(SOLIDFILENAME(),filename);
  this->Modified();
}
const char* vtkDiscreteModelRegion::GetSolidFileName()
{
  return this->GetProperties()->Get(SOLIDFILENAME());
}

void vtkDiscreteModelRegion::Serialize(vtkSerializer* ser)
{
  this->Superclass::Serialize(ser);
}

void vtkDiscreteModelRegion::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
