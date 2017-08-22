//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_vtk_ModelMultiBlockSource_h
#define __smtk_vtk_ModelMultiBlockSource_h

#include "smtk/PublicPointerDefs.h"
#include "smtk/common/UUID.h"
#include "smtk/common/UUIDGenerator.h"
#include "smtk/extension/vtk/source/Exports.h"
#include "smtk/extension/vtk/source/vtkTracksAllInstances.h"
#include "smtk/model/CellEntity.h" // for CellEntities

#include "vtkMultiBlockDataSetAlgorithm.h"
#include "vtkNew.h"
#include "vtkSmartPointer.h"

#include <map>

class vtkPolyData;
class vtkPolyDataNormals;
class vtkInformationStringKey;

/**\brief A VTK source for exposing model geometry in SMTK Manager as multiblock data.
  *
  * This filter generates a single block per UUID, for every UUID
  * in model manager with a tessellation entry.
  */
class VTKSMTKSOURCEEXT_EXPORT vtkModelMultiBlockSource : public vtkMultiBlockDataSetAlgorithm
{
public:
  smtkDeclareTracksAllInstances(vtkModelMultiBlockSource);
  static vtkModelMultiBlockSource* New();
  void PrintSelf(ostream& os, vtkIndent indent) override;
  vtkTypeMacro(vtkModelMultiBlockSource, vtkMultiBlockDataSetAlgorithm);

  enum OutputPorts
  {
    MODEL_ENTITY_PORT,
    PROTOTYPE_PORT,
    INSTANCE_PORT,
    NUMBER_OF_OUTPUT_PORTS
  };

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
  vtkGetObjectMacro(CachedOutputPoly, vtkPolyData);

  smtk::model::ManagerPtr GetModelManager();
  void SetModelManager(smtk::model::ManagerPtr);

  // Description:
  // Model entity ID that this source will be built upon.
  vtkSetStringMacro(ModelEntityID);
  vtkGetStringMacro(ModelEntityID);

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

  // Description:
  // Key used to put entity UUID in the meta-data associated with a block.
  static vtkInformationStringKey* ENTITYID();

  static smtk::common::UUID GetDataObjectUUID(vtkInformation*);
  template <typename T>
  static T GetDataObjectEntityAs(smtk::model::ManagerPtr mgr, vtkInformation* info)
  {
    return T(mgr, vtkModelMultiBlockSource::GetDataObjectUUID(info));
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
  void PrepareInstanceOutput(vtkPolyData* instancePoly, vtkIdType numPoints, vtkIdType numInst);
  void AddInstancePoints(vtkPolyData* instancePoly, const smtk::model::Instance& inst,
    std::map<smtk::model::EntityRef, vtkIdType>& instancePrototypes);
  void GenerateRepresentationFromModel(vtkMultiBlockDataSet* mbds, vtkPolyData* instancePoly,
    vtkMultiBlockDataSet* protoBlocks, smtk::model::ManagerPtr model);
  void GenerateRepresentationFromMeshTessellation(
    vtkPolyData* poly, const smtk::model::EntityRef& entity, bool genNormals);
  void GenerateRepresentationFromModel(vtkMultiBlockDataSet* mbds, smtk::model::ManagerPtr model);

  //virtual int FillInputPortInformation(int port, vtkInformation* request);
  int FillOutputPortInformation(int port, vtkInformation* request) override;

  int RequestData(
    vtkInformation* request, vtkInformationVector** inInfo, vtkInformationVector* outInfo) override;

  void SetCachedOutput(vtkMultiBlockDataSet*, vtkPolyData*, vtkMultiBlockDataSet*);

  smtk::model::ManagerPtr ModelMgr;
  vtkMultiBlockDataSet* CachedOutputMBDS;
  vtkPolyData* CachedOutputPoly;
  vtkMultiBlockDataSet* CachedOutputProto;
  double DefaultColor[4];
  char* ModelEntityID; // Model Entity UUID
  int AllowNormalGeneration;
  int ShowAnalysisTessellation;
  vtkNew<vtkPolyDataNormals> NormalGenerator;
  std::map<smtk::common::UUID, vtkIdType> UUID2BlockIdMap; // UUIDs to block index map

  static smtk::common::UUIDGenerator UUIDGenerator;

private:
  vtkModelMultiBlockSource(const vtkModelMultiBlockSource&); // Not implemented.
  void operator=(const vtkModelMultiBlockSource&);           // Not implemented.
};

#endif // __smtk_vtk_ModelMultiBlockSource_h
