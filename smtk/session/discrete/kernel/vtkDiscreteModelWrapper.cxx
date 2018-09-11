//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkDiscreteModelWrapper.h"
#include "vtkDiscreteModel.h"

#include "vtkAlgorithmOutput.h"
#include "vtkCallbackCommand.h"
#include "vtkCellData.h"
#include "vtkCompositeDataIterator.h"
#include "vtkDiscreteModelEdge.h"
#include "vtkDiscreteModelFace.h"
#include "vtkDiscreteModelVertex.h"
#include "vtkFieldData.h"
#include "vtkIdList.h"
#include "vtkInformation.h"
#include "vtkInformationIdTypeKey.h"
#include "vtkInformationStringKey.h"
#include "vtkInformationVector.h"
#include "vtkModelGeometricEntity.h"
#include "vtkModelGeometricEntity.h"
#include "vtkModelGridRepresentation.h"
#include "vtkModelItemIterator.h"
#include "vtkObjectFactory.h"
#include "vtkOpenGLProperty.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkPolyData.h"
#include "vtkSmartPointer.h"
#include "vtkStringArray.h"
#include "vtkXMLModelReader.h"
#include "vtkXMLModelWriter.h"
#include <sstream>

vtkStandardNewMacro(vtkDiscreteModelWrapper);
//vtkCxxSetObjectMacro(vtkDiscreteModelWrapper, Model, vtkDiscreteModel);
vtkCxxSetObjectMacro(vtkDiscreteModelWrapper, SerializedModel, vtkStringArray);
vtkInformationKeyMacro(vtkDiscreteModelWrapper, NAME, String);

vtkDiscreteModelWrapper::vtkDiscreteModelWrapper()
{
  this->ModelCBC = vtkCallbackCommand::New();
  this->ModelCBC->SetCallback(vtkDiscreteModelWrapper::ModelEntitySetGeometryCallback);
  this->ModelCBC->SetClientData(static_cast<void*>(this));

  this->Model = vtkDiscreteModel::New();
  //  this->Model->AddObserver(ModelEntityGeometrySet, this->ModelCBC);
  this->Model->AddObserver(ModelGeometricEntityAboutToDestroy, this->ModelCBC);
  this->Model->AddObserver(ModelReset, this->ModelCBC);

  this->SerializedModel = 0;
}

vtkDiscreteModelWrapper::~vtkDiscreteModelWrapper()
{
  if (this->Model)
  {
    this->Model->RemoveObserver(this->ModelCBC);
    this->Model->Reset();
    this->Model->Delete();
    this->ModelCBC->Delete();
  }

  this->SetSerializedModel(0);
}

void vtkDiscreteModelWrapper::ResetModel()
{
  if (this->Model)
  {
    this->Model->Reset();
  }
}

void vtkDiscreteModelWrapper::SetGeometricEntityPoints(vtkPoints* points)
{
  vtkDiscreteModel* model = this->GetModel();
  if (model->HasInValidMesh())
  {
    vtkErrorMacro("The discrete model does not have a valid master polydata.");
    return;
  }

  //get the current mesh and set a new point set
  const DiscreteMesh& mesh = model->GetMesh();
  mesh.UpdatePoints(points);
  model->UpdateMesh();

  vtkCompositeDataIterator* iter = this->NewIterator();
  for (iter->InitTraversal(); !iter->IsDoneWithTraversal(); iter->GoToNextItem())
  {
    unsigned int curr_idx = iter->GetCurrentFlatIndex();
    vtkModelGeometricEntity* geomEntity =
      vtkModelGeometricEntity::SafeDownCast(this->GetEntityObjectByFlatIndex(curr_idx));
    if (!geomEntity)
    {
      continue;
    }
    vtkPolyData* entityPoly = vtkPolyData::SafeDownCast(geomEntity->GetGeometry());
    if (entityPoly)
    {
      entityPoly->SetPoints(points);
    }
  }
  iter->Delete();
  this->Modified();
}

void vtkDiscreteModelWrapper::SetGeometricEntityPointData(vtkPointData* pointData)
{
  vtkDiscreteModel* model = this->GetModel();
  if (model->HasInValidMesh())
  {
    vtkErrorMacro("The discrete model does not have a valid master polydata.");
    return;
  }

  //get the current mesh and set a new point data
  const DiscreteMesh& mesh = model->GetMesh();
  mesh.UpdatePointData(pointData);

  vtkCompositeDataIterator* iter = this->NewIterator();
  for (iter->InitTraversal(); !iter->IsDoneWithTraversal(); iter->GoToNextItem())
  {
    unsigned int curr_idx = iter->GetCurrentFlatIndex();
    vtkModelGeometricEntity* geomEntity =
      vtkModelGeometricEntity::SafeDownCast(this->GetEntityObjectByFlatIndex(curr_idx));
    if (!geomEntity)
    {
      continue;
    }
    vtkPolyData* entityPoly = vtkPolyData::SafeDownCast(geomEntity->GetGeometry());
    if (entityPoly)
    {
      entityPoly->GetPointData()->ShallowCopy(pointData);
    }
  }
  iter->Delete();
  this->Modified();
}

vtkDiscreteModel* vtkDiscreteModelWrapper::GetModel()
{
  return this->Model;
}

void vtkDiscreteModelWrapper::SetModel(vtkDiscreteModel* model)
{
  if (this->Model == model)
  {
    return;
  }

  if (this->Model)
  {
    //this->Model->UnRegister(this);
    this->Model = NULL;
  }

  this->Model = model;

  this->Modified();
}
void vtkDiscreteModelWrapper::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "Model: ";
  if (this->Model)
  {
    os << "\n";
    this->Model->PrintSelf(os, indent.GetNextIndent());
  }
  else
  {
    os << "(none)\n";
  }
}

vtkMTimeType vtkDiscreteModelWrapper::GetMTime()
{
  vtkMTimeType mtime = this->Superclass::GetMTime();
  if (this->Model)
  {
    vtkMTimeType stime = this->Model->GetMTime();
    if (stime > mtime)
    {
      mtime = stime;
    }
  }
  return mtime;
}

vtkDiscreteModelWrapper* vtkDiscreteModelWrapper::GetData(vtkInformation* info)
{
  return info ? vtkDiscreteModelWrapper::SafeDownCast(info->Get(DATA_OBJECT())) : 0;
}

vtkDiscreteModelWrapper* vtkDiscreteModelWrapper::GetData(vtkInformationVector* v, int i)
{
  return vtkDiscreteModelWrapper::GetData(v->GetInformationObject(i));
}

const char* vtkDiscreteModelWrapper::GetAnalysisGridFileName()
{
  if (this->Model->GetAnalysisGridInfo())
  {
    return this->Model->GetAnalysisGridInfo()->GetGridFileName();
  }
  return NULL;
}

vtkModelEntity* vtkDiscreteModelWrapper::GetModelEntity(vtkIdType uniquePersistentId)
{
  return this->Model->GetModelEntity(uniquePersistentId);
}

vtkModelEntity* vtkDiscreteModelWrapper::GetModelEntity(int itemType, vtkIdType uniquePersistentId)
{
  return this->Model->GetModelEntity(itemType, uniquePersistentId);
}

vtkStringArray* vtkDiscreteModelWrapper::SerializeModel()
{
  vtkDebugMacro("Serializing the model on the server.");
  if (!this->Model)
  {
    vtkWarningMacro("No model to serialize.");
    return 0;
  }
  vtkSmartPointer<vtkXMLModelWriter> serializer = vtkSmartPointer<vtkXMLModelWriter>::New();
  std::ostringstream ostr;
  // Set to version to 1 (default is 0)
  serializer->SetArchiveVersion(1);
  // The archiver expects a vector of objects
  std::vector<vtkSmartPointer<vtkObject> > objs;
  vtkSmartPointer<vtkObject> obj = this->Model;
  //obj.TakeReference(this->Model);
  objs.push_back(obj);
  // The root node with be called ConceptualModel. This is for
  // reference only.
  serializer->Serialize(ostr, "ConceptualModel", objs);
  if (!this->SerializedModel)
  {
    this->SetSerializedModel(vtkStringArray::New());
    this->SerializedModel->Delete();
  }
  this->SerializedModel->Reset();
  this->SerializedModel->SetNumberOfTuples(1);
  this->SerializedModel->SetValue(0, vtkStdString(ostr.str().c_str()));

  return this->SerializedModel;
}

int vtkDiscreteModelWrapper::RebuildModel(const char* data,
  std::map<vtkIdType, vtkSmartPointer<vtkIdList> >& faceToIds,
  std::map<vtkIdType, vtkSmartPointer<vtkIdList> >& edgeToIds,
  std::map<vtkIdType, vtkIdType>& vertexToIds,
  std::map<vtkIdType, vtkSmartPointer<vtkProperty> >& entityToProperties)
{
  vtkDebugMacro("Serializing the model on the server.");
  if (!this->Model || !data || !(*data))
  {
    vtkWarningMacro("No model to serialize, or no serialized model string.");
    return 0;
  }

  if (this->Model->HasInValidMesh())
  {
    // There is no master poly on the server yet
    vtkErrorMacro("There is no mesh on the server.");
    return 0;
  }

  // make a new poly data and copy the necessary old stuff
  vtkSmartPointer<vtkPolyData> poly = this->Model->GetMesh().GetAsSinglePolyData();

  poly->GetFieldData()->Initialize();
  poly->GetPointData()->Initialize();
  poly->GetCellData()->Initialize();

  this->Model->Reset();

  // Create an input stream to read the XML back
  std::istringstream istr(data);
  vtkSmartPointer<vtkXMLModelReader> reader = vtkSmartPointer<vtkXMLModelReader>::New();

  reader->SetModel(this->Model);
  reader->Serialize(istr, "ConceptualModel");

  DiscreteMesh newMesh(poly);
  this->Model->SetMesh(newMesh);

  this->SerializedModel->Reset();
  this->SerializedModel->SetNumberOfTuples(1);
  this->SerializedModel->SetValue(0, vtkStdString(istr.str().c_str()));

  // Now rebuild the model face geometry
  if (this->Model->HasInValidMesh())
  {
    // There is no master poly on the server yet
    vtkErrorMacro("There is no new mesh on the server.");
    return 0;
  }

  vtkDiscreteModelGeometricEntity* modelEntity = NULL;
  std::map<vtkIdType, vtkSmartPointer<vtkIdList> >::iterator it;
  // Faces
  for (it = faceToIds.begin(); it != faceToIds.end(); it++)
  {
    vtkModelEntity* tmpEntity = this->Model->GetModelEntity(vtkModelFaceType, it->first);
    modelEntity = vtkDiscreteModelFace::SafeDownCast(tmpEntity);
    modelEntity->AddCellsToGeometry(it->second);
    if (entityToProperties.find(it->first) != entityToProperties.end())
    {
      vtkModelGeometricEntity::SafeDownCast(modelEntity->GetThisModelEntity())
        ->SetDisplayProperty(entityToProperties[it->first]);
    }
  }
  // Edges
  for (it = edgeToIds.begin(); it != edgeToIds.end(); it++)
  {
    vtkModelEntity* tmpEntity = this->Model->GetModelEntity(vtkModelEdgeType, it->first);
    modelEntity = vtkDiscreteModelEdge::SafeDownCast(tmpEntity);
    modelEntity->AddCellsToGeometry(it->second);
    if (entityToProperties.find(it->first) != entityToProperties.end())
    {
      vtkModelGeometricEntity::SafeDownCast(modelEntity->GetThisModelEntity())
        ->SetDisplayProperty(entityToProperties[it->first]);
    }
  }
  // Vetex
  std::map<vtkIdType, vtkIdType>::iterator vit;
  for (vit = vertexToIds.begin(); vit != vertexToIds.end(); vit++)
  {
    vtkModelEntity* entity = this->Model->GetModelEntity(vtkModelVertexType, vit->first);
    vtkDiscreteModelVertex* vtxEntity = vtkDiscreteModelVertex::SafeDownCast(entity);
    vtxEntity->SetPointId(vit->second);
    vtxEntity->CreateGeometry();
  }

  this->Modified();
  this->InitializeWithModelGeometry();
  return 1;
}

bool vtkDiscreteModelWrapper::GetEntityIdByChildIndex(unsigned int index, vtkIdType& entityId)
{
  if (this->HasChildMetaData(index))
  {
    vtkInformation* childInfo = this->GetChildMetaData(index);
    vtkIdType entId = static_cast<vtkIdType>(childInfo->Get(vtkModelEntity::UNIQUEPERSISTENTID()));
    entityId = entId;
    return true;
  }
  return false;
}

vtkProperty* vtkDiscreteModelWrapper::GetEntityPropertyByEntityId(vtkIdType entityId)
{
  unsigned int cur_index;
  if (this->GetChildIndexByEntityId(entityId, cur_index))
  {
    return this->GetEntityPropertyByChildIndex(cur_index);
  }
  return NULL;
}

vtkProperty* vtkDiscreteModelWrapper::GetEntityPropertyByChildIndex(unsigned int index)
{
  if (this->HasChildMetaData(index))
  {
    vtkInformation* childInfo = this->GetChildMetaData(index);
    vtkIdType entId = static_cast<vtkIdType>(childInfo->Get(vtkModelEntity::UNIQUEPERSISTENTID()));
    vtkModelGeometricEntity* entity =
      vtkModelGeometricEntity::SafeDownCast(this->Model->GetModelEntity(entId));
    return entity ? entity->GetDisplayProperty() : NULL;
  }
  return NULL;
}

vtkModelEntity* vtkDiscreteModelWrapper::GetEntityObjectByFlatIndex(unsigned int index)
{
  // The flat index starts with root index, so we should minus 1.
  unsigned int flat_index = index - 1;
  if (this->HasChildMetaData(flat_index))
  {
    vtkInformation* childInfo = this->GetChildMetaData(flat_index);
    vtkIdType entId = static_cast<vtkIdType>(childInfo->Get(vtkModelEntity::UNIQUEPERSISTENTID()));
    return this->Model->GetModelEntity(entId);
  }
  return NULL;
}

bool vtkDiscreteModelWrapper::GetChildIndexByEntityId(vtkIdType entityId, unsigned int& index)
{
  unsigned int numChildren = this->GetNumberOfChildren();
  for (unsigned int cc = 0; cc < numChildren; cc++)
  {
    if (this->HasChildMetaData(cc))
    {
      vtkInformation* childInfo = this->GetChildMetaData(cc);
      vtkIdType entId =
        static_cast<vtkIdType>(childInfo->Get(vtkModelEntity::UNIQUEPERSISTENTID()));
      if (entId == entityId)
      {
        index = cc;
        return true;
      }
    }
  }

  return false;
}

void vtkDiscreteModelWrapper::InitializeWithModelGeometry()
{
  this->Initialize();

  // Faces first, then edges, then vertex
  this->AddGeometricEntities(vtkModelFaceType);
  this->AddGeometricEntities(vtkModelEdgeType);
  this->AddGeometricEntities(vtkModelVertexType);
}

void vtkDiscreteModelWrapper::AddGeometricEntities(int entType)
{
  std::vector<vtkModelGeometricEntity*> entities;
  vtkModelItemIterator* iter = this->Model->NewIterator(entType);
  for (iter->Begin(); !iter->IsAtEnd(); iter->Next())
  {
    entities.push_back(vtkModelGeometricEntity::SafeDownCast(iter->GetCurrentItem()));
  }
  iter->Delete();
  this->AddGeometricEntities(entities);
  entities.clear();
}

void vtkDiscreteModelWrapper::AddGeometricEntities(std::set<vtkIdType>& entities)
{
  std::vector<vtkModelGeometricEntity*> geoentities;
  std::set<vtkIdType>::iterator iter;
  unsigned int dummyIdx;
  for (iter = entities.begin(); iter != entities.end(); ++iter)
  {
    // skip if it is already a child
    if (this->GetChildIndexByEntityId(*iter, dummyIdx))
    {
      continue;
    }
    vtkModelGeometricEntity* entity =
      vtkModelGeometricEntity::SafeDownCast(this->Model->GetModelEntity(*iter));
    geoentities.push_back(entity);
  }
  this->AddGeometricEntities(geoentities);
  geoentities.clear();
}

void vtkDiscreteModelWrapper::AddGeometricEntities(std::vector<vtkModelGeometricEntity*>& entities)
{
  unsigned int numEnt = static_cast<unsigned int>(entities.size());
  if (numEnt == 0)
  {
    return;
  }
  unsigned int numItems = this->GetNumberOfChildren();
  this->SetNumberOfChildren(numItems + numEnt);

  std::vector<vtkModelGeometricEntity*>::iterator it;
  unsigned int i = 0;
  for (it = entities.begin(); it != entities.end() && i < numEnt; ++it)
  {
    vtkPolyData* Geometry = vtkPolyData::SafeDownCast((*it)->GetGeometry());
    if (Geometry)
    {
      this->SetChild(numItems + i, Geometry);
      this->GetChildMetaData(numItems + i)
        ->Set(vtkModelEntity::UNIQUEPERSISTENTID(), (*it)->GetUniquePersistentId());
      this->GetEntityPropertyByChildIndex(numItems + i);
      i++;
    }
  }
}

void vtkDiscreteModelWrapper::ModelEntitySetGeometryCallback(
  vtkObject* caller, unsigned long event, void* clientData, void* callData)
{
  vtkDiscreteModel* model = reinterpret_cast<vtkDiscreteModel*>(caller);
  vtkModelGeometricEntity* modelEntity = reinterpret_cast<vtkModelGeometricEntity*>(callData);

  vtkDiscreteModelWrapper* self = reinterpret_cast<vtkDiscreteModelWrapper*>(clientData);
  if (!self || (!model && !modelEntity))
  {
    return;
  }

  switch (event)
  {
    case ModelReset:
    {
      self->SetNumberOfChildren(0);
      self->Modified();
    }
    break;
    case ModelGeometricEntityAboutToDestroy:
    {
      unsigned int childIdx;
      if (self->GetChildIndexByEntityId(modelEntity->GetUniquePersistentId(), childIdx))
      {
        self->RemoveChild(childIdx);
        self->Modified();
      }
    }
    break;
    default:
      break;
  }
}
