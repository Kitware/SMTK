/*=========================================================================

Copyright (c) 1998-2005 Kitware Inc. 28 Corporate Drive, Suite 204,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced,
distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO
PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

=========================================================================*/

// .NAME vtkDiscreteModelWrapper - The new CMB model
//
// .SECTION Description
// This class is just a shell around vtkDiscreteModel, which
// is the real "Model" that contains all the model related APIs.
// The main reason to have this class is that we need a new vtkCmbModelMapper to take
// a model as input and handle the rendering of the model as a whole, instead of having
// a mapper for each geometric model entities. Since vtkDiscreteModel is not a vtkDataObject,
// so we have to have this new class, which is derived from vtkDataOject,
// so that it can be used by the mapper as input.
//
// .SECTION See Also
// vtkDiscreteModel, vtkCmbModelMapper

#ifndef __vtkDiscreteModelWrapper_h
#define __vtkDiscreteModelWrapper_h

#include "vtkCompositeDataSet.h"
#include "vtkSmartPointer.h"
#include "vtkWeakPointer.h"
#include <map>
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

class VTK_EXPORT vtkDiscreteModelWrapper : public vtkCompositeDataSet
{
public:
  vtkTypeMacro(vtkDiscreteModelWrapper, vtkCompositeDataSet);
  void PrintSelf(ostream& os, vtkIndent indent);
  static vtkDiscreteModelWrapper* New();

  // Description:
  // Function to serialize the model from the server to the client.
  vtkStringArray* SerializeModel();

//BTX
  // Description:
  // Function for getting a model entity from its unique persistent id.
  vtkModelEntity* GetModelEntity(vtkIdType UniquePersistentId);

  // Description:
  // Function for getting a model entity from its unique persistent id.  It 
  // only looks for this entity in the given entity type to make the search
  // more efficient.
  vtkModelEntity* GetModelEntity(int itemType, vtkIdType UniquePersistentId);

  vtkModelEntity* GetEntityObjectByFlatIndex(
    unsigned int index);

  // Description:
  // The serializable model that contains all the CMB model APIs.
  vtkDiscreteModel* GetModel();

  // Description:
  // Function to deserialize the model and rebuild the model face geometry 
  // on the server given a serialized model string 
  // ,FaceId/EdgeId->ReverseClassification map and VertexId->ReverseClassification map
  // Return 0 on failure, 1 on success
  int RebuildModel(const char* serializedModel, 
                   std::map<vtkIdType, vtkSmartPointer<vtkIdList> > & FaceToIds,
                   std::map<vtkIdType, vtkIdType> & VertexToIds,
                   std::map<vtkIdType, vtkSmartPointer<vtkProperty> > &EntityToProperties);

  // Description:
  // Add model entities as child dataset.
  void AddGeometricEntities(std::vector<vtkIdType> &entities);
  void AddGeometricEntities(std::vector<vtkModelGeometricEntity*> &entities);
  void AddGeometricEntities(int entType);
//ETX

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
  void SetSerializedModel(vtkStringArray* Array);

  // Description:
  // Retrieve a vtkDiscreteModelWrapper stored inside an information object.
  static vtkDiscreteModelWrapper* GetData(vtkInformation* info);
  static vtkDiscreteModelWrapper* GetData(vtkInformationVector* v, int i=0);

  // Description:
  // This is a convenient method to switch the points for all the
  // geometric entities in the model
  void SetGeometricEntityPoints(vtkPoints* points);

  // Description:
  // Key used to put node name in the meta-data associated with a node.
  static vtkInformationStringKey* NAME();

  // Description:
  // Get the modified time of this object.
  virtual unsigned long GetMTime();
  // Description:
  // Get the composite index given an entity id, return true if EntityId is found
  bool GetChildIndexByEntityId(
    vtkIdType EntityId, unsigned int& index);
  // Description:
  // Get the EntityId given composite index, return true if success
  bool GetEntityIdByChildIndex(
    unsigned int index, vtkIdType& EntityId);
  vtkProperty* GetEntityPropertyByChildIndex(
    unsigned int index);
  vtkProperty* GetEntityPropertyByEntityId(vtkIdType EntityId);

  // Description:
  // Callback function to handle DomainSetCreated/Destroyed event from CMBModel
  static void ModelEntitySetGeometryCallback(vtkObject *caller,
    unsigned long event, void *clientData, void *callData);

//BTX
protected:
  vtkDiscreteModelWrapper();
  ~vtkDiscreteModelWrapper();

  // Description:
  // This is protected on purpose
  void SetModel(vtkDiscreteModel* model);

  friend class vtkCmbModelRepresentation;
  friend class vtkCmbModelSelectionSource;
  friend class vtkModelEntityOperator;
  friend class vtkCmbModelSource;
  friend class vtkCmbModelMapper;
  friend class vtkCmbModelRepresentation;

  vtkDiscreteModel* Model;
  vtkStringArray* SerializedModel;
  vtkCallbackCommand* ModelCBC;

private:
  vtkDiscreteModelWrapper(const vtkDiscreteModelWrapper&);  // Not implemented.
  void operator=(const vtkDiscreteModelWrapper&);  // Not implemented.

//ETX
};

#endif