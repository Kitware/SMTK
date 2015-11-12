//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_vtk_MeshMultiBlockSource_h
#define __smtk_vtk_MeshMultiBlockSource_h

#include "smtk/extension/vtk/Exports.h"
#include "smtk/model/CellEntity.h" // for CellEntities
#include "smtk/mesh/MeshSet.h" // for MeshSet

#include "smtk/PublicPointerDefs.h"
#include "smtk/common/UUID.h"

#include "vtkMultiBlockDataSetAlgorithm.h"

#include "vtkNew.h"

#include <map>

class vtkPolyData;
class vtkPolyDataNormals;

/**\brief A VTK source for exposing mesh geometry in SMTK Mesh Manager as multiblock data.
  *
  * This filter generates a single block per UUID, for every UUID
  * in model manager with an mesh collection in mesh manager.
  */
class VTKSMTKEXT_EXPORT vtkMeshMultiBlockSource : public vtkMultiBlockDataSetAlgorithm
{
public:
  static vtkMeshMultiBlockSource* New();
  virtual void PrintSelf(ostream& os, vtkIndent indent);
  vtkTypeMacro(vtkMeshMultiBlockSource,vtkMultiBlockDataSetAlgorithm);

  vtkGetObjectMacro(CachedOutput,vtkMultiBlockDataSet);

  smtk::model::ManagerPtr GetModelManager();
  void SetModelManager(smtk::model::ManagerPtr);

  // Description:
  // Model entity ID that this source will be built upon.
  vtkSetStringMacro(ModelEntityID);
  vtkGetStringMacro(ModelEntityID);

  smtk::mesh::ManagerPtr GetMeshManager();
  void SetMeshManager(smtk::mesh::ManagerPtr);

  // Description:
  // Mesh collection ID that this source will be built upon.
  vtkSetStringMacro(MeshCollectionID);
  vtkGetStringMacro(MeshCollectionID);

  void GetMeshSet2BlockIdMap(std::map<smtk::mesh::MeshSet, unsigned int>& mesh2block);
  void Dirty();

  vtkGetMacro(AllowNormalGeneration,int);
  vtkSetMacro(AllowNormalGeneration,int);
  vtkBooleanMacro(AllowNormalGeneration,int);

protected:
  vtkMeshMultiBlockSource();
  virtual ~vtkMeshMultiBlockSource();

  void GenerateRepresentationFromMesh(
    vtkMultiBlockDataSet* mbds);
  void GenerateRepresentationForSingleMesh(
  const smtk::mesh::MeshSet& meshes,
  vtkPolyData* pd, const smtk::model::EntityRef& entityref, bool genNormals);

  virtual int RequestData(
    vtkInformation* request,
    vtkInformationVector** inInfo,
    vtkInformationVector* outInfo);

  void SetCachedOutput(vtkMultiBlockDataSet*);

  void FindEntitiesWithMesh(
    const smtk::mesh::CollectionPtr& meshes,
    const smtk::model::EntityRef &root,
    std::map<smtk::model::EntityRef,
              std::pair<smtk::model::EntityRef, smtk::mesh::MeshSet> > &entityrefMap,
    std::set<smtk::model::EntityRef>& touched);

  void GenerateNormals(
    vtkPolyData* pd, const smtk::model::EntityRef& entityref, bool genNormals);

  smtk::model::ManagerPtr m_modelMgr;
  smtk::mesh::ManagerPtr m_meshMgr;
  std::map<smtk::mesh::MeshSet, unsigned int> m_Meshset2BlockIdMap; // MeshSets to block index map
  vtkNew<vtkPolyDataNormals> m_normalGenerator;

  vtkMultiBlockDataSet* CachedOutput;
  char* ModelEntityID; // Model Entity UUID
  char* MeshCollectionID; // Model Entity UUID
  int AllowNormalGeneration;

private:
  vtkMeshMultiBlockSource(const vtkMeshMultiBlockSource&); // Not implemented.
  void operator = (const vtkMeshMultiBlockSource&); // Not implemented.
};

#endif // __smtk_vtk_MeshMultiBlockSource_h
