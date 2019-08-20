//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/vtk/source/vtkResourceMultiBlockSource.h"

#include "vtkInformation.h"
#include "vtkInformationStringKey.h"
#include "vtkInformationVector.h"
#include "vtkMultiBlockDataSet.h"

vtkInformationKeyMacro(vtkResourceMultiBlockSource, COMPONENT_ID, String);

//----------------------------------------------------------------------------
vtkResourceMultiBlockSource::vtkResourceMultiBlockSource() = default;

//----------------------------------------------------------------------------
vtkResourceMultiBlockSource::~vtkResourceMultiBlockSource() = default;

//----------------------------------------------------------------------------
void vtkResourceMultiBlockSource::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
void vtkResourceMultiBlockSource::SetDataObjectUUID(
  vtkInformation* info, const smtk::common::UUID& id)
{
  // FIXME: Eventually this should encode the UUID without string conversion
  info->Set(vtkResourceMultiBlockSource::COMPONENT_ID(), id.toString().c_str());
}

//----------------------------------------------------------------------------
smtk::common::UUID vtkResourceMultiBlockSource::GetDataObjectUUID(vtkInformation* datainfo)
{
  // FIXME: Eventually this should decode the UUID without string conversion
  smtk::common::UUID id;
  if (!datainfo)
  {
    return id;
  }

  const char* uuidChar = datainfo->Get(vtkResourceMultiBlockSource::COMPONENT_ID());
  if (uuidChar)
  {
    id = smtk::common::UUID(uuidChar);
  }
  return id;
}

//----------------------------------------------------------------------------
void vtkResourceMultiBlockSource::SetResourceId(
  vtkMultiBlockDataSet* dataset, const smtk::common::UUID& uid)
{
  if (dataset->GetNumberOfBlocks() <= BlockId::Components)
  {
    dataset->SetNumberOfBlocks(BlockId::NumberOfBlocks);
  }
  vtkResourceMultiBlockSource::SetDataObjectUUID(dataset->GetMetaData(BlockId::Components), uid);
}

//----------------------------------------------------------------------------
smtk::common::UUID vtkResourceMultiBlockSource::GetResourceId(vtkMultiBlockDataSet* dataset)
{
  if (dataset->GetNumberOfBlocks() <= BlockId::Components)
  {
    return smtk::common::UUID::null();
  }
  return vtkResourceMultiBlockSource::GetDataObjectUUID(dataset->GetMetaData(BlockId::Components));
}

//----------------------------------------------------------------------------
smtk::resource::ComponentPtr vtkResourceMultiBlockSource::GetComponent(
  const smtk::resource::ResourcePtr& resource, vtkInformation* info)
{
  if (resource == nullptr)
  {
    return smtk::resource::ComponentPtr();
  }

  return resource->find(vtkResourceMultiBlockSource::GetDataObjectUUID(info));
}

//----------------------------------------------------------------------------
smtk::resource::ComponentPtr vtkResourceMultiBlockSource::GetComponent(vtkInformation* info)
{
  return vtkResourceMultiBlockSource::GetComponent(this->GetResource(), info);
}

//----------------------------------------------------------------------------
smtk::resource::ResourcePtr vtkResourceMultiBlockSource::GetResource()
{
  return this->Resource.lock();
}

//----------------------------------------------------------------------------
void vtkResourceMultiBlockSource::SetResource(const smtk::resource::ResourcePtr& resource)
{
  this->Resource = resource;
  this->Modified();
}

void vtkResourceMultiBlockSource::DumpBlockStructureWithUUIDsInternal(
  vtkMultiBlockDataSet* dataset, int& counter, int indent)
{
  if (!dataset)
  {
    return;
  }
  int nb = dataset->GetNumberOfBlocks();
  for (int ii = 0; ii < nb; ++ii)
  {
    std::cout << std::setfill(' ') << std::setw(indent) << " " << std::setfill(' ') << std::setw(4)
              << ii << " " << std::setfill(' ') << std::setw(4) << (counter++) << " ";
    smtk::common::UUID uid;
    if (dataset->HasMetaData(ii))
    {
      uid = vtkResourceMultiBlockSource::GetDataObjectUUID(dataset->GetMetaData(ii));
    }
    if (uid)
    {
      std::cout << uid;
    }
    else
    {
      std::cout << " no uuid                            ";
    }
    auto block = dataset->GetBlock(ii);
    std::cout << "  " << (block ? block->GetClassName() : "(null)") << "\n";
    auto mbds = vtkMultiBlockDataSet::SafeDownCast(block);
    if (mbds)
    {
      vtkResourceMultiBlockSource::DumpBlockStructureWithUUIDsInternal(mbds, counter, indent + 2);
    }
  }
}
