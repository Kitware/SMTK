/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkXMLModelReader.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

  =========================================================================*/
#include "vtkXMLModelReader.h"

#include "vtkModelMaterial.h"
#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelEdge.h"
#include "vtkDiscreteModelEntityGroup.h"
#include "vtkDiscreteModelFace.h"
#include "vtkDiscreteModelRegion.h"
#include "vtkDiscreteModelVertex.h"
#
#include "vtkModelUniqueNodalGroup.h"
#include "vtkModelUserName.h"
#include "vtkCollection.h"
#include "vtkInformation.h"
#include "vtkInformationDoubleKey.h"
#include "vtkInformationIdTypeKey.h"
#include "vtkInformationIntegerKey.h"
#include "vtkInformationIntegerVectorKey.h"
#include "vtkInformationStringKey.h"
#include "vtkInformationDoubleVectorKey.h"
#include "vtkInformationKeyMap.h"
#include "vtkInformationObjectBaseKey.h"
#include "vtkInstantiator.h"
#include "vtkXMLElement.h"
#include "vtkModelXMLParser.h"
#
#include "vtkModelEdgeUse.h"
#include "vtkModelFaceUse.h"
#include "vtkModelItemIterator.h"
#include "vtkModelLoopUse.h"
#include "vtkModelShellUse.h"
#include "vtkModelVertexUse.h"
#include "vtkObjectFactory.h"
#include "vtkSmartPointer.h"
#include "vtkType.h"
#
#include <map>
#include "vtksys/ios/sstream"
#include <vtksys/SystemTools.hxx>


vtkStandardNewMacro(vtkXMLModelReader);

vtkCxxSetObjectMacro(vtkXMLModelReader, RootElement, vtkXMLElement);
vtkCxxSetObjectMacro(vtkXMLModelReader, Model, vtkDiscreteModel);

namespace
{

  template <typename KeyType, typename ValueType>
  void SerializeScalarKey(vtkInformation* info, KeyType* key, vtkXMLElement* elem)
  {
    ValueType val;
    if (elem->GetScalarAttribute("value", &val))
      {
      info->Set(key, val);
      }
  }

  template <typename KeyType, typename ValueType>
  void SerializeVectorKey(vtkInformation* info, KeyType* key, vtkXMLElement* elem)
  {
    int length;
    if (!elem->GetScalarAttribute("length", &length))
      {
      return;
      }
    if (length > 0)
      {
      ValueType* vals = new ValueType[length];
      if (elem->GetVectorAttribute("values", length, vals))
        {
        info->Set(key, vals, length);
        }
      delete[] vals;
      }
  }
}

vtkXMLModelReader::vtkXMLModelReader()
{
  // Register keys with the map. This makes it possible for the
  // archiver to access this key when reading an archive
  vtkInformationKeyMap::RemoveAllKeys();
  vtkInformationKeyMap::RegisterKey(vtkModelEntity::UNIQUEPERSISTENTID());
  vtkInformationKeyMap::RegisterKey(vtkModelUserName::USERNAME());
  vtkInformationKeyMap::RegisterKey(vtkModelEntity::COLOR());
  vtkInformationKeyMap::RegisterKey(vtkDiscreteModelRegion::POINTINSIDE());
  vtkInformationKeyMap::RegisterKey(vtkModelEntity::VISIBILITY());
  vtkInformationKeyMap::RegisterKey(vtkDiscreteModelEdge::LINERESOLUTION());
  vtkInformationKeyMap::RegisterKey(vtkModelEdgeUse::DIRECTION());
  vtkInformationKeyMap::RegisterKey(vtkDiscreteModelVertex::POINTID());
  vtkInformationKeyMap::RegisterKey(vtkDiscreteModelRegion::SOLIDFILENAME());
  vtkInformationKeyMap::RegisterKey(vtkModelEntity::DISPLAY_PROPERTY());
  vtkInformationKeyMap::RegisterKey(vtkModelEntity::PICKABLE());
  vtkInformationKeyMap::RegisterKey(vtkModelEntity::USERDATA());

  this->RootElement = 0;
  this->CurrentElement = 0;
  this->Model = 0;
}

vtkXMLModelReader::~vtkXMLModelReader()
{
  this->SetRootElement(0);
  this->SetModel(0);
}

//----------------------------------------------------------------------------
void vtkXMLModelReader::GetElementsByType(
  vtkXMLElement* element, const char* type, vtkCollection* elements)
{
  if (!elements)
    {
    vtkErrorMacro("Elements cannot be NULL.");
    return;
    }
  if (!type)
    {
    vtkErrorMacro("Type cannot be NULL.");
    return;
    }

  unsigned int numChildren = element->GetNumberOfNestedElements();
  unsigned int cc;
  for (cc=0; cc < numChildren; cc++)
    {
    vtkXMLElement* child = element->GetNestedElement(cc);
    if (child && child->GetAttribute("type") &&
        strcmp(child->GetAttribute("type"), type) == 0)
      {
      elements->AddItem(child);
      }
    }

  for (cc=0; cc < numChildren; cc++)
    {
    vtkXMLElement* child = element->GetNestedElement(cc);
    if (child)
      {
      this->GetElementsByType(child, type, elements);
      }
    }
}

void vtkXMLModelReader::Serialize(istream& str, const char*)
{
  this->ParseStream(str);

  int version;
  if (this->RootElement->GetScalarAttribute("version", &version))
    {
    this->SetArchiveVersion(version);
    }

  vtkModel* model = this->GetModel();
  bool blockEvent =
    model->GetBlockModelGeometricEntityEvent();
  model->BlockModelGeometricEntityEventOn();

  this->SerializeModel();
  this->Serialize("vtkModelMaterial");
  this->Serialize("vtkDiscreteModelVertex");
  this->Serialize("vtkModelVertexUse");
  this->Serialize("vtkDiscreteModelEdge");
  this->Serialize("vtkModelEdgeUse");
  this->Serialize("vtkDiscreteModelFace");
  this->Serialize("vtkModelFaceUse");
  this->Serialize("vtkModelLoopUse");
  this->Serialize("vtkDiscreteModelRegion");
  this->Serialize("vtkModelShellUse");
  this->Serialize("vtkDiscreteModelEntityGroup");
  this->Serialize("vtkModelNodalGroup");
  this->Serialize("vtkModelUniqueNodalGroup");

  model->SetBlockModelGeometricEntityEvent(blockEvent);
  int types[4] = {vtkModelVertexType, vtkModelEdgeType,
                  vtkModelFaceType, vtkModelRegionType};
  for(int i=0;i<4;i++)
    {
    if(this->GetModel()->GetNumberOfAssociations(types[i]))
      {
      vtkModelItemIterator* entities = this->GetModel()->NewIterator(types[i]);
      for(entities->Begin();!entities->IsAtEnd();entities->Next())
        {
        this->GetModel()->InvokeModelGeometricEntityEvent(ModelGeometricEntityCreated,
                                                          entities->GetCurrentItem());
        }
      entities->Delete();
      }
    }
}

void vtkXMLModelReader::SerializeModel()
{
  vtkSmartPointer<vtkCollection> collection = vtkSmartPointer<vtkCollection>::New();
  this->GetElementsByType(this->RootElement, "vtkDiscreteModel", collection);

  if(collection->GetNumberOfItems() != 1)
    {
    vtkWarningMacro("Could not find proper vtkDiscreteModel XML element.");
    return;
    }
  this->CurrentElement = vtkXMLElement::SafeDownCast(collection->GetItemAsObject(0));
  vtkIdType id = 0;
  if(!this->CurrentElement->GetScalarAttribute("id", &id))
    {
    vtkWarningMacro("Could not get object id");
    return;
    }

  vtkDiscreteModel* model = this->GetModel();
  if(!model)
    {
    model = vtkDiscreteModel::New();
    this->SetModel(model);
    model->Delete();
    }
  model->Serialize(this);
}

void vtkXMLModelReader::Serialize(const char* ObjectName)
{
  if(!this->Model)
    {
    vtkErrorMacro("Model needs to be set before model entities can be added.");
    return;
    }
  vtkSmartPointer<vtkCollection> collection = vtkSmartPointer<vtkCollection>::New();
  this->GetElementsByType(this->RootElement, ObjectName, collection);

  for(int i=0;i<collection->GetNumberOfItems();i++)
    {
    this->CurrentElement = vtkXMLElement::SafeDownCast(collection->GetItemAsObject(i));
    vtkIdType id = 0;
    if(!this->CurrentElement->GetScalarAttribute("id", &id))
      {
      vtkWarningMacro("Could not get object id");
      continue;
      }
    vtkSerializableObject* obj;
    if(!strcmp(ObjectName, "vtkModelMaterial"))
      {
      obj = this->Model->BuildMaterial(id);
      }
    else if(!strcmp(ObjectName, "vtkDiscreteModelFace"))
      {
      obj = this->ConstructModelFace(id);
      }
    else if(!strcmp(ObjectName, "vtkModelFaceUse"))
      {
      obj = this->ConstructModelFaceUse(id);
      if(!obj)
        {
        return;
        }
      }
    else if(!strcmp(ObjectName, "vtkModelLoopUse"))
      {
      obj = this->ConstructModelLoopUse(id);
      if(!obj)
        {
        vtkErrorMacro("Could not construct model loop use.");
        return;
        }
      }
    else if(!strcmp(ObjectName, "vtkDiscreteModelRegion"))
      {
      obj = this->ConstructModelRegion(id);
      if(!obj)
        {
        return;
        }
      }
    else if(!strcmp(ObjectName, "vtkDiscreteModelEdge"))
      {
      obj = this->ConstructModelEdge(id);
      if(!obj)
        {
        return;
        }
      }
    else if(!strcmp(ObjectName, "vtkModelEdgeUse"))
      {
      obj = this->ConstructModelEdgeUse(id);
      if(!obj)
        {
        return;
        }
      }
    else if(!strcmp(ObjectName, "vtkModelShellUse"))
      {
      obj = this->ConstructModelShellUse(id);
      if(!obj)
        {
        return;
        }
      }
    else if(!strcmp(ObjectName, "vtkDiscreteModelVertex"))
      {
      obj = this->ConstructModelVertex(id);
      }
    else if(!strcmp(ObjectName, "vtkModelVertexUse"))
      {
      obj = this->ConstructModelVertexUse(id);
      if(!obj)
        {
        vtkErrorMacro("Could not construct model vertex use.");
        return;
        }
      }
    else if(!strcmp(ObjectName, "vtkDiscreteModelEntityGroup"))
      {
      obj = this->ConstructModelEntityGroup(id);
      if(!obj)
        {
        return;
        }
      }
    else if(!strcmp(ObjectName, "vtkModelNodalGroup"))
      {
      obj = this->ConstructNodalGroup(id);
      if(!obj)
        {
        return;
        }
      }
    else if(!strcmp(ObjectName, "vtkModelUniqueNodalGroup"))
      {
      obj = this->ConstructUniqueNodalGroup(id);
      if(!obj)
        {
        return;
        }
      }
    obj->Serialize(this);
    }
}

vtkModelLoopUse* vtkXMLModelReader::ConstructModelLoopUse(int id)
{
  std::vector<vtkIdType> associatedModelFaceUse;
  this->GetAssociations(this->CurrentElement->FindNestedElementByName("Associations"),
                        vtkModelFaceUseType, associatedModelFaceUse);
  if(associatedModelFaceUse.size() != 1)
    {
    vtkErrorMacro("ModelLoopUse has incorrect number of adjacent model faces uses.");
    return 0;
    }
  // because the model doesn't directly know about face uses we have to go through
  // all of the faces and then look there for the face use we want
  vtkModelItemIterator* faces = this->Model->NewIterator(vtkModelFaceType);
  vtkModelFaceUse* faceUse = 0;
  for(faces->Begin();!faces->IsAtEnd() && faceUse == 0;faces->Next())
    {
    vtkModelFace* face = vtkModelFace::SafeDownCast(faces->GetCurrentItem());
    faceUse = vtkModelFaceUse::SafeDownCast(
      face->GetModelEntity(vtkModelFaceUseType, associatedModelFaceUse[0]));
    }
  faces->Delete();
  if(!faceUse)
    {
    vtkErrorMacro("ModelFaceUse is not yet available for needed ModelLoopUse.");
    return 0;
    }
  // now build up the model edge uses
  std::vector<vtkIdType> associatedModelEdgeUses;
  this->GetAssociations(this->CurrentElement->FindNestedElementByName("Associations"),
                        vtkModelEdgeUseType, associatedModelEdgeUses);
  if(associatedModelEdgeUses.size() == 0)
    {
    vtkErrorMacro("ModelLoopUse has incorrect number of adjacent model edge uses.");
    return 0;
    }

  vtkModelLoopUse *loopUse = vtkModelLoopUse::New();
  faceUse->AddLoopUse(loopUse);
  loopUse->Delete();

  for(size_t i=0;i<associatedModelEdgeUses.size();i++)
    {
    // get edge use adjacencies
    vtksys_ios::ostringstream idstr;
    idstr << associatedModelEdgeUses[i] << ends;
    vtkXMLElement* edgeUseElement =
      this->RootElement->FindNestedElement(idstr.str().c_str());
    std::vector<vtkIdType> edgeUseEdge;
    this->GetAssociations(edgeUseElement->FindNestedElementByName("Associations"),
                          vtkModelEdgeType, edgeUseEdge);
    if(edgeUseEdge.size() != 1)
      {
      vtkErrorMacro("Could not find edge of edge use.");
      }
    vtkModelEdge* edge = vtkModelEdge::SafeDownCast(
      this->Model->GetModelEntity(vtkModelEdgeType, edgeUseEdge[0]));
    vtkModelEntity* edgeUse = edge->GetModelEntity(vtkModelEdgeUseType,
                                                   associatedModelEdgeUses[i]);
    loopUse->AddAssociation(edgeUse->GetType(), edgeUse);
    }

  return loopUse;
}

vtkModelFace* vtkXMLModelReader::ConstructModelFace(int id)
{
  std::vector<vtkIdType> modelFaceUses;
  this->GetAssociations(this->CurrentElement->FindNestedElementByName("Associations"),
                        vtkModelFaceUseType, modelFaceUses);
  if(modelFaceUses.size() != 2)
    {
    vtkErrorMacro("Model face does not have two uses.");
    return 0;
    }
  vtkDiscreteModelFace* modelFace = vtkDiscreteModelFace::New();
  this->Model->AddAssociation(modelFace->GetType(), modelFace);
  modelFace->Delete();
  modelFace->SetUniquePersistentId(id);
  modelFace->GetModelFaceUse(0)->SetUniquePersistentId(modelFaceUses[0]);
  modelFace->GetModelFaceUse(1)->SetUniquePersistentId(modelFaceUses[1]);

  // if this model face is not adjacent to any model regions then
  // there will be a material associated with it
  std::vector<vtkIdType> materials;
  this->GetAssociations(this->CurrentElement->FindNestedElementByName("Associations"),
                        vtkModelMaterialType, materials);
  if(materials.size() == 1)
    {
    vtkModelMaterial* material = vtkModelMaterial::SafeDownCast(
      this->Model->GetModelEntity(vtkModelMaterialType, materials[0]));
    material->AddModelGeometricEntity(modelFace);
    }

  return modelFace;
}

vtkModelFaceUse* vtkXMLModelReader::ConstructModelFaceUse(int id)
{
  vtkModelFaceUse* faceUse = 0;
  std::vector<vtkIdType> associatedModelFace;
  this->GetAssociations(this->CurrentElement->FindNestedElementByName("Associations"),
                        vtkModelFaceType, associatedModelFace);
  if(associatedModelFace.size() != 1)
    {
    vtkErrorMacro("ModelFaceUse has incorrect number of adjacent model faces.");
    return 0;
    }
  vtkModelFace* face = vtkModelFace::SafeDownCast(
    this->Model->GetModelEntity(vtkModelFaceType, associatedModelFace[0]));
  if(!face)
    {
    vtkErrorMacro("ModelFace is not yet available for needed ModelFaceUse.");
    return 0;
    }
  if(face->GetModelFaceUse(0)->GetUniquePersistentId() == id)
    {
    faceUse = face->GetModelFaceUse(0);
    }
  else if(face->GetModelFaceUse(1)->GetUniquePersistentId() == id)
    {
    faceUse = face->GetModelFaceUse(1);
    }
  else
    {
    vtkErrorMacro("ModelFaceUse associated with wrong model face.");
    return 0;
    }
  return faceUse;
}

vtkModelRegion* vtkXMLModelReader::ConstructModelRegion(int id)
{
  vtkModelRegion* region = 0;
  std::map<int, std::vector<vtkIdType> > associations;
  this->GetAssociations(this->CurrentElement->FindNestedElementByName("Associations"),
                        associations);
  if(associations[vtkModelMaterialType].size() != 1 ||
     associations[vtkModelShellUseType].size() != 1)
    {
    // Currently serialization only supports a single shell
    vtkErrorMacro("Model region has wrong associations.");
    return 0;
    }
  vtkModelMaterial* material = vtkModelMaterial::SafeDownCast(
    this->Model->GetModelEntity(vtkModelMaterialType, associations[vtkModelMaterialType][0]));

  // get shell use adjacencies
  vtksys_ios::ostringstream idstr;
  idstr << associations[vtkModelShellUseType][0] << ends;
  vtkXMLElement* shellElement = this->RootElement->FindNestedElement(idstr.str().c_str());
  std::vector<vtkIdType> shellFaceUses;
  this->GetAssociations(shellElement->FindNestedElementByName("Associations"),
                        vtkModelFaceUseType, shellFaceUses);
  size_t numFaces = shellFaceUses.size();
  std::vector<vtkModelFace*> faces(numFaces);
  std::vector<int> faceSides(numFaces);
  for(size_t j=0;j<numFaces;j++)
    {
    vtkModelFaceUse* faceUse =
      vtkModelFaceUse::SafeDownCast(this->Model->GetModelEntity(shellFaceUses[j]));
    faces[j] = faceUse->GetModelFace();
    faceSides[j] = (faces[j]->GetModelFaceUse(0) == faceUse) ? 0 : 1;
    }

  region = this->Model->BuildModelRegion(numFaces, &faces[0], &faceSides[0],
                                         id, material);
  vtkModelItemIterator* shellUses = region->NewModelShellUseIterator();
  int counter = 0;
  for(shellUses->Begin();!shellUses->IsAtEnd();shellUses->Next(),counter++)
    {
    vtkModelShellUse* shellUse =
      vtkModelShellUse::SafeDownCast(shellUses->GetCurrentItem());
    shellUse->SetUniquePersistentId(associations[vtkModelShellUseType][0]);
    }
  shellUses->Delete();

  std::vector<vtkIdType> floatingEdges;
  this->GetAssociations(this->CurrentElement->FindNestedElementByName("Associations"),
                        vtkModelEdgeType, floatingEdges);
  if(floatingEdges.size() != 0)
    {
    vtkModelItem* edge = this->Model->GetModelEntity(vtkModelEdgeType, floatingEdges[0]);
    region->AddAssociation(edge->GetType(), edge);
    }
  return region;
}

vtkModelVertex* vtkXMLModelReader::ConstructModelVertex(int id)
{
  vtkModelVertex* vertex = this->Model->BuildModelVertex(-2, id);

  // add in the vertex uses here so that we keep the order
  std::vector<vtkIdType> associatedVertexUses;
  this->GetAssociations(this->CurrentElement->FindNestedElementByName("Associations"),
                        vtkModelVertexUseType, associatedVertexUses);
  for(std::vector<vtkIdType>::iterator it=associatedVertexUses.begin();
      it!=associatedVertexUses.end();it++)
    {
    vtkModelVertexUse* vertexUse = vertex->BuildModelVertexUse();
    vertexUse->SetUniquePersistentId(*it);
    }
  return vertex;
}

vtkModelVertexUse* vtkXMLModelReader::ConstructModelVertexUse(int id)
{
  std::vector<vtkIdType> associatedVertex;
  this->GetAssociations(this->CurrentElement->FindNestedElementByName("Associations"),
                        vtkModelVertexType, associatedVertex);
  if(associatedVertex.size() != 1)
    {
    vtkErrorMacro("Could not find proper vertex for vertex use.");
    return 0;
    }
  vtkModelVertex* vertex = vtkModelVertex::SafeDownCast(
    this->Model->GetModelEntity(vtkModelVertexType, associatedVertex[0]));
  vtkModelVertexUse* vertexUse = vtkModelVertexUse::SafeDownCast(
    vertex->GetModelEntity(vtkModelVertexUseType, id));

  return vertexUse;
}

vtkModelEdge* vtkXMLModelReader::ConstructModelEdge(int id)
{
  std::map<int, std::vector<vtkIdType> > associations;
  this->GetAssociations(this->CurrentElement->FindNestedElementByName("Associations"),
                        associations);

  vtkDiscreteModelEdge* edge = vtkDiscreteModelEdge::New();
  edge->SetUniquePersistentId(id);
  this->Model->AddAssociation(edge->GetType(), edge);
  edge->Delete();
  // we add in the model edge uses now so that we make sure
  // to get them in in the proper order
  for(std::vector<vtkIdType>::iterator it=associations[vtkModelEdgeUseType].begin();
      it!=associations[vtkModelEdgeUseType].end();it++)
    { // iterate through the edge uses
    vtkModelEdgeUse* edgeUse = vtkModelEdgeUse::New();
    edgeUse->SetUniquePersistentId(*it);
    edge->AddAssociation(edgeUse->GetType(), edgeUse);
    edgeUse->Delete();
    }

  return edge;
}

vtkModelEdgeUse* vtkXMLModelReader::ConstructModelEdgeUse(int id)
{
  std::map<int, std::vector<vtkIdType> > associations;
  this->GetAssociations(this->CurrentElement->FindNestedElementByName("Associations"),
                        associations);
  if(associations[vtkModelEdgeType].size() != 1)
    {
    vtkErrorMacro("Model edge use has wrong associations.");
    return 0;
    }

  vtkModelEntity* edge = this->Model->GetModelEntity(vtkModelEdgeType,
                                                     associations[vtkModelEdgeType][0]);
  vtkModelItemIterator* edgeUses = edge->NewIterator(vtkModelEdgeUseType);
  vtkModelEdgeUse* edgeUse = 0;
  for(edgeUses->Begin();!edgeUses->IsAtEnd()&&edgeUse==0;edgeUses->Next())
    {
    vtkModelEdgeUse* tmp = vtkModelEdgeUse::SafeDownCast(edgeUses->GetCurrentItem());
    if(tmp->GetUniquePersistentId() == id)
      {
      edgeUse = tmp;
      }
    }
  edgeUses->Delete();

  // hook up the edge use pair
  vtkIdType edgeUsePairId = associations[vtkModelEdgeUseType][0];
  if(edgeUse->GetPairedModelEdgeUse() == 0)
    {
    vtkModelEntity* edgeUsePair = edge->GetModelEntity(vtkModelEdgeUseType,
                                                       edgeUsePairId);
    if(edgeUsePair)
      {
      edgeUse->AddAssociation(edgeUsePair->GetType(), edgeUsePair);
      }
    }

  // now hook up the vertex uses to this edge use
  for(size_t i=0;i<associations[vtkModelVertexUseType].size();i++)
    {
    // get vertex use adjacencies
    vtksys_ios::ostringstream idstr;
    idstr << associations[vtkModelVertexUseType][i] << ends;
    vtkXMLElement* vertexUseElement =
      this->RootElement->FindNestedElement(idstr.str().c_str());
    std::vector<vtkIdType> vertexId;
    this->GetAssociations(vertexUseElement->FindNestedElementByName("Associations"),
                          vtkModelVertexType, vertexId);
    vtkModelEntity* vertex = this->Model->GetModelEntity(vtkModelVertexType, vertexId[0]);
    vtkModelVertexUse* vertexUse = vtkModelVertexUse::SafeDownCast(
      vertex->GetModelEntity(vtkModelVertexUseType, associations[vtkModelVertexUseType][i]));
    if(vertexUse == 0)
      {
      vtkErrorMacro("Could not find vertex use for edge use.");
      return 0;
      }
    edgeUse->AddAssociation(vertexUse->GetType(), vertexUse);
    }

  return edgeUse;
}

vtkModelShellUse* vtkXMLModelReader::ConstructModelShellUse(int id)
{
  std::vector<vtkIdType> associatedRegion;
  this->GetAssociations(this->CurrentElement->FindNestedElementByName("Associations"),
                        vtkModelRegionType, associatedRegion);
  if(associatedRegion.size() != 1)
    {
    vtkErrorMacro("ModelRegion is not yet available for needed ModelShellUse.");
    return 0;
    }
  vtkModelShellUse* shellUse = 0;
  vtkModelItemIterator* iter = vtkDiscreteModelRegion::SafeDownCast(
    this->Model->GetModelEntity(vtkModelRegionType, associatedRegion[0]))
    ->NewModelShellUseIterator();
  for(iter->Begin();!iter->IsAtEnd()&&!shellUse;iter->Next())
    {
    if(id==vtkModelShellUse::SafeDownCast(iter->GetCurrentItem())->GetUniquePersistentId())
      {
      shellUse = vtkModelShellUse::SafeDownCast(iter->GetCurrentItem());
      }
    }
  iter->Delete();
  if(!shellUse)
    {
    vtkErrorMacro("Model shell use cannot find the proper region.");
    return 0;
    }
  return shellUse;
}

vtkDiscreteModelEntityGroup* vtkXMLModelReader::ConstructModelEntityGroup(int id)
{
  std::map<int, std::vector<vtkIdType> > associations;
  this->GetAssociations(this->CurrentElement->FindNestedElementByName("Associations"),
                        associations);
  // the entity group should only contain one type of entity
  // so get rid of the association to the model to get it
  associations.erase(this->Model->GetType());
  if(associations.size() == 1)
    {
    std::map<int, std::vector<vtkIdType> >::iterator it=
      associations.begin();
    int type = it->first;
    int numberOfEntities = it->second.size();
    std::vector<vtkDiscreteModelEntity*> entities(numberOfEntities);
    for(int j=0;j<numberOfEntities;j++)
      {
      vtkModelEntity* ent = this->Model->GetModelEntity(type, it->second[j]);
      switch(type)
        {
        case vtkModelEdgeType:
          {
          entities[j] = vtkDiscreteModelEdge::SafeDownCast(ent);
          break;
          }
        case vtkModelFaceType:
          {
          entities[j] = vtkDiscreteModelFace::SafeDownCast(ent);
          break;
          }
        case vtkModelRegionType:
          {
          entities[j] = vtkDiscreteModelRegion::SafeDownCast(ent);
          break;
          }
        default:
          {
          vtkErrorMacro("Wrong type of object in model entity group.");
          return 0;
          }
        }
      }
    return this->Model->BuildModelEntityGroup(
      type, numberOfEntities,
      numberOfEntities ? (&entities[0]) : NULL, id);
    }
  else if(associations.size() ==0)
    {
    return this->Model->BuildModelEntityGroup(
      -1, 0, NULL, id);
    }

  vtkErrorMacro("Model entity group contains more than one type.");
  return 0;
}

vtkModelNodalGroup* vtkXMLModelReader::ConstructNodalGroup(int id)
{
  // nodal groups only have an association with the model
  // and the model will take care of keeping track of that association
  // when the nodal group is built
  return this->Model->BuildNodalGroup(BASE_NODAL_GROUP, 0, id);
}

vtkModelUniqueNodalGroup* vtkXMLModelReader::ConstructUniqueNodalGroup(int id)
{
  // nodal groups only have an association with the model
  // and the model will take care of keeping track of that association
  // when the nodal group is built
  return vtkModelUniqueNodalGroup::SafeDownCast(
    this->Model->BuildNodalGroup(UNIQUE_NODAL_GROUP, 0, id));
}

void vtkXMLModelReader::Serialize(const char* name, vtkInformation* info)
{
  info->Clear();
  vtkXMLElement* elem = this->CurrentElement->FindNestedElementByName(name);
  if (!elem)
    {
    return;
    }

  unsigned int nnested = elem->GetNumberOfNestedElements();
  for(unsigned int i=0; i<nnested; i++)
    {
    vtkXMLElement* keyElem = elem->GetNestedElement(i);
    vtkInformationKey* key = vtkInformationKeyMap::FindKey(keyElem->GetName());
    if (!key)
      {
      continue;
      }

    if (key->IsA("vtkInformationIntegerKey"))
      {
      SerializeScalarKey<vtkInformationIntegerKey, int>(info,
                                                        static_cast<vtkInformationIntegerKey*>(key), keyElem);
      }
    else if (key->IsA("vtkInformationDoubleKey"))
      {
      SerializeScalarKey<vtkInformationDoubleKey, double>(info,
                                                          static_cast<vtkInformationDoubleKey*>(key), keyElem);
      }
    else if (key->IsA("vtkInformationIdTypeKey"))
      {
      SerializeScalarKey<vtkInformationIdTypeKey, vtkIdType>(info,
                                                             static_cast<vtkInformationIdTypeKey*>(key), keyElem);
      }
    else if (key->IsA("vtkInformationStringKey"))
      {
      const char* val = keyElem->GetAttribute("value");
      if (val)
        {
        info->Set(static_cast<vtkInformationStringKey*>(key), val);
        }
      }
    else if (key->IsA("vtkInformationDoubleVectorKey"))
      {
      SerializeVectorKey<vtkInformationDoubleVectorKey, double>(info,
                                                                static_cast<vtkInformationDoubleVectorKey*>(key), keyElem);
      }
    else if (key->IsA("vtkInformationIntegerVectorKey"))
      {
      SerializeVectorKey<vtkInformationIntegerVectorKey, int>(info,
                                                              static_cast<vtkInformationIntegerVectorKey*>(key), keyElem);
      }
    else if (key->IsA("vtkInformationObjectBaseKey"))
      {
      vtkIdType id;
      if (keyElem->GetScalarAttribute("to_id", &id))
        {
        vtkObject *obj = this->ReadObject(id, false);
        info->Set(static_cast<vtkInformationObjectBaseKey*>(key), obj);
        // ReadObject incremented the ReferenceCount (weakPtr = false), or
        // created the object (ReferenceCount = 1); we then stuff it in the
        // information object , which is where we were trying to get it but
        // has the side effect of further incrementing the ReferenceCount.
        // To prevent a leak we need to decrment the ReferenceCount.
        obj->UnRegister(0);
        }
      }
    }

}

void vtkXMLModelReader::Serialize(const char* name, vtkObject*& obj,
                                  bool weakPtr/*=false*/)
{
  vtkXMLElement* elem = this->CurrentElement->FindNestedElementByName(name);//BaseSerialize(this->Internal, name);
  if (!elem)
    {
    return;
    }

  obj = 0;
  vtkIdType id;
  if (elem->GetScalarAttribute("to_id", &id))
    {
    obj = this->ReadObject(id, weakPtr);
    }
}

// -------------integers---------------
void vtkXMLModelReader::Serialize(const char* name, int& val)
{
  vtkXMLElement* elem = this->CurrentElement->FindNestedElementByName(name);//BaseSerialize(this->Internal, name);
  if (!elem)
    {
    return;
    }

  elem->GetScalarAttribute("value", &val);
}

void vtkXMLModelReader::Serialize(const char* name, int*& val, unsigned int& length)
{
  vtkXMLElement* elem = this->CurrentElement->FindNestedElementByName(name);//BaseSerialize(this->Internal, name);
  if (!elem)
    {
    return;
    }
  if (val)
    {
    delete[] val;
    val = 0;
    }
  length = 0;
  elem->GetScalarAttribute("length", &length);
  if (length > 0)
    {
    val = new int[length];
    elem->GetVectorAttribute("values", length, val);
    }
}

// -------------unsigned longs---------------
void vtkXMLModelReader::Serialize(const char* name, unsigned long& val)
{
  vtkXMLElement* elem = this->CurrentElement->FindNestedElementByName(name);//BaseSerialize(this->Internal, name);
  if (!elem)
    {
    return;
    }

  elem->GetScalarAttribute("value", &val);
}

void vtkXMLModelReader::Serialize(const char* name, unsigned long*& val, unsigned int& length)
{
  vtkXMLElement* elem = this->CurrentElement->FindNestedElementByName(name);//BaseSerialize(this->Internal, name);
  if (!elem)
    {
    return;
    }
  if (val)
    {
    delete[] val;
    val = 0;
    }
  length = 0;
  elem->GetScalarAttribute("length", &length);
  if (length > 0)
    {
    val = new unsigned long[length];
    elem->GetVectorAttribute("values", length, val);
    }
}

// -------------vtkIdTypes---------------
#if defined(VTK_USE_64BIT_IDS)
void vtkXMLModelReader::Serialize(const char* name, vtkIdType& val)
{
  vtkXMLElement* elem = this->CurrentElement->FindNestedElementByName(name);//BaseSerialize(this->Internal, name);
  if (!elem)
    {
    return;
    }

  elem->GetScalarAttribute("value", &val);
}

void vtkXMLModelReader::Serialize(const char* name, vtkIdType*& val, unsigned int& length)
{
  vtkXMLElement* elem = this->CurrentElement->FindNestedElementByName(name);//BaseSerialize(this->Internal, name);
  if (!elem)
    {
    return;
    }
  if (val)
    {
    delete[] val;
    val = 0;
    }
  length = 0;
  elem->GetScalarAttribute("length", &length);
  if (length > 0)
    {
    val = new vtkIdType[length];
    elem->GetVectorAttribute("values", length, val);
    }
}
#endif // if defined(VTK_USE_64BIT_IDS)

// -------------doubles---------------
void vtkXMLModelReader::Serialize(const char* name, double& val)
{
  vtkXMLElement* elem = this->CurrentElement->FindNestedElementByName(name);//BaseSerialize(this->Internal, name);
  if (!elem)
    {
    return;
    }

  elem->GetScalarAttribute("value", &val);
}

void vtkXMLModelReader::Serialize(const char* name, double*& val, unsigned int& length)
{
  vtkXMLElement* elem = this->CurrentElement->FindNestedElementByName(name);//BaseSerialize(this->Internal, name);
  if (!elem)
    {
    return;
    }
  if (val)
    {
    delete[] val;
    val = 0;
    }
  length = 0;
  elem->GetScalarAttribute("length", &length);
  if (length > 0)
    {
    val = new double[length];
    elem->GetVectorAttribute("values", length, val);
    }
}

void vtkXMLModelReader::Serialize(const char* name, char*& str)
{
  vtkXMLElement* elem = this->CurrentElement->FindNestedElementByName(name);//BaseSerialize(this->Internal, name);
  if (!elem)
    {
    return;
    }

  str = 0;
  const char* val = elem->GetAttribute("value");
  if (val)
    {
    str = vtksys::SystemTools::DuplicateString(val);
    }
}

void vtkXMLModelReader::Serialize(const char* name, std::string& str)
{
  vtkXMLElement* elem = this->CurrentElement->FindNestedElementByName(name);//BaseSerialize(this->Internal, name);
  if (!elem)
    {
    return;
    }

  const char* val = elem->GetAttribute("value");
  if (val)
    {
    str = val;
    }
  else
    {
    str = std::string();
    }

}

void vtkXMLModelReader::Serialize(
  const char* name, std::vector<vtkSmartPointer<vtkObject> >& objs,
  bool weakPtr)
{
  objs.clear();
  vtkXMLElement* elem = this->CurrentElement->FindNestedElementByName(name);//BaseSerialize(this->Internal, name);
  if (!elem)
    {
    return;
    }

  unsigned int nnested = elem->GetNumberOfNestedElements();
  for(unsigned int i=0; i<nnested; i++)
    {
    vtkXMLElement* objElem = elem->GetNestedElement(i);
    vtkIdType to_id;
    if (objElem->GetScalarAttribute("to_id", &to_id))
      {
      vtkSerializableObject* obj = this->Model->GetModelEntity(to_id);
      if (obj)
        {
        objs.push_back(obj);
        // ReadObject incremented the ReferenceCount (weakPtr = false), or
        // created the object (ReferenceCount = 1); we then stuff it in the
        // vector , which is where we were trying to get it but has the
        // side effect of further incrementing the ReferenceCount.  To prevent
        // a leak we need to decrment the ReferenceCount.
        //obj->UnRegister(0);
        }
      }
    }

}

void vtkXMLModelReader::Serialize(
  const char* name,
  std::map<int, std::vector<vtkSmartPointer<vtkObject> > >& objs)
{
  objs.clear();
  vtkXMLElement* elem = this->CurrentElement->FindNestedElementByName(name);//BaseSerialize(this->Internal, name);
  if (!elem)
    {
    return;
    }

  unsigned int nnested = elem->GetNumberOfNestedElements();
  for(unsigned int i=0; i<nnested; i++)
    {
    vtkXMLElement* vecElem = elem->GetNestedElement(i);
    // Take out Key_ and convert to int
    int key = atoi(vecElem->GetName() + 4);
    this->Serialize(vecElem->GetName(), objs[key]);
    }
}

void vtkXMLModelReader::GetAssociations(
  vtkXMLElement* elem, std::map<int, std::vector<vtkIdType> >& objs)
{
  // this should be an Associations elemement
  objs.clear();
  unsigned int nnested = elem->GetNumberOfNestedElements();
  for(unsigned int i=0; i<nnested; i++)
    {
    vtkXMLElement* vecElem = elem->GetNestedElement(i);
    // Take out Key_ and convert to int
    int key = atoi(vecElem->GetName() + 4);
    this->GetAssociations(elem, key, objs[key]);
    }
}

void vtkXMLModelReader::GetAssociations(
  vtkXMLElement* elem, int entityType, std::vector<vtkIdType>& objs)
{
  // this should be an Associations elemement
  objs.clear();
  unsigned int nnested = elem->GetNumberOfNestedElements();
  for(unsigned int i=0; i<nnested; i++)
    {
    vtkXMLElement* vecElem = elem->GetNestedElement(i);
    // Take out Key_ and convert to int
    int key = atoi(vecElem->GetName() + 4);
    if(key == entityType)
      {
      unsigned int numObjects = vecElem->GetNumberOfNestedElements();
      for(unsigned int j=0;j<numObjects;j++)
        {
        vtkXMLElement* objectElem = vecElem->GetNestedElement(j);
        vtkIdType to_id;
        if(objectElem->GetScalarAttribute("to_id", &to_id))
          {
          objs.push_back(to_id);
          }
        }
      }
    }
}

vtkObject* vtkXMLModelReader::ReadObject(vtkIdType id, bool weakPtr)
{
  return 0;
}

int vtkXMLModelReader::ParseStream(istream& str)
{
  vtkSmartPointer<vtkModelXMLParser> parser =
    vtkSmartPointer<vtkModelXMLParser>::New();
  parser->SetStream(&str);
  parser->Parse();
  this->SetRootElement(parser->GetRootElement());
  return 1;
}

void vtkXMLModelReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "RootElement: " << this->RootElement << "\n";
  os << indent << "CurrentElement: " << this->CurrentElement << "\n";
  os << indent << "Model: " << this->Model << "\n";
}
