//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkXMLModelWriter.h"

#include "vtkInformation.h"
#include "vtkInformationDoubleKey.h"
#include "vtkInformationDoubleVectorKey.h"
#include "vtkInformationIdTypeKey.h"
#include "vtkInformationIntegerKey.h"
#include "vtkInformationIntegerVectorKey.h"
#include "vtkInformationIterator.h"
#include "vtkInformationKeyMap.h"
#include "vtkInformationObjectBaseKey.h"
#include "vtkInformationStringKey.h"
#include "vtkModel.h"
#include "vtkModelEntity.h"
#include "vtkObjectFactory.h"
#include "vtkXMLElement.h"

#include <list>
#include <map>
#include <sstream>

vtkStandardNewMacro(vtkXMLModelWriter);

vtkCxxSetObjectMacro(vtkXMLModelWriter, RootElement, vtkXMLElement);

struct vtkXMLModelWriterInternals
{
  vtkXMLModelWriterInternals() {}

  void Push(vtkXMLElement* elem) { this->Stack.push_front(elem); }

  void Pop() { this->Stack.pop_front(); }

  std::list<vtkXMLElement*> Stack;
  std::map<vtkSerializableObject*, vtkIdType> IDs;
};

vtkXMLModelWriter::vtkXMLModelWriter()
{
  this->Internal = new vtkXMLModelWriterInternals;
  this->RootElement = 0;
}

vtkXMLModelWriter::~vtkXMLModelWriter()
{
  delete this->Internal;
  this->SetRootElement(0);
}

void vtkXMLModelWriter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

namespace
{
vtkSmartPointer<vtkXMLElement> BaseSerialize(const char* name, vtkXMLModelWriterInternals* internal)
{
  vtkXMLElement* root = internal->Stack.front();
  if (!root)
  {
    vtkGenericWarningMacro("Serialize cannot be called before Initialize()");
    return 0;
  }

  vtkSmartPointer<vtkXMLElement> elem = vtkSmartPointer<vtkXMLElement>::New();
  elem->SetName(name);
  root->AddNestedElement(elem);

  return elem;
}
}

void vtkXMLModelWriter::Serialize(const char* name, vtkObject*& obj, bool /*weakPtr*/)
{

  vtkSmartPointer<vtkXMLElement> elem = BaseSerialize(name, this->Internal);
  if (!elem.GetPointer())
  {
    return;
  }

  elem->AddAttribute("type", "Pointer");
  vtkSerializableObject* serializableObject = vtkSerializableObject::SafeDownCast(obj);
  if (serializableObject)
  {
    elem->AddAttribute("to_id", this->Serialize(serializableObject));
  }
}

vtkIdType vtkXMLModelWriter::Serialize(vtkSerializableObject*& obj)
{
  if (!obj)
  {
    return -2;
  }

  std::map<vtkSerializableObject*, vtkIdType>::iterator iter = this->Internal->IDs.find(obj);
  if (iter != this->Internal->IDs.end())
  {
    return iter->second;
  }

  if (!this->RootElement)
  {
    vtkErrorMacro("Cannot call serialize before Initialize()");
    return 0;
  }
  vtkSmartPointer<vtkXMLElement> newElem = vtkSmartPointer<vtkXMLElement>::New();
  newElem->SetName("Object");
  newElem->AddAttribute("type", obj->GetClassName());
  vtkModelEntity* modelEntity = vtkModelEntity::SafeDownCast(obj);
  vtkIdType id = -2;
  if (modelEntity)
  {
    id = modelEntity->GetUniquePersistentId();
  }
  else if (vtkModel::SafeDownCast(obj))
  {
    id = -1; // model entities start at 0 so we use -1 here to be unique
  }
  else
  {
    vtkErrorMacro("Wrong type of object.");
  }
  newElem->AddAttribute("id", id);
  this->Internal->IDs[obj] = id;
  this->RootElement->AddNestedElement(newElem);
  this->Internal->Push(newElem);
  obj->Serialize(this);
  this->Internal->Pop();
  return id;
}

namespace
{
template <typename KeyType, typename ValueType>
void SerializeScalarKey(
  vtkInformation* info, KeyType* key, const char* keyName, vtkXMLElement* parent)
{
  vtkSmartPointer<vtkXMLElement> keyElem = vtkSmartPointer<vtkXMLElement>::New();
  keyElem->SetName(keyName);
  parent->AddNestedElement(keyElem);
  ValueType val = info->Get(key);
  keyElem->AddAttribute("value", val);
}

template <typename KeyType, typename ValueType>
void SerializeVectorKey(
  vtkInformation* info, KeyType* key, const char* keyName, vtkXMLElement* parent)
{
  vtkSmartPointer<vtkXMLElement> keyElem = vtkSmartPointer<vtkXMLElement>::New();
  keyElem->SetName(keyName);
  parent->AddNestedElement(keyElem);
  ValueType* vals = info->Get(key);
  int length = info->Length(key);
  keyElem->AddAttribute("values", vals, length);
  keyElem->AddAttribute("length", length);
}
}

void vtkXMLModelWriter::Serialize(const char* name, vtkInformation* info)
{
  vtkSmartPointer<vtkXMLElement> elem = BaseSerialize(name, this->Internal);
  if (!elem.GetPointer())
  {
    return;
  }
  elem->AddAttribute("type", info->GetClassName());

  vtkInformationIterator* iter = vtkInformationIterator::New();
  iter->SetInformation(info);
  // the map below is to alphabetize based on keystr so that we
  // consistently get the same ordering
  std::map<std::string, vtkInformationKey*> keystrToKey;
  for (iter->GoToFirstItem(); !iter->IsDoneWithTraversal(); iter->GoToNextItem())
  {
    vtkInformationKey* key = iter->GetCurrentKey();
    std::string keystr = vtkInformationKeyMap::GetFullName(key);
    if (keystrToKey.find(keystr) != keystrToKey.end())
    {
      vtkErrorMacro("Got non-unique key string in serialization.");
    }
    keystrToKey[keystr] = key;
  }

  for (std::map<std::string, vtkInformationKey*>::iterator it = keystrToKey.begin();
       it != keystrToKey.end(); it++)
  {
    vtkInformationKey* key = it->second;
    if (key->IsA("vtkInformationIntegerKey"))
    {
      SerializeScalarKey<vtkInformationIntegerKey, int>(
        info, static_cast<vtkInformationIntegerKey*>(key), it->first.c_str(), elem);
    }
    else if (key->IsA("vtkInformationDoubleKey"))
    {
      SerializeScalarKey<vtkInformationDoubleKey, double>(
        info, static_cast<vtkInformationDoubleKey*>(key), it->first.c_str(), elem);
    }
    else if (key->IsA("vtkInformationIdTypeKey"))
    {
      SerializeScalarKey<vtkInformationIdTypeKey, vtkIdType>(
        info, static_cast<vtkInformationIdTypeKey*>(key), it->first.c_str(), elem);
    }
    else if (key->IsA("vtkInformationStringKey"))
    {
      SerializeScalarKey<vtkInformationStringKey, const char*>(
        info, static_cast<vtkInformationStringKey*>(key), it->first.c_str(), elem);
    }
    else if (key->IsA("vtkInformationDoubleVectorKey"))
    {
      SerializeVectorKey<vtkInformationDoubleVectorKey, double>(
        info, static_cast<vtkInformationDoubleVectorKey*>(key), it->first.c_str(), elem);
    }
    else if (key->IsA("vtkInformationIntegerVectorKey"))
    {
      SerializeVectorKey<vtkInformationIntegerVectorKey, int>(
        info, static_cast<vtkInformationIntegerVectorKey*>(key), it->first.c_str(), elem);
    }
    else if (key->IsA("vtkInformationObjectBaseKey"))
    {
      vtkSerializableObject* ptr = vtkSerializableObject::SafeDownCast(
        info->Get(static_cast<vtkInformationObjectBaseKey*>(key)));
      if (ptr)
      {
        vtkSmartPointer<vtkXMLElement> keyElem = vtkSmartPointer<vtkXMLElement>::New();
        keyElem->SetName(it->first.c_str());
        elem->AddNestedElement(keyElem);
        keyElem->AddAttribute("to_id", this->Serialize(ptr));
      }
    }
  }
  iter->Delete();
}

void vtkXMLModelWriter::Serialize(
  const char* name, std::vector<vtkSmartPointer<vtkObject> >& objs, bool /*weakPtr*/)
{
  vtkSmartPointer<vtkXMLElement> elem = BaseSerialize(name, this->Internal);
  if (!elem.GetPointer())
  {
    return;
  }
  elem->AddAttribute("type", "vtkSerializableObjectVector");

  this->Internal->Push(elem);
  std::vector<vtkSmartPointer<vtkObject> >::iterator iter = objs.begin();
  for (; iter != objs.end(); iter++)
  {
    vtkObject* obj = iter->GetPointer();
    this->Serialize("Item", obj);
  }
  this->Internal->Pop();
}

void vtkXMLModelWriter::Serialize(
  const char* name, std::map<int, std::vector<vtkSmartPointer<vtkObject> > >& map)
{
  vtkSmartPointer<vtkXMLElement> elem = BaseSerialize(name, this->Internal);
  if (!elem.GetPointer())
  {
    return;
  }
  elem->AddAttribute("type", "vtkSerializableObjectVectorMap");

  this->Internal->Push(elem);
  std::map<int, std::vector<vtkSmartPointer<vtkObject> > >::iterator iter = map.begin();
  for (; iter != map.end(); iter++)
  {
    std::vector<vtkSmartPointer<vtkObject> >& objs = iter->second;
    std::ostringstream str;
    str << "Key_" << iter->first;
    this->Serialize(str.str().c_str(), objs);
  }
  this->Internal->Pop();
}

void vtkXMLModelWriter::Initialize(const char* name)
{
  this->RootElement = vtkXMLElement::New();
  this->RootElement->SetName(name);
  this->RootElement->AddAttribute("version", this->GetArchiveVersion());
}

vtkXMLElement* vtkXMLModelWriter::CreateDOM(
  const char* rootName, std::vector<vtkSmartPointer<vtkObject> >& objs)
{
  this->Initialize(rootName);

  vtkSmartPointer<vtkXMLElement> elem = vtkSmartPointer<vtkXMLElement>::New();
  elem->SetName("RootObjects");
  this->RootElement->AddNestedElement(elem);

  this->Internal->Push(elem);
  std::vector<vtkSmartPointer<vtkObject> >::iterator iter = objs.begin();
  for (; iter != objs.end(); iter++)
  {
    vtkObject* obj = iter->GetPointer();
    this->Serialize("Item", obj);
  }
  this->Internal->Pop();
  return this->RootElement;
}

void vtkXMLModelWriter::Serialize(
  std::ostringstream& ostr, const char* rootName, std::vector<vtkSmartPointer<vtkObject> >& objs)
{
  if (this->Internal)
  {
    delete this->Internal;
  }
  this->Internal = new vtkXMLModelWriterInternals;

  vtkXMLElement* re = this->CreateDOM(rootName, objs);
  re->PrintXML(ostr, vtkIndent());
  this->SetRootElement(0);
  delete this->Internal;
  this->Internal = 0;
}

// -------------integers---------------
void vtkXMLModelWriter::Serialize(const char* name, int& val)
{
  vtkSmartPointer<vtkXMLElement> elem = BaseSerialize(name, this->Internal);
  if (!elem.GetPointer())
  {
    return;
  }

  elem->AddAttribute("value", val);
}

void vtkXMLModelWriter::Serialize(const char* name, int*& val, unsigned int& length)
{
  vtkSmartPointer<vtkXMLElement> elem = BaseSerialize(name, this->Internal);
  if (!elem.GetPointer())
  {
    return;
  }

  if (!val)
  {
    elem->AddAttribute("length", static_cast<int>(0));
    return;
  }

  elem->AddAttribute("length", length);
  elem->AddAttribute("values", val, length);
}

// -------------unsigned longs---------------
void vtkXMLModelWriter::Serialize(const char* name, unsigned long& val)
{
  vtkSmartPointer<vtkXMLElement> elem = BaseSerialize(name, this->Internal);
  if (!elem.GetPointer())
  {
    return;
  }

  elem->AddAttribute("value", val);
}

void vtkXMLModelWriter::Serialize(const char* name, unsigned long*& val, unsigned int& length)
{
  vtkSmartPointer<vtkXMLElement> elem = BaseSerialize(name, this->Internal);
  if (!elem.GetPointer())
  {
    return;
  }

  if (!val)
  {
    elem->AddAttribute("length", static_cast<unsigned long>(0));
    return;
  }

  elem->AddAttribute("length", length);
  elem->AddAttribute("values", val, length);
}

// -------------vtkIdTypes---------------
#if defined(VTK_USE_64BIT_IDS)
void vtkXMLModelWriter::Serialize(const char* name, vtkIdType& val)
{
  vtkSmartPointer<vtkXMLElement> elem = BaseSerialize(name, this->Internal);
  if (!elem.GetPointer())
  {
    return;
  }

  elem->AddAttribute("value", val);
}

void vtkXMLModelWriter::Serialize(const char* name, vtkIdType*& val, unsigned int& length)
{
  vtkSmartPointer<vtkXMLElement> elem = BaseSerialize(name, this->Internal);
  if (!elem.GetPointer())
  {
    return;
  }

  if (!val)
  {
    elem->AddAttribute("length", static_cast<int>(0));
    return;
  }

  elem->AddAttribute("length", length);
  elem->AddAttribute("values", val, length);
}
#endif // if defined(VTK_USE_64BIT_IDS)

// -------------doubles---------------
void vtkXMLModelWriter::Serialize(const char* name, double& val)
{
  vtkSmartPointer<vtkXMLElement> elem = BaseSerialize(name, this->Internal);
  if (!elem.GetPointer())
  {
    return;
  }

  elem->AddAttribute("value", val);
}

void vtkXMLModelWriter::Serialize(const char* name, double*& val, unsigned int& length)
{
  vtkSmartPointer<vtkXMLElement> elem = BaseSerialize(name, this->Internal);
  if (!elem.GetPointer())
  {
    return;
  }

  if (!val)
  {
    elem->AddAttribute("length", static_cast<int>(0));
    return;
  }

  elem->AddAttribute("length", length);
  elem->AddAttribute("values", val, length);
}

void vtkXMLModelWriter::Serialize(const char* name, char*& str)
{
  vtkSmartPointer<vtkXMLElement> elem = BaseSerialize(name, this->Internal);
  if (!elem.GetPointer())
  {
    return;
  }

  if (str)
  {
    elem->AddAttribute("value", str);
  }
}

void vtkXMLModelWriter::Serialize(const char* name, std::string& str)
{
  vtkSmartPointer<vtkXMLElement> elem = BaseSerialize(name, this->Internal);
  if (!elem.GetPointer())
  {
    return;
  }

  if (str.c_str())
  {
    elem->AddAttribute("value", str.c_str());
  }
}
