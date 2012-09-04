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
  vtkModelItemIterator* Faces = this->Model->NewIterator(vtkModelFaceType);
  vtkModelFaceUse* FaceUse = 0;
  for(Faces->Begin();!Faces->IsAtEnd() && FaceUse == 0;Faces->Next())
    {
    vtkModelFace* Face = vtkModelFace::SafeDownCast(Faces->GetCurrentItem());
    FaceUse = vtkModelFaceUse::SafeDownCast(
      Face->GetModelEntity(vtkModelFaceUseType, associatedModelFaceUse[0]));
    }
  Faces->Delete();
  if(!FaceUse)
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
  FaceUse->AddLoopUse(loopUse);
  loopUse->Delete();

  for(size_t i=0;i<associatedModelEdgeUses.size();i++)
    {
    // get edge use adjacencies
    vtksys_ios::ostringstream idstr;
    idstr << associatedModelEdgeUses[i] << ends;
    vtkXMLElement* EdgeUseElement = 
      this->RootElement->FindNestedElement(idstr.str().c_str());
    std::vector<vtkIdType> EdgeUseEdge;
    this->GetAssociations(EdgeUseElement->FindNestedElementByName("Associations"),
                          vtkModelEdgeType, EdgeUseEdge);
    if(EdgeUseEdge.size() != 1)
      {
      vtkErrorMacro("Could not find edge of edge use.");
      }
    vtkModelEdge* Edge = vtkModelEdge::SafeDownCast(
      this->Model->GetModelEntity(vtkModelEdgeType, EdgeUseEdge[0]));
    vtkModelEntity* EdgeUse = Edge->GetModelEntity(vtkModelEdgeUseType,
                                                   associatedModelEdgeUses[i]);
    loopUse->AddAssociation(EdgeUse->GetType(), EdgeUse);
    }

  return loopUse;
}

vtkModelFace* vtkXMLModelReader::ConstructModelFace(int id)
{
  std::vector<vtkIdType> ModelFaceUses;
  this->GetAssociations(this->CurrentElement->FindNestedElementByName("Associations"), 
                        vtkModelFaceUseType, ModelFaceUses);
  if(ModelFaceUses.size() != 2)
    {
    vtkErrorMacro("Model face does not have two uses.");
    return 0;
    }
  vtkDiscreteModelFace* ModelFace = vtkDiscreteModelFace::New();
  this->Model->AddAssociation(ModelFace->GetType(), ModelFace);
  ModelFace->Delete();
  ModelFace->SetUniquePersistentId(id);
  ModelFace->GetModelFaceUse(0)->SetUniquePersistentId(ModelFaceUses[0]);
  ModelFace->GetModelFaceUse(1)->SetUniquePersistentId(ModelFaceUses[1]);

  // if this model face is not adjacent to any model regions then 
  // there will be a material associated with it
  std::vector<vtkIdType> Materials;
  this->GetAssociations(this->CurrentElement->FindNestedElementByName("Associations"), 
                        vtkModelMaterialType, Materials);
  if(Materials.size() == 1)
    {
    vtkModelMaterial* Material = vtkModelMaterial::SafeDownCast(
      this->Model->GetModelEntity(vtkModelMaterialType, Materials[0]));
    Material->AddModelGeometricEntity(ModelFace);
    }

  return ModelFace;
}

vtkModelFaceUse* vtkXMLModelReader::ConstructModelFaceUse(int id)
{
  vtkModelFaceUse* FaceUse = 0;
  std::vector<vtkIdType> associatedModelFace;
  this->GetAssociations(this->CurrentElement->FindNestedElementByName("Associations"), 
                        vtkModelFaceType, associatedModelFace);
  if(associatedModelFace.size() != 1)
    {
    vtkErrorMacro("ModelFaceUse has incorrect number of adjacent model faces.");
    return 0;
    }
  vtkModelFace* Face = vtkModelFace::SafeDownCast(
    this->Model->GetModelEntity(vtkModelFaceType, associatedModelFace[0]));
  if(!Face)
    {
    vtkErrorMacro("ModelFace is not yet available for needed ModelFaceUse.");
    return 0;
    }
  if(Face->GetModelFaceUse(0)->GetUniquePersistentId() == id)
    {
    FaceUse = Face->GetModelFaceUse(0);
    }
  else if(Face->GetModelFaceUse(1)->GetUniquePersistentId() == id)
    {
    FaceUse = Face->GetModelFaceUse(1);
    }
  else
    {
    vtkErrorMacro("ModelFaceUse associated with wrong model face.");
    return 0;
    }
  return FaceUse;
}

vtkModelRegion* vtkXMLModelReader::ConstructModelRegion(int id)
{
  vtkModelRegion* Region = 0;
  std::map<int, std::vector<vtkIdType> > Associations;
  this->GetAssociations(this->CurrentElement->FindNestedElementByName("Associations"),
                        Associations);
  if(Associations[vtkModelMaterialType].size() != 1 || 
     Associations[vtkModelShellUseType].size() != 1)
    {
    // Currently serialization only supports a single shell
    vtkErrorMacro("Model region has wrong associations.");
    return 0;
    }
  vtkModelMaterial* Material = vtkModelMaterial::SafeDownCast(
    this->Model->GetModelEntity(vtkModelMaterialType, Associations[vtkModelMaterialType][0]));
  
  // get shell use adjacencies
  vtksys_ios::ostringstream idstr;
  idstr << Associations[vtkModelShellUseType][0] << ends;        
  vtkXMLElement* shellElement = this->RootElement->FindNestedElement(idstr.str().c_str());
  std::vector<vtkIdType> ShellFaceUses;
  this->GetAssociations(shellElement->FindNestedElementByName("Associations"),
                        vtkModelFaceUseType, ShellFaceUses);
  size_t NumFaces = ShellFaceUses.size();
  std::vector<vtkModelFace*> Faces(NumFaces);
  std::vector<int> FaceSides(NumFaces);
  for(size_t j=0;j<NumFaces;j++)
    {
    vtkModelFaceUse* FaceUse = 
      vtkModelFaceUse::SafeDownCast(this->Model->GetModelEntity(ShellFaceUses[j]));
    Faces[j] = FaceUse->GetModelFace();
    FaceSides[j] = (Faces[j]->GetModelFaceUse(0) == FaceUse) ? 0 : 1;
    }
  
   Region = this->Model->BuildModelRegion(NumFaces, &Faces[0], &FaceSides[0], 
                                          id, Material);
   vtkModelItemIterator* ShellUses = Region->NewModelShellUseIterator();
   int Counter = 0;
   for(ShellUses->Begin();!ShellUses->IsAtEnd();ShellUses->Next(),Counter++)
     {
     vtkModelShellUse* ShellUse = 
       vtkModelShellUse::SafeDownCast(ShellUses->GetCurrentItem());
     ShellUse->SetUniquePersistentId(Associations[vtkModelShellUseType][0]);
     }
   ShellUses->Delete();

   std::vector<vtkIdType> FloatingEdges;
   this->GetAssociations(this->CurrentElement->FindNestedElementByName("Associations"),
                      vtkModelEdgeType, FloatingEdges);
   if(FloatingEdges.size() != 0)
     {
     vtkModelItem* Edge = this->Model->GetModelEntity(vtkModelEdgeType, FloatingEdges[0]);
     Region->AddAssociation(Edge->GetType(), Edge);
     }
   return Region;
}

vtkModelVertex* vtkXMLModelReader::ConstructModelVertex(int id)
{
  vtkModelVertex* Vertex = this->Model->BuildModelVertex(-2, id);

  // add in the vertex uses here so that we keep the order
  std::vector<vtkIdType> AssociatedVertexUses;
  this->GetAssociations(this->CurrentElement->FindNestedElementByName("Associations"),
                        vtkModelVertexUseType, AssociatedVertexUses);
  for(std::vector<vtkIdType>::iterator it=AssociatedVertexUses.begin();
      it!=AssociatedVertexUses.end();it++)
    {
    vtkModelVertexUse* VertexUse = Vertex->BuildModelVertexUse();
    VertexUse->SetUniquePersistentId(*it);
    }
  return Vertex;
}

vtkModelVertexUse* vtkXMLModelReader::ConstructModelVertexUse(int id)
{
  std::vector<vtkIdType> AssociatedVertex;
  this->GetAssociations(this->CurrentElement->FindNestedElementByName("Associations"),
                        vtkModelVertexType, AssociatedVertex);
  if(AssociatedVertex.size() != 1)
    {
    vtkErrorMacro("Could not find proper vertex for vertex use.");
    return 0;
    }
  vtkModelVertex* Vertex = vtkModelVertex::SafeDownCast(
    this->Model->GetModelEntity(vtkModelVertexType, AssociatedVertex[0]));
  vtkModelVertexUse* VertexUse = vtkModelVertexUse::SafeDownCast(
    Vertex->GetModelEntity(vtkModelVertexUseType, id));

  return VertexUse;
}

vtkModelEdge* vtkXMLModelReader::ConstructModelEdge(int id)
{
  std::map<int, std::vector<vtkIdType> > Associations;
  this->GetAssociations(this->CurrentElement->FindNestedElementByName("Associations"),
                        Associations);

  vtkDiscreteModelEdge* Edge = vtkDiscreteModelEdge::New();
  Edge->SetUniquePersistentId(id);
  this->Model->AddAssociation(Edge->GetType(), Edge);
  Edge->Delete();
  // we add in the model edge uses now so that we make sure
  // to get them in in the proper order
  for(std::vector<vtkIdType>::iterator it=Associations[vtkModelEdgeUseType].begin();
      it!=Associations[vtkModelEdgeUseType].end();it++)
    { // iterate through the edge uses
    vtkModelEdgeUse* EdgeUse = vtkModelEdgeUse::New();
    EdgeUse->SetUniquePersistentId(*it);
    Edge->AddAssociation(EdgeUse->GetType(), EdgeUse);
    EdgeUse->Delete();
    }

  return Edge;
}

vtkModelEdgeUse* vtkXMLModelReader::ConstructModelEdgeUse(int id)
{
  std::map<int, std::vector<vtkIdType> > Associations;
  this->GetAssociations(this->CurrentElement->FindNestedElementByName("Associations"),
                        Associations);
  if(Associations[vtkModelEdgeType].size() != 1)
    {
    vtkErrorMacro("Model edge use has wrong associations.");
    return 0;
    }
  
  vtkModelEntity* Edge = this->Model->GetModelEntity(vtkModelEdgeType, 
                                                     Associations[vtkModelEdgeType][0]);
  vtkModelItemIterator* EdgeUses = Edge->NewIterator(vtkModelEdgeUseType);
  vtkModelEdgeUse* EdgeUse = 0;
  for(EdgeUses->Begin();!EdgeUses->IsAtEnd()&&EdgeUse==0;EdgeUses->Next())
    {
    vtkModelEdgeUse* tmp = vtkModelEdgeUse::SafeDownCast(EdgeUses->GetCurrentItem());
    if(tmp->GetUniquePersistentId() == id)
      {
      EdgeUse = tmp;
      }
    }
  EdgeUses->Delete();
  
  // hook up the edge use pair
  vtkIdType EdgeUsePairId = Associations[vtkModelEdgeUseType][0];
  if(EdgeUse->GetPairedModelEdgeUse() == 0)
    {
    vtkModelEntity* EdgeUsePair = Edge->GetModelEntity(vtkModelEdgeUseType,
                                                       EdgeUsePairId);
    if(EdgeUsePair)
      {
      EdgeUse->AddAssociation(EdgeUsePair->GetType(), EdgeUsePair);
      }
    }

  // now hook up the vertex uses to this edge use
  for(size_t i=0;i<Associations[vtkModelVertexUseType].size();i++)
    {
    // get vertex use adjacencies
    vtksys_ios::ostringstream idstr;
    idstr << Associations[vtkModelVertexUseType][i] << ends;        
    vtkXMLElement* VertexUseElement = 
      this->RootElement->FindNestedElement(idstr.str().c_str());
    std::vector<vtkIdType> VertexId;
    this->GetAssociations(VertexUseElement->FindNestedElementByName("Associations"),
                          vtkModelVertexType, VertexId);
    vtkModelEntity* Vertex = this->Model->GetModelEntity(vtkModelVertexType, VertexId[0]);
    vtkModelVertexUse* VertexUse = vtkModelVertexUse::SafeDownCast(
      Vertex->GetModelEntity(vtkModelVertexUseType, Associations[vtkModelVertexUseType][i]));
    if(VertexUse == 0)
      {
      vtkErrorMacro("Could not find vertex use for edge use.");
      return 0;
      }
    EdgeUse->AddAssociation(VertexUse->GetType(), VertexUse);
    }

  

  return EdgeUse;
}

vtkModelShellUse* vtkXMLModelReader::ConstructModelShellUse(int id)
{
  std::vector<vtkIdType> AssociatedRegion;
  this->GetAssociations(this->CurrentElement->FindNestedElementByName("Associations"),
                        vtkModelRegionType, AssociatedRegion);
  if(AssociatedRegion.size() != 1)
    {
    vtkErrorMacro("ModelRegion is not yet available for needed ModelShellUse.");
    return 0;
    }
  vtkModelShellUse* ShellUse = 0;
  vtkModelItemIterator* iter = vtkDiscreteModelRegion::SafeDownCast(
    this->Model->GetModelEntity(vtkModelRegionType, AssociatedRegion[0]))
    ->NewModelShellUseIterator();
  for(iter->Begin();!iter->IsAtEnd()&&!ShellUse;iter->Next())
    {
    if(id==vtkModelShellUse::SafeDownCast(iter->GetCurrentItem())->GetUniquePersistentId())
      {
      ShellUse = vtkModelShellUse::SafeDownCast(iter->GetCurrentItem());
      }
    }
  iter->Delete();
  if(!ShellUse)
    {
    vtkErrorMacro("Model shell use cannot find the proper region.");
    return 0;
    }
  return ShellUse;
}

vtkDiscreteModelEntityGroup* vtkXMLModelReader::ConstructModelEntityGroup(int id)
{
  std::map<int, std::vector<vtkIdType> > Associations;
  this->GetAssociations(this->CurrentElement->FindNestedElementByName("Associations"),
                        Associations);
  // the entity group should only contain one type of entity
  // so get rid of the association to the model to get it
  Associations.erase(this->Model->GetType());
  if(Associations.size() == 1)
    {
    std::map<int, std::vector<vtkIdType> >::iterator it=
      Associations.begin();
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
  else if(Associations.size() ==0)
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
