//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_vtk_ModelMultiBlockSource_h
#define smtk_vtk_ModelMultiBlockSource_h

#include "smtk/PublicPointerDefs.h"
#include "smtk/common/UUID.h"
#include "smtk/extension/vtk/source/Exports.h"
#include "smtk/extension/vtk/source/vtkTracksAllInstances.h"
#include "smtk/model/CellEntity.h" // for CellEntities

#include "vtkMultiBlockDataSetAlgorithm.h"
#include "vtkNew.h"
#include "vtkSmartPointer.h"

#include <map>

#define VTK_INSTANCE_ORIENTATION "instance orientation"
#define VTK_INSTANCE_SCALE "instance scale"
#define VTK_INSTANCE_SOURCE "instance source"
#define VTK_INSTANCE_VISIBILITY "instance visibility"

class vtkPolyData;
class vtkPolyDataNormals;
class vtkInformationStringKey;

/**\brief A VTK source for exposing model geometry in SMTK Resource as multiblock data.
  *
  * This filter generates a single block per UUID, for every UUID
  * in model resource with a tessellation entry.
  */
class VTKSMTKSOURCEEXT_EXPORT vtkModelMultiBlockSource : public vtkMultiBlockDataSetAlgorithm
{
public:
  smtkDeclareTracksAllInstances(vtkModelMultiBlockSource);
  static vtkModelMultiBlockSource* New();
  void PrintSelf(ostream& os, vtkIndent indent) override;
  vtkTypeMacro(vtkModelMultiBlockSource, vtkMultiBlockDataSetAlgorithm);

  enum ToplevelBlockType
  {
    AUXILIARY_VOLUMES,
    AUXILIARY_SURFACES,
    AUXILIARY_CURVES,
    AUXILIARY_POINTS,
    AUXILIARY_MIXED,
    GROUPS,
    VOLUMES,
    FACES,
    EDGES,
    VERTICES,
    NUMBER_OF_BLOCK_TYPES
  };

  vtkGetObjectMacro(CachedOutputMBDS, vtkMultiBlockDataSet);
  vtkGetObjectMacro(CachedOutputInst, vtkMultiBlockDataSet);

  smtk::model::ResourcePtr GetModelResource();
  void SetModelResource(smtk::model::ResourcePtr);

  void GetUUID2BlockIdMap(std::map<smtk::common::UUID, vtkIdType>& uuid2mid);
  void Dirty();

  vtkGetVector4Macro(DefaultColor, double);
  vtkSetVector4Macro(DefaultColor, double);

  vtkGetMacro(ShowAnalysisTessellation, int);
  vtkSetMacro(ShowAnalysisTessellation, int);
  vtkBooleanMacro(ShowAnalysisTessellation, int);

  vtkGetMacro(AllowNormalGeneration, int);
  vtkSetMacro(AllowNormalGeneration, int);
  vtkBooleanMacro(AllowNormalGeneration, int);

  // Description:
  // Functions get string names used to store cell/field data.
  static const char* GetEntityTagName() { return "Entity"; }
  static const char* GetGroupTagName() { return "Group"; }
  static const char* GetVolumeTagName() { return "Volume"; }
  static const char* GetAttributeTagName() { return "Attribute"; }

  /// Key used to put entity UUID in the meta-data associated with a block.
  static vtkInformationStringKey* ENTITYID();

  /// Set the ENTITYID key on the given \a information object to \a uid.
  static void SetDataObjectUUID(vtkInformation* information, const smtk::common::UUID& uid);

  /**\brief Return a UUID for the data object, adding one if it was not present.
    *
    * UUIDs are stored in the vtkInformation object associated with each
    * data object.
    */
  static smtk::common::UUID GetDataObjectUUID(vtkInformation*);
  template <typename T>
  static T GetDataObjectEntityAs(smtk::model::ResourcePtr resource, vtkInformation* info)
  {
    return T(resource, vtkModelMultiBlockSource::GetDataObjectUUID(info));
  }

  static void AddPointsAsAttribute(vtkPolyData* data);

protected:
  vtkModelMultiBlockSource();
  ~vtkModelMultiBlockSource() override;

  vtkSmartPointer<vtkDataObject> GenerateRepresentationFromModel(
    const smtk::model::EntityRef& entity, bool genNormals);
  vtkSmartPointer<vtkPolyData> GenerateRepresentationFromTessellation(
    const smtk::model::EntityRef& entity, const smtk::model::Tessellation* tess, bool genNormals);
  vtkSmartPointer<vtkPolyData> GenerateRepresentationFromMeshTessellation(
    const smtk::model::EntityRef& entity, bool genNormals);

  void GenerateRepresentationFromModel(
    vtkPolyData* poly, const smtk::model::EntityRef& entity, bool genNormals);
  void AddInstanceMetadata(vtkIdType& npts, smtk::model::InstanceSet& modelInstances,
    const smtk::model::Instance& inst,
    std::map<smtk::model::EntityRef, vtkIdType>& instancePrototypes);
  void PreparePrototypeOutput(vtkMultiBlockDataSet* mbds, vtkMultiBlockDataSet* protoBlocks,
    std::map<smtk::model::EntityRef, vtkIdType>& instancePrototypes);
  void PrepareInstanceOutput(vtkMultiBlockDataSet* instanceBlocks, const smtk::model::InstanceSet&,
    std::map<smtk::model::EntityRef, vtkIdType>&);
  void AddInstancePoints(vtkPolyData* instancePoly, const smtk::model::Instance& inst,
    std::map<smtk::model::EntityRef, vtkIdType>& instancePrototypes);
  void GenerateRepresentationFromModel(vtkMultiBlockDataSet* mbds,
    vtkMultiBlockDataSet* instancePoly, vtkMultiBlockDataSet* protoBlocks,
    smtk::model::ResourcePtr model);
  void GenerateRepresentationFromMeshTessellation(
    vtkPolyData* poly, const smtk::model::EntityRef& entity, bool genNormals);
  void GenerateRepresentationFromModel(vtkMultiBlockDataSet* mbds, smtk::model::ResourcePtr model);

  int RequestData(
    vtkInformation* request, vtkInformationVector** inInfo, vtkInformationVector* outInfo) override;

  void SetCachedOutput(vtkMultiBlockDataSet*, vtkMultiBlockDataSet*, vtkMultiBlockDataSet*);

  smtk::model::ResourcePtr ModelResource;
  vtkMultiBlockDataSet* CachedOutputMBDS;
  vtkMultiBlockDataSet* CachedOutputProto;
  vtkMultiBlockDataSet* CachedOutputInst;
  double DefaultColor[4];
  int AllowNormalGeneration;
  int ShowAnalysisTessellation;
  vtkNew<vtkPolyDataNormals> NormalGenerator;
  std::map<smtk::common::UUID, vtkIdType> UUID2BlockIdMap; // UUIDs to block index map

private:
  vtkModelMultiBlockSource(const vtkModelMultiBlockSource&); // Not implemented.
  void operator=(const vtkModelMultiBlockSource&);           // Not implemented.
};

#endif // smtk_vtk_ModelMultiBlockSource_h
