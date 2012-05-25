//=============================================================================
//   This file is part of VTKEdge. See vtkedge.org for more information.
//
//   Copyright (c) 2010 Kitware, Inc.
//
//   VTKEdge may be used under the terms of the BSD License
//   Please see the file Copyright.txt in the root directory of
//   VTKEdge for further information.
//
//   Alternatively, you may see: 
//
//   http://www.vtkedge.org/vtkedge/project/license.html
//
//
//   For custom extensions, consulting services, or training for
//   this or any other Kitware supported open source project, please
//   contact Kitware at sales@kitware.com.
//
//
//=============================================================================
#include "vtkXMLArchiveWriter.h"

#include "vtkInformation.h"
#include "vtkInformationIdTypeKey.h"
#include "vtkInformationIntegerKey.h"
#include "vtkInformationIntegerVectorKey.h"
#include "vtkInformationDoubleKey.h"
#include "vtkInformationStringKey.h"
#include "vtkInformationDoubleVectorKey.h"
#include "vtkInformationIterator.h"
#include "vtkInformationObjectBaseKey.h"
#include "vtkInformationObjectBaseVectorKey.h"
#include "vtkInformationKeyVectorKey.h"
#include "vtkInformationStringVectorKey.h"
#include "vtkInformationKeyMap.h"
#include "vtkSerializableObject.h"
#include "vtkSerializationHelperMap.h"
#include "vtkXMLElement.h"
#include "vtkObjectFactory.h"
#include "vtkSmartPointer.h"

#include <list>
#include <map>
#include <algorithm>
#include "vtksys/ios/sstream"

vtkCxxRevisionMacro(vtkXMLArchiveWriter, "1774");
vtkStandardNewMacro(vtkXMLArchiveWriter);

vtkCxxSetObjectMacro(vtkXMLArchiveWriter, RootElement, vtkXMLElement);

struct vtkXMLArchiveWriterInternals
{
  vtkXMLArchiveWriterInternals() : CurrentID(1) {}

  void Push(vtkXMLElement* elem)
    {
    this->Stack.push_front(elem);
    }

  void Pop()
    {
    this->Stack.pop_front();
    }

  std::list<vtkXMLElement*> Stack;
  std::map<vtkObject*, unsigned int> IDs;
  unsigned int CurrentID;
};

//----------------------------------------------------------------------------
vtkXMLArchiveWriter::vtkXMLArchiveWriter()
{
  this->Internal = new vtkXMLArchiveWriterInternals;
  vtkSerializationHelperMap::InstantiateDefaultHelpers();
  this->RootElement = 0;
}

//----------------------------------------------------------------------------
vtkXMLArchiveWriter::~vtkXMLArchiveWriter()
{
  delete this->Internal;
  this->SetRootElement(0);
}

//----------------------------------------------------------------------------
void vtkXMLArchiveWriter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

namespace
{
vtkSmartPointer<vtkXMLElement> BaseSerialize(const char* name,
  vtkXMLArchiveWriterInternals* internal)
{
  vtkXMLElement* root = internal->Stack.front();
  if (!root)
    {
    vtkGenericWarningMacro("Serialize cannot be called before setting the RootElement");
    return 0;
    }

  vtkSmartPointer<vtkXMLElement> elem =
    vtkSmartPointer<vtkXMLElement>::New();
  elem->SetName(name);
  root->AddNestedElement(elem);

  return elem;
}
}

//----------------------------------------------------------------------------
void vtkXMLArchiveWriter::Serialize(const char* name, vtkObject*& object,
                                       bool vtkNotUsed(weakPtr))
{
  vtkSmartPointer<vtkXMLElement> elem = BaseSerialize(name, this->Internal);
  if (!elem.GetPointer())
    {
    return;
    }

  elem->AddAttribute("type", "Pointer");
  if (object)
    {
    unsigned int id = this->Serialize(object);
    if (id > 0)
      {
      elem->AddAttribute("to_id", id);
      }
    }
}

//----------------------------------------------------------------------------
unsigned int vtkXMLArchiveWriter::Serialize(vtkObject*& object)
{
  if (!object)
    {
    return 0;
    }

  std::map<vtkObject*, unsigned int>::iterator iter =
    this->Internal->IDs.find(object);
  if (iter != this->Internal->IDs.end())
    {
    return iter->second;
    }

  if (!this->RootElement)
    {
    vtkGenericWarningMacro("Serialize cannot be called before setting the RootElement");
    return 0;
    }

  vtkSerializableObject *serializableObject =
    vtkSerializableObject::SafeDownCast(object);
  if (!serializableObject && !vtkInformation::SafeDownCast(object) &&
    !vtkSerializationHelperMap::IsSerializable(object))
    {
    return 0;
    }

  vtkSmartPointer<vtkXMLElement> newElem =
    vtkSmartPointer<vtkXMLElement>::New();
  newElem->SetName("Object");
  this->RootElement->AddNestedElement(newElem);

  // set the "type" attribute... may have to get from SerializationHelper
  if (serializableObject)
    {
    newElem->AddAttribute("type", serializableObject->GetClassName());
    }
  else if (!vtkInformation::SafeDownCast(object))
    {
    newElem->AddAttribute("type",
      vtkSerializationHelperMap::GetSerializationType(object));
    }
  // type for vtkInformation added inside Serialize

  unsigned int id = this->Internal->CurrentID++;
  newElem->AddAttribute("id", id);
  this->Internal->IDs[object] = id;

  this->Internal->Push(newElem);
  if (serializableObject)
    {
    serializableObject->Serialize(this);
    }
  else if (vtkInformation::SafeDownCast(object))
    {
    this->Serialize(newElem, vtkInformation::SafeDownCast(object));
    }
  else
    {
    vtkSerializationHelperMap::Serialize(object, this);
    }
  this->Internal->Pop();
  return id;
}

namespace
{
template <typename KeyType, typename ValueType>
void SerializeScalarKey(vtkInformation* info,
  KeyType* key,
  const char* keyName,
  vtkXMLElement* parent)
{
  vtkSmartPointer<vtkXMLElement> keyElem =
    vtkSmartPointer<vtkXMLElement>::New();
  keyElem->SetName(keyName);
  parent->AddNestedElement(keyElem);
  ValueType val = info->Get(key);
  keyElem->AddAttribute("value", val);
}

template <typename KeyType, typename ValueType>
void SerializeVectorKey(vtkInformation* info,
  KeyType* key,
  const char* keyName,
  vtkXMLElement* parent)
{
  vtkSmartPointer<vtkXMLElement> keyElem =
    vtkSmartPointer<vtkXMLElement>::New();
  keyElem->SetName(keyName);
  parent->AddNestedElement(keyElem);
  ValueType* vals = info->Get(key);
  unsigned int length = static_cast<unsigned int>(info->Length(key));
  keyElem->AddAttribute("values", vals, length);
  keyElem->AddAttribute("length", length);

}

void SerializeStringVectorKey(vtkInformation* info,
  vtkInformationStringVectorKey* key,
  const char* keyName,
  vtkXMLElement* parent)
{
  vtkSmartPointer<vtkXMLElement> keyElem =
    vtkSmartPointer<vtkXMLElement>::New();
  keyElem->SetName(keyName);
  parent->AddNestedElement(keyElem);

  vtksys_ios::ostringstream valueStr;
  unsigned int length = static_cast<unsigned int>(info->Length(key));
  int* lengths = new int[length];
  for (unsigned int i = 0; i < length; ++i)
    {
    if (i != 0)
      {
      valueStr << ",";
      }
    const char* tmpStr = info->Get(key, i);
    lengths[i] = static_cast<int>(strlen(tmpStr));
    valueStr << tmpStr;
    }
  keyElem->AddAttribute("values", valueStr.str().c_str());
  keyElem->AddAttribute("lengths", lengths, length);
  keyElem->AddAttribute("length", length);
  delete[] lengths;
}

void SerializeKeyVectorKey(vtkInformation* info,
  vtkInformationKeyVectorKey* key,
  const char* keyName,
  vtkXMLElement* parent)
{
  vtkSmartPointer<vtkXMLElement> keyElem =
    vtkSmartPointer<vtkXMLElement>::New();
  keyElem->SetName(keyName);
  parent->AddNestedElement(keyElem);

  vtksys_ios::ostringstream valueStr;
  unsigned int length = static_cast<unsigned int>(info->Length(key));
  for (unsigned int i = 0; i < length; ++i)
    {
    vtkInformationKey* tmpKey = info->Get(key, i);
    valueStr << vtkInformationKeyMap::GetFullName(tmpKey) << " ";
    }
  keyElem->AddAttribute("values", valueStr.str().c_str());
  keyElem->AddAttribute("length", length);
}
}

//----------------------------------------------------------------------------
void vtkXMLArchiveWriter::Serialize(const char* name, vtkInformation* info)
{
  this->Serialize(BaseSerialize(name, this->Internal), info);
}

//----------------------------------------------------------------------------
void vtkXMLArchiveWriter::Serialize(vtkXMLElement* elem, vtkInformation* info)
{
  if (!elem)
    {
    return;
    }
  elem->AddAttribute("type", info->GetClassName());

  vtkInformationIterator* iter = vtkInformationIterator::New();
  iter->SetInformation(info);
  for (iter->GoToFirstItem(); !iter->IsDoneWithTraversal(); iter->GoToNextItem())
    {
    vtkInformationKey* key = iter->GetCurrentKey();
    std::string keystr = vtkInformationKeyMap::GetFullName(key);
    if (key->IsA("vtkInformationIntegerKey"))
      {
      SerializeScalarKey<vtkInformationIntegerKey, int>(info,
        static_cast<vtkInformationIntegerKey*>(key),
        keystr.c_str(), elem);
      }
    else if (key->IsA("vtkInformationDoubleKey"))
      {
      SerializeScalarKey<vtkInformationDoubleKey, double>(info,
        static_cast<vtkInformationDoubleKey*>(key),
        keystr.c_str(), elem);
      }
    else if (key->IsA("vtkInformationIdTypeKey"))
      {
      SerializeScalarKey<vtkInformationIdTypeKey, vtkIdType>(info,
        static_cast<vtkInformationIdTypeKey*>(key),
        keystr.c_str(), elem);
      }
    else if (key->IsA("vtkInformationStringKey"))
      {
      SerializeScalarKey<vtkInformationStringKey, const char*>(info,
        static_cast<vtkInformationStringKey*>(key),
        keystr.c_str(), elem);
      }
    else if (key->IsA("vtkInformationDoubleVectorKey"))
      {
      SerializeVectorKey<vtkInformationDoubleVectorKey, double>(info,
        static_cast<vtkInformationDoubleVectorKey*>(key),
        keystr.c_str(), elem);
      }
    else if (key->IsA("vtkInformationIntegerVectorKey"))
      {
      SerializeVectorKey<vtkInformationIntegerVectorKey, int>(info,
        static_cast<vtkInformationIntegerVectorKey*>(key),
        keystr.c_str(), elem);
      }
    else if (key->IsA("vtkInformationStringVectorKey"))
      {
      SerializeStringVectorKey(info,
        static_cast<vtkInformationStringVectorKey*>(key),
        keystr.c_str(), elem);
      }
    else if (key->IsA("vtkInformationKeyVectorKey"))
      {
      SerializeKeyVectorKey(info,
        static_cast<vtkInformationKeyVectorKey*>(key),
        keystr.c_str(), elem);
      }
    else if (key->IsA("vtkInformationObjectBaseKey"))
      {
      vtkObject *ptr =vtkObject::SafeDownCast(
        info->Get(static_cast<vtkInformationObjectBaseKey*>(key)));

      // Returns 0 if ptr is null or isn't serializable
      unsigned int serializedId = this->Serialize(ptr);

      vtkSmartPointer<vtkXMLElement> keyElem =
        vtkSmartPointer<vtkXMLElement>::New();
      keyElem->SetName(keystr.c_str());
      elem->AddNestedElement(keyElem);
      keyElem->AddAttribute("to_id", serializedId);
      }
    else if (key->IsA("vtkInformationObjectBaseVectorKey"))
      {
      vtkSmartPointer<vtkXMLElement> keyElem =
        vtkSmartPointer<vtkXMLElement>::New();
      keyElem->SetName(keystr.c_str());
      elem->AddNestedElement(keyElem);

      std::vector<unsigned int> ids;
      vtkInformationObjectBaseVectorKey* vecKey =
        static_cast<vtkInformationObjectBaseVectorKey*>(key);
      int size = vecKey->Size(info);
      for (int i = 0; i < size; ++i)
        {
        vtkObject *ptr = vtkObject::SafeDownCast(vecKey->Get(info, i));
        // Returns 0 if ptr is null or isn't serializable
        unsigned int serializedId = this->Serialize(ptr);
        ids.push_back(serializedId);
        }
      unsigned int length = static_cast<unsigned int>(ids.size());
      unsigned long* idVec = new unsigned long[length];
      std::copy(ids.begin(), ids.end(), idVec);
      keyElem->AddAttribute("ids", idVec, length);
      keyElem->AddAttribute("length", length);
      delete[] idVec;
      }
    }
  iter->Delete();
}

//----------------------------------------------------------------------------
void vtkXMLArchiveWriter::Serialize(const char* name,
                                       std::vector<vtkSmartPointer<vtkObject> >& objs,
                                       bool weakPtr/*=false*/)
{
  vtkSmartPointer<vtkXMLElement> elem = BaseSerialize(name, this->Internal);
  if (!elem.GetPointer())
    {
    return;
    }
  elem->AddAttribute("type", "vtkObjectVector");

  this->Internal->Push(elem);
  std::vector<vtkSmartPointer<vtkObject> >::iterator iter =
    objs.begin();
  for(; iter != objs.end(); iter++)
    {
    vtkObject* obj = iter->GetPointer();
    this->Serialize("Item", obj, weakPtr);
    }
  this->Internal->Pop();
}

//----------------------------------------------------------------------------
void vtkXMLArchiveWriter::Serialize(const char* name,
                                       std::map<int, std::vector<vtkSmartPointer<vtkObject> > >& map)
{
  vtkSmartPointer<vtkXMLElement> elem = BaseSerialize(name, this->Internal);
  if (!elem.GetPointer())
    {
    return;
    }
  elem->AddAttribute("type", "vtkObjectVectorMap");

  this->Internal->Push(elem);
  std::map<int, std::vector<vtkSmartPointer<vtkObject> > >::iterator
    iter = map.begin();
  for(; iter != map.end(); iter++)
    {
    std::vector<vtkSmartPointer<vtkObject> >& objs = iter->second;
    vtksys_ios::ostringstream str;
    str << "Key_" << iter->first;
    this->Serialize(str.str().c_str(), objs);
    }
  this->Internal->Pop();

}

//----------------------------------------------------------------------------
void vtkXMLArchiveWriter::CreateDOM(const char* rootName,
                                       std::vector<vtkSmartPointer<vtkObject> >& objs)
{
  // Initialize
  this->RootElement->SetName(rootName);
  this->RootElement->AddAttribute("version", this->GetArchiveVersion());

  vtkSmartPointer<vtkXMLElement> elem =
    vtkSmartPointer<vtkXMLElement>::New();
  elem->SetName("RootObjects");
  this->RootElement->AddNestedElement(elem);

  this->Internal->Push(elem);
  std::vector<vtkSmartPointer<vtkObject> >::iterator iter =
    objs.begin();
  for( ; iter != objs.end(); iter++)
    {
    vtkObject* obj = iter->GetPointer();
    this->Serialize("Item", obj);
    }
  this->Internal->Pop();
  //return this->RootElement;
}

//----------------------------------------------------------------------------
void vtkXMLArchiveWriter::Serialize(vtkXMLElement* elem,
                                       const char* rootName,
                                       vtkObject *obj)
{
  std::vector<vtkSmartPointer<vtkObject> > objs;
  objs.push_back(obj);
  this->Serialize(elem, rootName, objs);
}

//----------------------------------------------------------------------------
void vtkXMLArchiveWriter::Serialize(vtkXMLElement* elem, const char* rootName,
                                       std::vector<vtkSmartPointer<vtkObject> >& objs)
{
  if (!elem)
    {
    vtkErrorMacro("vtkXMLElement must be instantiated before calling Serialize!");
    return;
    }

  delete this->Internal;
  this->Internal = new vtkXMLArchiveWriterInternals;

  this->SetRootElement(elem);
  this->CreateDOM(rootName, objs);
  this->SetRootElement(0);

  delete this->Internal;
  this->Internal = 0;
}

//----------------------------------------------------------------------------
void vtkXMLArchiveWriter::Serialize(ostream& ostr, const char* rootName,
                                       std::vector<vtkSmartPointer<vtkObject> >& objs)
{
  delete this->Internal;
  this->Internal = new vtkXMLArchiveWriterInternals;

  vtkXMLElement *rootElement = vtkXMLElement::New();
  this->SetRootElement(rootElement);
  rootElement->Delete();
  this->CreateDOM(rootName, objs);
  this->RootElement->PrintXML(ostr, vtkIndent());
  this->SetRootElement(0);

  delete this->Internal;
  this->Internal = 0;
}

//----------------------------------------------------------------------------
void vtkXMLArchiveWriter::Serialize(const char* name, int& val)
{
  vtkSmartPointer<vtkXMLElement> elem = BaseSerialize(name, this->Internal);
  if (!elem.GetPointer())
    {
    return;
    }

  elem->AddAttribute("value", val);
}

//----------------------------------------------------------------------------
void vtkXMLArchiveWriter::Serialize(const char* name,
                                       int*& val,
                                       unsigned int& length)
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

//----------------------------------------------------------------------------
void vtkXMLArchiveWriter::Serialize(const char* name, unsigned long& val)
{
  vtkSmartPointer<vtkXMLElement> elem = BaseSerialize(name, this->Internal);
  if (!elem.GetPointer())
    {
    return;
    }

  elem->AddAttribute("value", val);
}

//----------------------------------------------------------------------------
void vtkXMLArchiveWriter::Serialize(const char* name,
                                       unsigned long*& val,
                                       unsigned int& length)
{
  vtkSmartPointer<vtkXMLElement> elem=BaseSerialize(name, this->Internal);
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

//----------------------------------------------------------------------------
#if defined(VTK_USE_64BIT_IDS)
void vtkXMLArchiveWriter::Serialize(const char* name, vtkIdType& val)
{
  vtkSmartPointer<vtkXMLElement> elem=BaseSerialize(name, this->Internal);
  if (!elem.GetPointer())
    {
    return;
    }

  elem->AddAttribute("value", val);
}

//----------------------------------------------------------------------------
void vtkXMLArchiveWriter::Serialize(const char* name,
                                       vtkIdType*& val,
                                       unsigned int& length)
{
  vtkSmartPointer<vtkXMLElement> elem=BaseSerialize(name, this->Internal);
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

//----------------------------------------------------------------------------
void vtkXMLArchiveWriter::Serialize(const char* name, double& val)
{
  vtkSmartPointer<vtkXMLElement> elem = BaseSerialize(name, this->Internal);
  if (!elem.GetPointer())
    {
    return;
    }

  elem->AddAttribute("value", val);
}

//----------------------------------------------------------------------------
void vtkXMLArchiveWriter::Serialize(const char* name,
                                       double*& val,
                                       unsigned int& length)
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

//----------------------------------------------------------------------------
void vtkXMLArchiveWriter::Serialize(const char* name, char*& str)
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

//----------------------------------------------------------------------------
void vtkXMLArchiveWriter::Serialize(const char* name, std::string& str)
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
