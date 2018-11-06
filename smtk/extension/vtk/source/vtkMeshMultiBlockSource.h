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

#include "smtk/extension/vtk/source/Exports.h"
#include "smtk/extension/vtk/source/vtkTracksAllInstances.h"
#include "smtk/mesh/core/MeshSet.h" // for MeshSet
#include "smtk/model/CellEntity.h"  // for CellEntities

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
  * in model resource with an mesh resource in mesh manager.
  */
class VTKSMTKSOURCEEXT_EXPORT vtkMeshMultiBlockSource : public vtkMultiBlockDataSetAlgorithm
{
public:
  smtkDeclareTracksAllInstances(vtkMeshMultiBlockSource);
  static vtkMeshMultiBlockSource* New();
  void PrintSelf(ostream& os, vtkIndent indent) override;
  vtkTypeMacro(vtkMeshMultiBlockSource, vtkMultiBlockDataSetAlgorithm);

  vtkGetObjectMacro(CachedOutput, vtkMultiBlockDataSet);

  smtk::model::ResourcePtr GetModelResource();
  void SetModelResource(smtk::model::ResourcePtr);

  // Description:
  // Model entity ID that this source will be built upon.
  vtkSetStringMacro(ModelEntityID);
  vtkGetStringMacro(ModelEntityID);

  smtk::mesh::ResourcePtr GetMeshResource();
  void SetMeshResource(smtk::mesh::ResourcePtr);

  void GetMeshSet2BlockIdMap(std::map<smtk::mesh::MeshSet, vtkIdType>& mesh2block);
  void Dirty();

  vtkGetMacro(AllowNormalGeneration, int);
  vtkSetMacro(AllowNormalGeneration, int);
  vtkBooleanMacro(AllowNormalGeneration, int);

protected:
  vtkMeshMultiBlockSource();
  virtual ~vtkMeshMultiBlockSource();

  void GenerateRepresentationFromMesh(vtkMultiBlockDataSet* mbds);
  void GenerateRepresentationForSingleMesh(const smtk::mesh::MeshSet& meshes, vtkPolyData* pd,
    const smtk::model::EntityRef& entityref, bool genNormals);

  int RequestData(
    vtkInformation* request, vtkInformationVector** inInfo, vtkInformationVector* outInfo) override;

  void SetCachedOutput(vtkMultiBlockDataSet*);

  void FindEntitiesWithMesh(const smtk::mesh::ResourcePtr& meshes,
    const smtk::model::EntityRef& root,
    std::map<smtk::model::EntityRef, std::pair<smtk::model::EntityRef, smtk::mesh::MeshSet> >&
      entityrefMap,
    std::set<smtk::model::EntityRef>& touched);

  void GenerateNormals(vtkPolyData* pd, const smtk::model::EntityRef& entityref, bool genNormals);

  smtk::model::ResourcePtr m_modelResource;
  smtk::mesh::ResourcePtr m_meshResource;
  std::map<smtk::mesh::MeshSet, vtkIdType> m_Meshset2BlockIdMap; // MeshSets to block index map
  vtkNew<vtkPolyDataNormals> m_normalGenerator;

  vtkMultiBlockDataSet* CachedOutput;
  char* ModelEntityID; // Model Entity UUID
  int AllowNormalGeneration;

private:
  vtkMeshMultiBlockSource(const vtkMeshMultiBlockSource&); // Not implemented.
  void operator=(const vtkMeshMultiBlockSource&);          // Not implemented.
};

#endif // __smtk_vtk_MeshMultiBlockSource_h
