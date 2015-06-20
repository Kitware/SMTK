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

#include "smtk/extension/vtk/Exports.h"
#include "smtk/model/CellEntity.h" // for CellEntities
#include "smtk/PublicPointerDefs.h"
#include "smtk/common/UUID.h"

#include "vtkMultiBlockDataSetAlgorithm.h"

#include "vtkNew.h"

#include <map>

class vtkPolyData;
class vtkPolyDataNormals;

/**\brief A VTK source for exposing model geometry in SMTK Manager as multiblock data.
  *
  * This filter generates a single block per UUID, for every UUID
  * in model manager with a tessellation entry.
  */
class VTKSMTKEXT_EXPORT vtkModelMultiBlockSource : public vtkMultiBlockDataSetAlgorithm
{
public:
  static vtkModelMultiBlockSource* New();
  virtual void PrintSelf(ostream& os, vtkIndent indent);
  vtkTypeMacro(vtkModelMultiBlockSource,vtkMultiBlockDataSetAlgorithm);

  vtkGetObjectMacro(CachedOutput,vtkMultiBlockDataSet);

  smtk::model::ManagerPtr GetModelManager();
  void SetModelManager(smtk::model::ManagerPtr);
  void SetModelManager(const char* pointerAsString);

  // Description:
  // Model entity ID that this source will be built upon.
  vtkSetStringMacro(ModelEntityID);
  vtkGetStringMacro(ModelEntityID);

  void GetUUID2BlockIdMap(std::map<smtk::common::UUID, unsigned int>& uuid2mid);
  void Dirty();

  vtkGetVector4Macro(DefaultColor,double);
  vtkSetVector4Macro(DefaultColor,double);

  vtkGetMacro(ShowAnalysisTessellation,int);
  vtkSetMacro(ShowAnalysisTessellation,int);
  vtkBooleanMacro(ShowAnalysisTessellation,int);

  bool GetHasAnalysisMesh() const;

  vtkGetMacro(AllowNormalGeneration,int);
  vtkSetMacro(AllowNormalGeneration,int);
  vtkBooleanMacro(AllowNormalGeneration,int);

  // Description:
  // Functions get string names used to store cell/field data.
  static const char* GetEntityTagName() { return "Entity"; }
  static const char* GetGroupTagName() { return "Group"; }
  static const char* GetVolumeTagName() { return "Volume"; }
  static const char* GetAttributeTagName() { return "Attribute"; }

protected:
  vtkModelMultiBlockSource();
  virtual ~vtkModelMultiBlockSource();

  void GenerateRepresentationFromModel(
    vtkPolyData* poly,
    const smtk::model::EntityRef& entity,
    bool genNormals);
  void GenerateRepresentationFromModel(
    vtkMultiBlockDataSet* mbds, smtk::model::ManagerPtr model);

  //virtual int FillInputPortInformation(int port, vtkInformation* request);
  //virtual int FillOutputPortInformation(int port, vtkInformation* request);

  virtual int RequestData(
    vtkInformation* request,
    vtkInformationVector** inInfo,
    vtkInformationVector* outInfo);

  void SetCachedOutput(vtkMultiBlockDataSet*);

  void FindEntitiesWithTessellation(
    const smtk::model::CellEntity &cellent,
    std::map<smtk::model::EntityRef, smtk::model::EntityRef> &entityrefMap);

  smtk::model::ManagerPtr ModelMgr;
  vtkMultiBlockDataSet* CachedOutput;
  double DefaultColor[4];

  std::map<smtk::common::UUID, unsigned int> UUID2BlockIdMap; // UUIDs to block index map
  char* ModelEntityID; // Model Entity UUID

  int AllowNormalGeneration;
  int ShowAnalysisTessellation;
  vtkNew<vtkPolyDataNormals> NormalGenerator;

private:
  vtkModelMultiBlockSource(const vtkModelMultiBlockSource&); // Not implemented.
  void operator = (const vtkModelMultiBlockSource&); // Not implemented.
};

#endif // __smtk_vtk_ModelMultiBlockSource_h
