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

#include "smtk/extension/vtk/geometry/Backend.h"
#include "smtk/extension/vtk/geometry/Geometry.h"

#include "smtk/geometry/Resource.h"

#include "vtkDataObject.h"
#include "vtkImageData.h"
#include "vtkInformation.h"
#include "vtkInformationStringKey.h"
#include "vtkInformationVector.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkObjectFactory.h"
#include "vtkSmartPointer.h"

using UUID = smtk::common::UUID;
using SequenceType = vtkResourceMultiBlockSource::SequenceType;

vtkStandardNewMacro(vtkResourceMultiBlockSource);
vtkInformationKeyMacro(vtkResourceMultiBlockSource, COMPONENT_ID, String);

//----------------------------------------------------------------------------
vtkResourceMultiBlockSource::vtkResourceMultiBlockSource()
{
  this->SetNumberOfInputPorts(0);
}

//----------------------------------------------------------------------------
vtkResourceMultiBlockSource::~vtkResourceMultiBlockSource()
{
  this->ClearCache();
}

//----------------------------------------------------------------------------
void vtkResourceMultiBlockSource::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
void vtkResourceMultiBlockSource::SetDataObjectUUID(vtkInformation* info, const UUID& id)
{
  // FIXME: Eventually this should encode the UUID without string conversion
  info->Set(vtkResourceMultiBlockSource::COMPONENT_ID(), id.toString().c_str());
}

//----------------------------------------------------------------------------
UUID vtkResourceMultiBlockSource::GetDataObjectUUID(vtkInformation* datainfo)
{
  // FIXME: Eventually this should decode the UUID without string conversion
  UUID id;
  if (!datainfo)
  {
    return id;
  }

  const char* uuidChar = datainfo->Get(vtkResourceMultiBlockSource::COMPONENT_ID());
  if (uuidChar)
  {
    id = UUID(uuidChar);
  }
  return id;
}

//----------------------------------------------------------------------------
void vtkResourceMultiBlockSource::SetResourceId(vtkMultiBlockDataSet* dataset, const UUID& uid)
{
  if (dataset->GetNumberOfBlocks() <= BlockId::Components)
  {
    dataset->SetNumberOfBlocks(BlockId::NumberOfBlocks);
  }
  vtkResourceMultiBlockSource::SetDataObjectUUID(dataset->GetMetaData(BlockId::Components), uid);
}

//----------------------------------------------------------------------------
UUID vtkResourceMultiBlockSource::GetResourceId(vtkMultiBlockDataSet* dataset)
{
  if (dataset->GetNumberOfBlocks() <= BlockId::Components)
  {
    return UUID::null();
  }
  return vtkResourceMultiBlockSource::GetDataObjectUUID(dataset->GetMetaData(BlockId::Components));
}

//----------------------------------------------------------------------------
smtk::resource::ComponentPtr vtkResourceMultiBlockSource::GetComponent(
  const smtk::resource::ResourcePtr& resource,
  vtkInformation* info)
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

vtkMTimeType vtkResourceMultiBlockSource::GetMTime()
{
  auto geometryResource = std::dynamic_pointer_cast<smtk::geometry::Resource>(this->GetResource());
  if (geometryResource)
  {
    smtk::extension::vtk::geometry::Backend vtk;
    const auto& geom = geometryResource->geometry(vtk);
    if (geom)
    {
      auto lastModified = geom->lastModified();
      if (lastModified > this->LastModified || lastModified == smtk::geometry::Geometry::Invalid)
      {
        this->MTime.Modified();
      }
    }
  }
  return this->MTime;
}

void vtkResourceMultiBlockSource::DumpBlockStructureWithUUIDsInternal(
  vtkMultiBlockDataSet* dataset,
  int& counter,
  int indent)
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
    UUID uid;
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
    auto* block = dataset->GetBlock(ii);
    std::cout << "  " << (block ? block->GetClassName() : "(null)") << "\n";
    auto* mbds = vtkMultiBlockDataSet::SafeDownCast(block);
    if (mbds)
    {
      vtkResourceMultiBlockSource::DumpBlockStructureWithUUIDsInternal(mbds, counter, indent + 2);
    }
  }
}

bool vtkResourceMultiBlockSource::SetCachedData(
  const UUID& uid,
  vtkDataObject* data,
  int sequenceNumber)
{
  if (!data)
  {
    return false;
  }
  CacheEntry entry{ data, sequenceNumber };
  auto it = this->Cache.find(uid);
  bool exists = (it != this->Cache.end());
  if (!exists || it->second.SequenceNumber < sequenceNumber)
  {
    if (exists)
    { // Release the existing entry's data.
      it->second.Data->UnRegister(this);
    }
    this->Cache[uid] = entry;
    data->Register(this);
    return true;
  }
  return false;
}

SequenceType vtkResourceMultiBlockSource::GetCachedDataSequenceNumber(const UUID& uid) const
{
  auto it = this->Cache.find(uid);
  if (it == this->Cache.end())
  {
    return static_cast<SequenceType>(-1);
  }
  return it->second.SequenceNumber;
}

vtkDataObject* vtkResourceMultiBlockSource::GetCachedDataObject(const UUID& uid)
{
  auto it = this->Cache.find(uid);
  if (it == this->Cache.end())
  {
    return nullptr;
  }
  return it->second.Data;
}

bool vtkResourceMultiBlockSource::RemoveCacheEntry(const UUID& uid)
{
  auto it = this->Cache.find(uid);
  if (it == this->Cache.end())
  {
    return false;
  }
  it->second.Data->UnRegister(this);
  this->Cache.erase(it);
  return true;
}

bool vtkResourceMultiBlockSource::RemoveCacheEntriesExcept(const std::set<UUID>& exceptions)
{
  bool didRemove = false;
  for (auto it = this->Cache.begin(); it != this->Cache.end(); /* do nothing */)
  {
    if (exceptions.find(it->first) == exceptions.end())
    {
      it->second.Data->UnRegister(this);
      didRemove = true;
      it = this->Cache.erase(it);
    }
    else
    {
      ++it;
    }
  }
  return didRemove;
}

void vtkResourceMultiBlockSource::ClearCache()
{
  for (auto& entry : this->Cache)
  {
    entry.second.Data->UnRegister(this);
  }
  this->Cache.clear();
}

int vtkResourceMultiBlockSource::RequestDataFromGeometry(
  vtkInformation* request,
  vtkInformationVector* outInfo,
  const smtk::extension::vtk::geometry::Geometry& geometry)
{
  (void)request;
  auto* output = vtkMultiBlockDataSet::GetData(outInfo, 0);
  if (!output)
  {
    vtkErrorMacro("No output dataset");
    return 0;
  }

  // Remember how up-to-date we are with respect to our data source.
  auto lastModified = geometry.lastModified();
  if (lastModified > this->LastModified || lastModified == smtk::geometry::Geometry::Invalid)
  {
    this->LastModified = lastModified;
  }

  // Images separated into fake dimension
  const int IMAGE_DIM = 5;
  std::map<int, std::vector<vtkSmartPointer<vtkDataObject>>> compBlocks;
  // smtk::extension::vtk::geometry::Backend source(&geometry);
  smtk::extension::vtk::geometry::Backend backend;
  geometry.visit([this, &geometry, &compBlocks, IMAGE_DIM](
                   const smtk::resource::PersistentObject::Ptr& obj,
                   smtk::geometry::Geometry::GenerationNumber gen) {
    if (obj)
    {
      int dim = geometry.dimension(obj);
      auto& data = geometry.data(obj);
      if (data)
      {
        // Add Data to the Cache Map
        this->SetCachedData(obj->id(), data, static_cast<SequenceType>(gen));
        vtkResourceMultiBlockSource::SetDataObjectUUID(data->GetInformation(), obj->id());
        // Lets see if this is a component or image object
        if (vtkImageData::SafeDownCast(data))
        {
          compBlocks[IMAGE_DIM].push_back(data);
        }
        else
        {
          compBlocks[dim].push_back(data);
        }
      }
    }
    return false;
  });

  output->SetNumberOfBlocks(BlockId::NumberOfBlocks);
  vtkNew<vtkMultiBlockDataSet> compPerDim;
  vtkNew<vtkMultiBlockDataSet> prototypes;
  vtkNew<vtkMultiBlockDataSet> instances;
  // We need 4 toplevel blocks for component data (dim = to 3)
  compPerDim->SetNumberOfBlocks(4);
  output->SetBlock(BlockId::Components, compPerDim);
  output->SetBlock(BlockId::Prototypes, prototypes);
  output->SetBlock(BlockId::Instances, instances);
  for (auto dit = compBlocks.begin(); dit != compBlocks.end(); ++dit)
  {
    vtkNew<vtkMultiBlockDataSet> entries;
    entries->SetNumberOfBlocks(static_cast<int>(dit->second.size()));
    int bb = 0;
    for (auto iit = dit->second.begin(); iit != dit->second.end(); ++iit, ++bb)
    {
      entries->SetBlock(bb, *iit);
      vtkResourceMultiBlockSource::SetDataObjectUUID(
        entries->GetMetaData(bb),
        vtkResourceMultiBlockSource::GetDataObjectUUID((*iit)->GetInformation()));
      if (const auto* dataName = (*iit)->GetInformation()->Get(vtkCompositeDataSet::NAME()))
      {
        std::string compName(dataName);
        if (!compName.empty())
        {
          entries->GetMetaData(bb)->Set(vtkCompositeDataSet::NAME(), compName.c_str());
        }
      }
    }
    if (dit->first == -1)
    {
      // put unknown here
      compPerDim->SetBlock(2, entries);
    }
    else if (dit->first == IMAGE_DIM)
    {
      // Add all of the image blocks
      output->SetBlock(BlockId::Images, entries);
    }
    else
    {
      compPerDim->SetBlock(dit->first, entries);
    }
  }

  return 1;
}

int vtkResourceMultiBlockSource::RequestData(
  vtkInformation* request,
  vtkInformationVector** /*inInfo*/,
  vtkInformationVector* outInfo)
{
  vtkMultiBlockDataSet* output = vtkMultiBlockDataSet::GetData(outInfo, 0);
  if (!output)
  {
    vtkErrorMacro("No output dataset");
    return 0;
  }

  // Check if the source has an associated resource.
  auto resource = this->GetResource();
  if (!resource)
  {
    vtkErrorMacro("No input resource.");
    return 0;
  }

  // Check if the associated resource is a geometry resource.
  auto geometryResource = std::dynamic_pointer_cast<smtk::geometry::Resource>(this->GetResource());
  if (!geometryResource)
  {
    // If it is not, then set the source's resource id and pass on the empty
    // dataset.
    vtkDebugMacro("Input resource is not a geometry resource");
    vtkResourceMultiBlockSource::SetResourceId(output, resource->id());
    return 1;
  }

  smtk::extension::vtk::geometry::Backend vtk;
  const auto& geom = geometryResource->geometry(vtk);
  if (!geom)
  {
    // Missing geometry is acceptable.
    // vtkErrorMacro("Input resource does not have geometry");
    return 1;
  }

  const auto& properGeom = dynamic_cast<const smtk::extension::vtk::geometry::Geometry&>(*geom);
  if (!geom)
  {
    vtkErrorMacro("Input resource's geometry is not a vtk geometry");
    return 0;
  }

  return this->RequestDataFromGeometry(request, outInfo, properGeom);
}
