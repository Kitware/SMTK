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
