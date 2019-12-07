//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/server/vtkPVModelSources.h"

#include "smtk/extension/vtk/source/vtkMeshMultiBlockSource.h"
#include "smtk/extension/vtk/source/vtkModelAuxiliaryGeometry.h"
#include "smtk/extension/vtk/source/vtkModelMultiBlockSource.h"

#include "vtkCompositeDataIterator.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkObjectFactory.h"

vtkPVModelSources* vtkPVModelSources::s_instance = nullptr;

static bool s_assuredDestruction = false;
void vtkPVModelSources::destroySingleton()
{
  if (vtkPVModelSources::s_instance)
  {
    vtkPVModelSources::s_instance->Delete();
    vtkPVModelSources::s_instance = nullptr;
    s_assuredDestruction = false;
  }
}

vtkPVModelSources* vtkPVModelSources::New()
{
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkPVModelSources", false);
  vtkPVModelSources* result = static_cast<vtkPVModelSources*>(ret);
  if (!result)
  {
    result = new vtkPVModelSources;
  }
  result->InitializeObjectBase();

  if (vtkPVModelSources::s_instance)
  {
    vtkPVModelSources::s_instance->Delete();
    vtkPVModelSources::s_instance = nullptr;
  }
  vtkPVModelSources::s_instance = result;
  result->Register(nullptr);
  if (!s_assuredDestruction)
  {
    atexit(&vtkPVModelSources::destroySingleton);
  }
  return result;
}

vtkPVModelSources* vtkPVModelSources::GetInstance()
{
  return vtkPVModelSources::s_instance;
}

class vtkPVModelSources::Internal
{
public:
  std::set<vtkSmartPointer<vtkModelMultiBlockSource> > m_modelSources;
  std::set<vtkSmartPointer<vtkMeshMultiBlockSource> > m_meshSources;
};

vtkPVModelSources::vtkPVModelSources()
{
  m_p = new vtkPVModelSources::Internal;
}

vtkPVModelSources::~vtkPVModelSources()
{
  delete m_p;
}

void vtkPVModelSources::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

/*
bool vtkPVModelSources::AddSource(vtkModelMultiBlockSource* ms)
{
  return m_p.m_modelSources.insert(ms).second;
}

bool vtkPVModelSources::AddSource(vtkMeshMultiBlockSource* ms)
{
  return m_p.m_meshSources.insert(ms).second;
}

bool vtkPVModelSources::RemoveSource(vtkModelMultiBlockSource* ms)
{
  return m_p.m_modelSources.erase(ms) > 0;
}

bool vtkPVModelSources::RemoveSource(vtkMeshMultiBlockSource* ms)
{
  return m_p.m_meshSources.erase(ms) > 0;
}

bool vtkPVModelSources::RemoveAllSources()
{
  bool anythingToErase = !m_p.m_modelSources.empty() || !m_p.m_meshSources.empty();
  m_p.m_modelSources.clear();
  m_p.m_meshSources.clear();
  return anythingToErase;
}

std::pair<vtkModelMultiBlockSource*, vtkIdType> vtkPVModelSources::findModelEntity(
  const smtk::model::EntityRef& ent)
{
  std::map<smtk::common::UUID, vtkIdType> blockMap;
  for (auto src : m_p.m_modelSources)
  {
    src->GetUUID2BlockIdMap(blockMap);
    if (blockMap.find(ent.entity()) != blockMap.end())
    {
      return std::make_pair(src, blockMap->second);
    }
  }
  return std::pair<vtkModelMultiBlockSource*, vtkIdType>(nullptr, -1);
}

std::pair<vtkMeshMultiBlockSource*, vtkIdType> vtkPVModelSources::findMeshSet(
  const smtk::mesh::MeshSet& mesh)
{
  std::map<smtk::mesh::MeshSet, vtkIdType> blockMap;
  for (auto src : m_p.m_meshSources)
  {
    src->GetMeshSet2BlockIdMap(blockMap);
    if (blockMap.find(mesh) != blockMap.end())
    {
      return std::make_pair(src, blockMap->second);
    }
  }
  return std::pair<vtkMeshMultiBlockSource*, vtkIdType>(nullptr, -1);
}
*/

std::pair<vtkMultiBlockDataSetAlgorithm*, vtkIdType> vtkPVModelSources::findModelEntitySource(
  const smtk::model::EntityRef& ent)
{
  vtkMultiBlockDataSetAlgorithm* source = nullptr;
  vtkIdType blockId = -1;
  vtkModelMultiBlockSource::visitInstances(
    [ent, &source, &blockId](vtkModelMultiBlockSource* inst) {
      std::map<smtk::common::UUID, vtkIdType> blockMap;
      inst->GetUUID2BlockIdMap(blockMap);
      std::map<smtk::common::UUID, vtkIdType>::const_iterator bit;
      if ((bit = blockMap.find(ent.entity())) != blockMap.end())
      {
        source = inst;
        blockId = bit->second;
        return true;
      }
      return false;
    });
  if (!source)
  {
    std::string uid(ent.entity().toString());
    vtkModelAuxiliaryGeometry::visitInstances(
      [uid, &source, &blockId](vtkModelAuxiliaryGeometry* inst) {
        if (uid == inst->GetAuxiliaryEntityID())
        {
          source = inst;
          blockId = 0;
          return true;
        }
        return false;
      });
  }
  return std::make_pair(source, blockId);
}

std::pair<vtkMeshMultiBlockSource*, vtkIdType> vtkPVModelSources::findMeshSetSource(
  const smtk::mesh::MeshSet& mesh)
{
  vtkMeshMultiBlockSource* source = nullptr;
  vtkIdType blockId = -1;
  vtkMeshMultiBlockSource::visitInstances([mesh, &source, &blockId](vtkMeshMultiBlockSource* inst) {
    std::map<smtk::common::UUID, vtkIdType> blockMap;
    inst->GetUUID2BlockIdMap(blockMap);
    std::map<smtk::common::UUID, vtkIdType>::const_iterator bit;
    if ((bit = blockMap.find(mesh.id())) != blockMap.end())
    {
      source = inst;
      blockId = bit->second;
      return true;
    }
    return false;
  });
  return std::make_pair(source, blockId);
}

vtkDataObject* vtkPVModelSources::findModelEntity(const smtk::model::EntityRef& ent)
{
  std::pair<vtkMultiBlockDataSetAlgorithm*, vtkIdType> result =
    vtkPVModelSources::findModelEntitySource(ent);
  if (!result.first)
  {
    return nullptr;
  }
  vtkDataObject* res = result.first->GetOutputDataObject(0);
  if (result.second < 0)
  {
    return res;
  }
  vtkCompositeDataSet* mbds = vtkCompositeDataSet::SafeDownCast(res);
  if (!mbds)
  {
    return nullptr;
  }
  auto iter = mbds->NewIterator();
  for (iter->InitTraversal(); !iter->IsDoneWithTraversal(); iter->GoToNextItem())
  {
    if (iter->GetCurrentFlatIndex() == result.second)
    {
      res = iter->GetCurrentDataObject();
      iter->Delete();
      return res;
    }
  }
  iter->Delete();
  return nullptr;
}

vtkDataObject* vtkPVModelSources::findMeshSet(const smtk::mesh::MeshSet& mesh)
{
  std::pair<vtkMeshMultiBlockSource*, vtkIdType> result =
    vtkPVModelSources::findMeshSetSource(mesh);
  if (!result.first)
  {
    return nullptr;
  }
  vtkDataObject* res = result.first->GetOutputDataObject(0);
  if (result.second < 0)
  {
    return res;
  }
  vtkCompositeDataSet* mbds = vtkCompositeDataSet::SafeDownCast(res);
  if (!mbds)
  {
    return nullptr;
  }
  auto iter = mbds->NewIterator();
  for (iter->InitTraversal(); !iter->IsDoneWithTraversal(); iter->GoToNextItem())
  {
    if (iter->GetCurrentFlatIndex() == result.second)
    {
      res = iter->GetCurrentDataObject();
      iter->Delete();
      return res;
    }
  }
  iter->Delete();
  return nullptr;
}
