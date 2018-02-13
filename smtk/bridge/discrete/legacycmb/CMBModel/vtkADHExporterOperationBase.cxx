//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkADHExporterOperationBase.h"

#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelEntityGroup.h"
#include <map>
#include <utility>
#include <vector>
#include <vtkObjectFactory.h>

struct ADHExporterOperationBaseInternals
{
  //std::vector<std::pair<int, int > > NodalBCs;
  // mapping from <bcIndex> to pair<bcsGroupId, bcGroupType>,
  // where bcGroupType is vtkSBBCInstance::enBCModelEntityGroupTypes
  std::map<int, std::pair<vtkIdType, int> > NodalBCs;
  std::vector<std::pair<int, vtkIdType> > FaceBCs;
};

vtkStandardNewMacro(vtkADHExporterOperationBase);

vtkADHExporterOperationBase::vtkADHExporterOperationBase()
{
  this->FileName = 0;
  this->Internal = new ADHExporterOperationBaseInternals;
}

vtkADHExporterOperationBase::~vtkADHExporterOperationBase()
{
  this->SetFileName(0);
  if (this->Internal)
  {
    delete this->Internal;
    this->Internal = 0;
  }
}

void vtkADHExporterOperationBase::AddAppliedNodalBC(
  int bcIndex, vtkIdType bcsGroupId, int bcsNodalGroupType)
{
  this->Internal->NodalBCs[bcIndex] = std::pair<vtkIdType, int>(bcsGroupId, bcsNodalGroupType);
}

void vtkADHExporterOperationBase::AddAppliedNodalBC(
  int bcIndex, vtkDiscreteModelEntityGroup* bcsGroup, int bcsNodalGroupType)
{
  if (bcsGroup)
  {
    this->AddAppliedNodalBC(bcIndex, bcsGroup->GetUniquePersistentId(), bcsNodalGroupType);
  }
}

int vtkADHExporterOperationBase::GetNumberOfAppliedNodalBCs()
{
  return static_cast<int>(this->Internal->NodalBCs.size());
}

bool vtkADHExporterOperationBase::GetAppliedNodalBC(
  int i, int& bcIndex, vtkIdType& bcsGroupId, int& bcsNodalGroupType)
{
  if (i < 0 || i >= this->GetNumberOfAppliedNodalBCs())
  {
    return false;
  }
  std::map<int, std::pair<vtkIdType, int> >::iterator it = this->Internal->NodalBCs.begin();
  std::advance(it, i);
  bcIndex = it->first;
  bcsGroupId = it->second.first;
  bcsNodalGroupType = it->second.second;
  return true;
}

void vtkADHExporterOperationBase::RemoveAllAppliedNodalBCs()
{
  this->Internal->NodalBCs.clear();
}

void vtkADHExporterOperationBase::AddAppliedElementBC(int bcIndex, vtkIdType faceGroupId)
{
  this->Internal->FaceBCs.push_back(std::make_pair(bcIndex, faceGroupId));
}

void vtkADHExporterOperationBase::AddAppliedElementBC(
  int bcIndex, vtkDiscreteModelEntityGroup* faceGroup)
{
  if (faceGroup)
  {
    this->AddAppliedElementBC(bcIndex, faceGroup->GetUniquePersistentId());
  }
}

int vtkADHExporterOperationBase::GetNumberOfAppliedElementBCs()
{
  return static_cast<int>(this->Internal->FaceBCs.size());
}

bool vtkADHExporterOperationBase::GetAppliedElementBC(int i, int& bcIndex, vtkIdType& faceGroupId)
{
  if (i < 0 || i >= this->GetNumberOfAppliedElementBCs())
  {
    return false;
  }
  bcIndex = this->Internal->FaceBCs[i].first;
  faceGroupId = this->Internal->FaceBCs[i].second;
  return true;
}

void vtkADHExporterOperationBase::RemoveAllAppliedElementBCs()
{
  this->Internal->FaceBCs.clear();
}

bool vtkADHExporterOperationBase::Operate(vtkDiscreteModel* /*model*/)
{
  return true;
}

bool vtkADHExporterOperationBase::AbleToOperate(vtkDiscreteModel* /*model*/)
{
  if (this->FileName)
  {
    return true;
  }
  return false;
}

void vtkADHExporterOperationBase::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "FileName: " << this->FileName << endl;
}
