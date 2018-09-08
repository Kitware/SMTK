//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkDiscreteModelWrapper - The new CMB model
//
// .SECTION Description
// This class is just a shell around vtkDiscreteModel, which
// is the real "Model" that contains all the model related APIs.
// The main reason to have this class is that we need a new vtkCMBModelMapper to take
// a model as input and handle the rendering of the model as a whole, instead of having
// a mapper for each geometric model entities. Since vtkDiscreteModel is not a vtkDataObject,
// so we have to have this new class, which is derived from vtkDataOject,
// so that it can be used by the mapper as input.
//
// .SECTION See Also
// vtkDiscreteModel, vtkCMBModelMapper

#ifndef __smtkdiscrete_vtkDiscreteModelWrapper_h
#define __smtkdiscrete_vtkDiscreteModelWrapper_h

#include "smtk/session/discrete/kernel/vtkSMTKDiscreteModelModule.h" // For export macro
#include "vtkDataObjectTree.h"
#include "vtkSmartPointer.h"
#include "vtkWeakPointer.h"

#include <map>
#include <set>
#include <vector>

class vtkDiscreteModel;
class vtkModelEntity;
class vtkModelGeometricEntity;
class vtkStringArray;
class vtkIdList;
class vtkCompositeDataIterator;
class vtkCallbackCommand;
class vtkInformationStringKey;
class vtkAlgorithmOutput;
class vtkProperty;
class vtkPoints;
class vtkPointData;

class VTKSMTKDISCRETEMODEL_EXPORT vtkDiscreteModelWrapper : public vtkDataObjectTree
{
public:
  vtkTypeMacro(vtkDiscreteModelWrapper, vtkDataObjectTree);
  void PrintSelf(ostream& os, vtkIndent indent) override;
  static vtkDiscreteModelWrapper* New();

  // Description:
  // Function to serialize the model from the server to the client.
  vtkStringArray* SerializeModel();

  // Description:
  // Function for getting a model entity from its unique persistent id.
  vtkModelEntity* GetModelEntity(vtkIdType uniquePersistentId);

  // Description:
  // Function for getting a model entity from its unique persistent id.  It
  // only looks for this entity in the given entity type to make the search
  // more efficient.
  vtkModelEntity* GetModelEntity(int itemType, vtkIdType UniquePersistentId);

  vtkModelEntity* GetEntityObjectByFlatIndex(unsigned int index);

  // Description:
  // The serializable model that contains all the CMB model APIs.
  vtkDiscreteModel* GetModel();

  // Description:
  // Function to deserialize the model and rebuild the model face geometry
  // on the server given a serialized model string
  // ,FaceId/EdgeId->ReverseClassification map and VertexId->ReverseClassification map
  // Return 0 on failure, 1 on success
  int RebuildModel(const char* serializedModel,
    std::map<vtkIdType, vtkSmartPointer<vtkIdList> >& faceToIds,
    std::map<vtkIdType, vtkSmartPointer<vtkIdList> >& edgeToIds,
    std::map<vtkIdType, vtkIdType>& vertexToIds,
    std::map<vtkIdType, vtkSmartPointer<vtkProperty> >& entityToProperties);

  // Description:
  // Add model entities as child dataset.
  void AddGeometricEntities(std::set<vtkIdType>& entities);
  void AddGeometricEntities(std::vector<vtkModelGeometricEntity*>& entities);
  void AddGeometricEntities(int entType);

  // Description:
  // Reset all its children with geometry entities
  void InitializeWithModelGeometry();

  // Description:
  // Reset internl model
  void ResetModel();

  // Description:
  // Get the analysis grid filename if it exists.
  const char* GetAnalysisGridFileName();

  // Description:
  // Get the Server Manager XML from a loaded plugin
  // the string array contains chunks of XML to process
  vtkGetObjectMacro(SerializedModel, vtkStringArray);

  // Description:
  // Set the SerializedModel
  void SetSerializedModel(vtkStringArray* array);

  // Description:
  // Retrieve a vtkDiscreteModelWrapper stored inside an information object.
  static vtkDiscreteModelWrapper* GetData(vtkInformation* info);
  static vtkDiscreteModelWrapper* GetData(vtkInformationVector* v, int i = 0);

  // Description:
  // This is a convenient method to switch the points for all the
  // geometric entities in the model
  void SetGeometricEntityPoints(vtkPoints* points);

  // Description:
  // This is a convenient method to switch the point data for all the
  // geometric entities in the model
  void SetGeometricEntityPointData(vtkPointData* pointData);

  // Description:
  // Key used to put node name in the meta-data associated with a node.
  static vtkInformationStringKey* NAME();

  // Description:
  // Get the modified time of this object.
  vtkMTimeType GetMTime() override;
  // Description:
  // Get the composite index given an entity id, return true if EntityId is found
  bool GetChildIndexByEntityId(vtkIdType EntityId, unsigned int& index);
  // Description:
  // Get the EntityId given composite index, return true if success
  bool GetEntityIdByChildIndex(unsigned int index, vtkIdType& entityId);
  vtkProperty* GetEntityPropertyByChildIndex(unsigned int index);
  vtkProperty* GetEntityPropertyByEntityId(vtkIdType entityId);

  // Description:
  // Callback function to handle DomainSetCreated/Destroyed event from CMBModel
  static void ModelEntitySetGeometryCallback(
    vtkObject* caller, unsigned long event, void* clientData, void* callData);

protected:
  vtkDiscreteModelWrapper();
  ~vtkDiscreteModelWrapper() override;

  // Description:
  // This is protected on purpose
  void SetModel(vtkDiscreteModel* model);

  friend class vtkCMBModelRepresentation;
  friend class vtkCMBModelSelectionSource;
  friend class vtkModelEntityOperation;
  friend class vtkCMBModelSource;
  friend class vtkCMBModelMapper;

  vtkDiscreteModel* Model;
  vtkStringArray* SerializedModel;
  vtkCallbackCommand* ModelCBC;

private:
  vtkDiscreteModelWrapper(const vtkDiscreteModelWrapper&); // Not implemented.
  void operator=(const vtkDiscreteModelWrapper&);          // Not implemented.
};

#endif
