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

#include "vtkCMBModelWrapper.h"
#include "vtkDiscreteModel.h"

#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkSmartPointer.h"
#include "vtkAlgorithmOutput.h"
#include "vtkModelGeometricEntity.h"
#include "vtkCallbackCommand.h"
#include "vtkPolyData.h"
#include "vtkInformationIdTypeKey.h"
#include "vtkInformationStringKey.h"
#include "vtkModelGeometricEntity.h"
#include "vtkCellData.h"
#include "vtkCmbGridRepresentation.h"
#include "vtkCMBModelEdge.h"
#include "vtkCMBModelFace.h"
#include "vtkCMBModelVertex.h"
#include "vtkCMBXMLModelReader.h"
#include "vtkCMBXMLModelWriter.h"
#include "vtkFieldData.h"
#include "vtkIdList.h"
#include "vtkOpenGLProperty.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkStringArray.h"
#include "vtkCompositeDataIterator.h"
#include "vtkModelItemIterator.h"
#include <vtksys/ios/sstream>

vtkStandardNewMacro(vtkCMBModelWrapper);
//vtkCxxSetObjectMacro(vtkCMBModelWrapper, Model, vtkDiscreteModel);
vtkCxxSetObjectMacro(vtkCMBModelWrapper, SerializedModel, vtkStringArray);
vtkInformationKeyMacro(vtkCMBModelWrapper, NAME, String);

vtkCMBModelWrapper::vtkCMBModelWrapper()
{
  this->ModelCBC = vtkCallbackCommand::New();
  this->ModelCBC->SetCallback(vtkCMBModelWrapper::ModelEntitySetGeometryCallback);
  this->ModelCBC->SetClientData((void *)this);

  this->Model = vtkDiscreteModel::New();
//  this->Model->AddObserver(ModelEntityGeometrySet, this->ModelCBC);
  this->Model->AddObserver(ModelGeometricEntityAboutToDestroy, this->ModelCBC);
  this->Model->AddObserver(ModelReset, this->ModelCBC);

  this->SerializedModel = 0;
}

vtkCMBModelWrapper::~vtkCMBModelWrapper()
{
  if(this->Model)
    {
    this->Model->RemoveObserver(this->ModelCBC);
    this->Model->Reset();
    this->Model->Delete();
    this->ModelCBC->Delete();
    }

 this->SetSerializedModel(0);
}

void vtkCMBModelWrapper::ResetModel()
{
  if(this->Model)
    {
    this->Model->Reset();
    }
}

//----------------------------------------------------------------------------
void vtkCMBModelWrapper::SetGeometricEntityPoints(vtkPoints* points)
{
  vtkDiscreteModel* Model = this->GetModel();
  vtkPolyData* MasterPoly = vtkPolyData::SafeDownCast(
    Model->GetGeometry());
  if(!MasterPoly)
    {
    vtkErrorMacro("The CMB model does not have a valid master polydata.");
    return;
    }
  MasterPoly->SetPoints(points);

  unsigned int curr_idx;
  vtkModelGeometricEntity* geomEntity;
  vtkPolyData* entityPoly;
  vtkCompositeDataIterator* iter = this->NewIterator();
  for (iter->InitTraversal(); !iter->IsDoneWithTraversal(); iter->GoToNextItem())
    {
    curr_idx = iter->GetCurrentFlatIndex();
    geomEntity = vtkModelGeometricEntity::SafeDownCast(
      this->GetEntityObjectByFlatIndex(curr_idx));
    if(!geomEntity)
      {
      continue;
      }
    entityPoly = vtkPolyData::SafeDownCast(geomEntity->GetGeometry());
    if(entityPoly)
      {
      entityPoly->SetPoints(points);
      }
    }
  iter->Delete();
  this->Modified();
}

//----------------------------------------------------------------------------
vtkDiscreteModel* vtkCMBModelWrapper::GetModel()
{
  return this->Model;
}
//----------------------------------------------------------------------------
void vtkCMBModelWrapper::SetModel(vtkDiscreteModel *model)
{
  if ( this->Model == model)
    {
    return;
    }

  if ( this->Model )
    {
    //this->Model->UnRegister(this);
    this->Model = NULL;
    }

  //if ( model )
  //  {
  //  model->Register(this);
  //  }

  this->Model = model;

  this->Modified();
}
void vtkCMBModelWrapper::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "Model: ";
  if(this->Model)
    {
    os << "\n";
    this->Model->PrintSelf(os, indent.GetNextIndent());
    }
  else
    {
    os << "(none)\n";
    }
}

unsigned long vtkCMBModelWrapper::GetMTime()
{
  unsigned long mtime = this->Superclass::GetMTime();
  if (this->Model)
    {
    unsigned long stime = this->Model->GetMTime();
    if (stime > mtime)
      {
      mtime = stime;
      }
    }
  return mtime;
}

vtkCMBModelWrapper* vtkCMBModelWrapper::GetData(vtkInformation* info)
{
  return info ? vtkCMBModelWrapper::SafeDownCast(info->Get(DATA_OBJECT())) : 0;
}

vtkCMBModelWrapper* vtkCMBModelWrapper::GetData(vtkInformationVector* v, int i)
{
  return vtkCMBModelWrapper::GetData(v->GetInformationObject(i));
}

//----------------------------------------------------------------------------
const char* vtkCMBModelWrapper::GetAnalysisGridFileName()
{
  if(this->Model->GetAnalysisGridInfo())
    {
    return this->Model->GetAnalysisGridInfo()->GetGridFileName();
    }
  return NULL;
}

//----------------------------------------------------------------------------
vtkModelEntity* vtkCMBModelWrapper::GetModelEntity(
  vtkIdType UniquePersistentId)
{
  return this->Model->GetModelEntity(UniquePersistentId);
}

//----------------------------------------------------------------------------
vtkModelEntity* vtkCMBModelWrapper::GetModelEntity(
  int itemType, vtkIdType UniquePersistentId)
{
  return this->Model->GetModelEntity(itemType, UniquePersistentId);
}

//----------------------------------------------------------------------------
vtkStringArray* vtkCMBModelWrapper::SerializeModel()
{
  vtkDebugMacro("Serializing the model on the server.");
  if(!this->Model)
    {
    vtkWarningMacro("No model to serialize.");
    return 0;
    }
  vtkSmartPointer<vtkCMBXMLModelWriter> serializer = 
    vtkSmartPointer<vtkCMBXMLModelWriter>::New();
  vtksys_ios::ostringstream ostr;
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
  if(!this->SerializedModel)
    {
    this->SetSerializedModel(vtkStringArray::New());
    this->SerializedModel->Delete();
    }
  this->SerializedModel->Reset();
  this->SerializedModel->SetNumberOfTuples(1);
  this->SerializedModel->SetValue(0, vtkStdString(ostr.str().c_str()));

  return this->SerializedModel;
}

//----------------------------------------------------------------------------
int vtkCMBModelWrapper::RebuildModel(const char* data,
  std::map<vtkIdType, vtkSmartPointer<vtkIdList> > & FaceToIds,
  std::map<vtkIdType, vtkIdType> & VertexToIds,
  std::map<vtkIdType, vtkSmartPointer<vtkProperty> > &EntityToProperties)
{
  vtkDebugMacro("Serializing the model on the server.");
  if(!this->Model || !data || !(*data))
    {
    vtkWarningMacro("No model to serialize, or no serialized model string.");
    return 0;
    }

  vtkPolyData* oldMasterPoly = vtkPolyData::SafeDownCast(this->Model->GetGeometry());
  if(oldMasterPoly == 0)
    {
    // There is no master poly on the server yet
    vtkErrorMacro("There is no master poly on the server.");
    return 0;
    }

  // make a new poly data and copy the necessary old stuff
  vtkPolyData* poly = vtkPolyData::New();
  poly->ShallowCopy(oldMasterPoly);
  poly->GetFieldData()->Initialize();
  poly->GetPointData()->Initialize();
  poly->GetCellData()->Initialize();

  this->Model->Reset();

  // Create an input stream to read the XML back
  vtksys_ios::istringstream istr(data);
  vtkSmartPointer<vtkCMBXMLModelReader> reader = 
    vtkSmartPointer<vtkCMBXMLModelReader>::New();

  reader->SetModel(this->Model);
  reader->Serialize(istr, "ConceptualModel");

  this->Model->SetGeometry(poly);
  poly->Delete();
  poly = 0; // just to be safe since we don't want to do anything with this poly

  this->SerializedModel->Reset();
  this->SerializedModel->SetNumberOfTuples(1);
  this->SerializedModel->SetValue(0, vtkStdString(istr.str().c_str()));

  // Now rebuild the model face geometry
  vtkPolyData* MasterPoly = vtkPolyData::SafeDownCast(this->Model->GetGeometry());
  if(MasterPoly == 0)
    {
    // There is no master poly on the server yet
    vtkErrorMacro("There is no new master poly on the server.");
    return 0;
    }

  bool dim2D = this->Model->GetNumberOfAssociations(
    vtkModelRegionType) > 0 ? false : true;
  int enumEnType = dim2D ? vtkModelEdgeType : vtkModelFaceType;
  vtkCMBModelGeometricEntity* modelEntity = NULL;
  std::map<vtkIdType, vtkSmartPointer<vtkIdList> >::iterator it;  
  for(it=FaceToIds.begin(); it!=FaceToIds.end(); it++)
    {
    vtkModelEntity* tmpEntity = this->Model->GetModelEntity(enumEnType, it->first);
    if(dim2D)
      {
      modelEntity = vtkCMBModelEdge::SafeDownCast(tmpEntity);
      }
    else
      {
      modelEntity = vtkCMBModelFace::SafeDownCast(tmpEntity);
      }
    modelEntity->AddCellsToGeometry(it->second);
    if(EntityToProperties.find(it->first) != EntityToProperties.end())
      {
      modelEntity->GetThisModelEntity()->SetDisplayProperty(
        EntityToProperties[it->first]);
      }
    }

  if(enumEnType == vtkModelEdgeType && VertexToIds.size()>0)
    {
    std::map<vtkIdType, vtkIdType >::iterator it;  
    for(it=VertexToIds.begin(); it!=VertexToIds.end(); it++)
      {
      vtkModelEntity* entity = this->Model->GetModelEntity(vtkModelVertexType, it->first);
      vtkCMBModelVertex* vtxEntity = vtkCMBModelVertex::SafeDownCast(entity);
      vtxEntity->SetPointId(it->second);
      vtxEntity->CreateGeometry();
      }
    }
  this->Modified();
  this->InitializeWithModelGeometry();
  return 1;
}

//----------------------------------------------------------------------------
bool vtkCMBModelWrapper::GetEntityIdByChildIndex(
  unsigned int index, vtkIdType& EntityId)
{
  if (this->HasChildMetaData(index))
    {
    vtkInformation* childInfo = this->GetChildMetaData(index);
    vtkIdType entId = static_cast<vtkIdType>(
      childInfo->Get(vtkModelEntity::UNIQUEPERSISTENTID()));
    EntityId = entId;
    return true;
    }
  return false;
}
//----------------------------------------------------------------------------
vtkProperty* vtkCMBModelWrapper::GetEntityPropertyByEntityId(
  vtkIdType EntityId)
{
  unsigned int cur_index;
  if(this->GetChildIndexByEntityId(EntityId, cur_index))
    {
    return this->GetEntityPropertyByChildIndex(cur_index);
    }
  return NULL;
}

//----------------------------------------------------------------------------
vtkProperty* vtkCMBModelWrapper::GetEntityPropertyByChildIndex(
  unsigned int index)
{
  if (this->HasChildMetaData(index))
    {
    vtkInformation* childInfo = this->GetChildMetaData(index);
    vtkIdType entId = static_cast<vtkIdType>(
      childInfo->Get(vtkModelEntity::UNIQUEPERSISTENTID()));
    vtkModelGeometricEntity* entity = vtkModelGeometricEntity::SafeDownCast(
      this->Model->GetModelEntity(entId));
    return entity ? entity->GetDisplayProperty() : NULL;
    }
  return NULL;
}
//----------------------------------------------------------------------------
vtkModelEntity* vtkCMBModelWrapper::GetEntityObjectByFlatIndex(
  unsigned int index)
{
  index--;
  if (this->HasChildMetaData(index))
    {
    vtkInformation* childInfo = this->GetChildMetaData(index);
    vtkIdType entId = static_cast<vtkIdType>(
      childInfo->Get(vtkModelEntity::UNIQUEPERSISTENTID()));
    return this->Model->GetModelEntity(entId);
    }
  return NULL;
}

//----------------------------------------------------------------------------
bool vtkCMBModelWrapper::GetChildIndexByEntityId(
  vtkIdType entityId, unsigned int& index)
{
  unsigned int numChildren = this->GetNumberOfChildren();
  this->SetNumberOfChildren(numChildren);
  for (unsigned int cc=0; cc < numChildren; cc++)
    {
    if (this->HasChildMetaData(cc))
      {
      vtkInformation* childInfo = this->GetChildMetaData(cc);
      vtkIdType entId = static_cast<vtkIdType>(
        childInfo->Get(vtkModelEntity::UNIQUEPERSISTENTID()));
      if(entId == entityId)
        {
        index = cc;
        return true;
        }
      }
    }

  return false;
}

//----------------------------------------------------------------------------
void vtkCMBModelWrapper::InitializeWithModelGeometry()
{
  this->Initialize();

  // Faces first, then edges, then vertex
  this->AddGeometricEntities(vtkModelFaceType);
  this->AddGeometricEntities(vtkModelEdgeType);
  this->AddGeometricEntities(vtkModelVertexType);
}

//----------------------------------------------------------------------------
void vtkCMBModelWrapper::AddGeometricEntities(int entType)
{
  std::vector<vtkModelGeometricEntity*> entities;
  vtkModelItemIterator* iter = this->Model->NewIterator(entType);
  for(iter->Begin();!iter->IsAtEnd();iter->Next())
    {
    entities.push_back(
      vtkModelGeometricEntity::SafeDownCast(iter->GetCurrentItem()));
    }
  iter->Delete();
  this->AddGeometricEntities(entities);
  entities.clear();
}

//----------------------------------------------------------------------------
void vtkCMBModelWrapper::AddGeometricEntities(std::vector<vtkIdType> &entities)
{
  std::vector<vtkModelGeometricEntity*> geoentities;
  std::vector<vtkIdType>::iterator iter;
  for(iter = entities.begin();iter != entities.end();++iter)
    {
    geoentities.push_back(vtkModelGeometricEntity::SafeDownCast(
      this->Model->GetModelEntity(*iter)));
    }
  this->AddGeometricEntities(geoentities);
  geoentities.clear();
}

//----------------------------------------------------------------------------
void vtkCMBModelWrapper::AddGeometricEntities(
  std::vector<vtkModelGeometricEntity*> &entities)
{
  int numEnt = static_cast<int>(entities.size());
  if(numEnt == 0)
    {
    return;
    }
  int numItems = this->GetNumberOfChildren();
  this->SetNumberOfChildren(numItems + numEnt);

  std::vector<vtkModelGeometricEntity* >::iterator it;
  int i=0;
  for(it = entities.begin(); it != entities.end(), i<numEnt; ++it)
    {
    vtkPolyData* Geometry = vtkPolyData::SafeDownCast(
      (*it)->GetGeometry());
    if(Geometry)
      {
      this->SetChild(numItems+i, Geometry);
      this->GetChildMetaData(numItems+i)->Set(
        vtkModelEntity::UNIQUEPERSISTENTID(), (*it)->GetUniquePersistentId());
      this->GetEntityPropertyByChildIndex(numItems+i);
      i++;
      }
    }
}

//----------------------------------------------------------------------------
void vtkCMBModelWrapper::ModelEntitySetGeometryCallback(vtkObject *caller,
  unsigned long event, void *clientData, void *callData)
{
  vtkDiscreteModel* model = reinterpret_cast<vtkDiscreteModel*>(caller);
  vtkModelGeometricEntity* modelEntity = reinterpret_cast<vtkModelGeometricEntity*>(callData);

  vtkCMBModelWrapper* self = reinterpret_cast<vtkCMBModelWrapper *>( clientData );
  if(!self || (!model && !modelEntity))
    {
    //vtkWarningMacro("vtkSBDomainSetContainer::CMBModelChangedCallback, Could not get the model entity.");
    return;
    }

  switch(event)
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
    if(self->GetChildIndexByEntityId(
      modelEntity->GetUniquePersistentId(), childIdx))
      {
      self->RemoveChild(childIdx);
      self->Modified();
      }
    }
    break;
  default :
    break;
  }
}
