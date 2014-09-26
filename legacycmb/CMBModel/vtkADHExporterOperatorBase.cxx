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

#include "vtkADHExporterOperatorBase.h"

#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelEntityGroup.h"
#include <vtkObjectFactory.h>
#include <utility>
#include <vector>
#include <map>

struct ADHExporterOperatorBaseInternals
{
  //std::vector<std::pair<int, int > > NodalBCs;
  // mapping from <bcIndex> to pair<bcsGroupId, bcGroupType>,
  // where bcGroupType is vtkSBBCInstance::enBCModelEntityGroupTypes
  std::map<int, std::pair<vtkIdType, int> > NodalBCs;
  std::vector<std::pair<int, vtkIdType> > FaceBCs;
};

vtkStandardNewMacro(vtkADHExporterOperatorBase);

vtkADHExporterOperatorBase::vtkADHExporterOperatorBase()
{
  this->FileName = 0;
  this->Internal = new ADHExporterOperatorBaseInternals;
}

vtkADHExporterOperatorBase::~vtkADHExporterOperatorBase()
{
  this->SetFileName(0);
  if(this->Internal)
    {
    delete this->Internal;
    this->Internal = 0;
    }
}

void vtkADHExporterOperatorBase::AddAppliedNodalBC(
  int bcIndex, vtkIdType bcsGroupId, int bcsNodalGroupType)
{
  this->Internal->NodalBCs[bcIndex] = std::pair<vtkIdType,int>(
    bcsGroupId, bcsNodalGroupType);
}

void vtkADHExporterOperatorBase::AddAppliedNodalBC(
  int bcIndex, vtkDiscreteModelEntityGroup* bcsGroup, int bcsNodalGroupType)
{
  if(bcsGroup)
    {
    this->AddAppliedNodalBC(bcIndex, bcsGroup->GetUniquePersistentId(),
      bcsNodalGroupType);
    }
}

int vtkADHExporterOperatorBase::GetNumberOfAppliedNodalBCs()
{
  return static_cast<int>(this->Internal->NodalBCs.size());
}

bool vtkADHExporterOperatorBase::GetAppliedNodalBC(int i, int & bcIndex,
  vtkIdType & bcsGroupId, int & bcsNodalGroupType)
{
  if(i< 0 || i >= this->GetNumberOfAppliedNodalBCs())
    {
    return false;
    }
  std::map<int, std::pair<vtkIdType, int> >::iterator it=
    this->Internal->NodalBCs.begin();
  std::advance(it, i);
  bcIndex = it->first;
  bcsGroupId = it->second.first;
  bcsNodalGroupType = it->second.second;
  return true;
}

void vtkADHExporterOperatorBase::RemoveAllAppliedNodalBCs()
{
  this->Internal->NodalBCs.clear();
}

void vtkADHExporterOperatorBase::AddAppliedElementBC(
  int bcIndex, vtkIdType faceGroupId)
{
  this->Internal->FaceBCs.push_back(std::make_pair(bcIndex, faceGroupId));
}

void vtkADHExporterOperatorBase::AddAppliedElementBC(
  int bcIndex, vtkDiscreteModelEntityGroup* faceGroup)
{
  if(faceGroup)
    {
    this->AddAppliedElementBC(bcIndex, faceGroup->GetUniquePersistentId());
    }
}

int vtkADHExporterOperatorBase::GetNumberOfAppliedElementBCs()
{
  return static_cast<int>(this->Internal->FaceBCs.size());
}

bool vtkADHExporterOperatorBase::GetAppliedElementBC(int i, int & bcIndex, vtkIdType & faceGroupId)
{
  if(i< 0 || i >= this->GetNumberOfAppliedElementBCs())
    {
    return false;
    }
  bcIndex = this->Internal->FaceBCs[i].first;
  faceGroupId = this->Internal->FaceBCs[i].second;
  return true;
}

void vtkADHExporterOperatorBase::RemoveAllAppliedElementBCs()
{
  this->Internal->FaceBCs.clear();
}

bool vtkADHExporterOperatorBase::Operate(vtkDiscreteModel* /*model*/)
{
  return true;
}

bool vtkADHExporterOperatorBase::AbleToOperate(vtkDiscreteModel* /*model*/)
{
  if(this->FileName)
    {
    return true;
    }
  return false;
}

void vtkADHExporterOperatorBase::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "FileName: " << this->FileName << endl;
}
