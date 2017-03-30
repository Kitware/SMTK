//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkXMLArchiveReader.h"

#include "vtkInformation.h"
#include "vtkInformationDoubleKey.h"
#include "vtkInformationDoubleVectorKey.h"
#include "vtkInformationIdTypeKey.h"
#include "vtkInformationIntegerKey.h"
#include "vtkInformationIntegerVectorKey.h"
#include "vtkInformationKeyMap.h"
#include "vtkInformationKeyVectorKey.h"
#include "vtkInformationObjectBaseKey.h"
#include "vtkInformationObjectBaseVectorKey.h"
#include "vtkInformationStringKey.h"
#include "vtkInformationStringVectorKey.h"
#include "vtkInstantiator.h"
#include "vtkModelXMLParser.h"
#include "vtkObjectFactory.h"
#include "vtkSerializableObject.h"
#include "vtkSerializationHelperMap.h"
#include "vtkSmartPointer.h"
#include "vtkXMLElement.h"

#include <list>
#include <map>
#include <sstream>
#include <vtksys/SystemTools.hxx>

vtkStandardNewMacro(vtkXMLArchiveReader);

vtkCxxSetObjectMacro(vtkXMLArchiveReader, RootElement, vtkXMLElement);

struct vtkXMLArchiveReaderInternals
{
  std::map<int, vtkXMLElement*> IdToElement;

  vtkXMLElement* FindElement(int id)
  {
    std::map<int, vtkXMLElement*>::iterator iter = this->IdToElement.find(id);
    if (iter != this->IdToElement.end())
    {
      return iter->second;
    }
    return 0;
  }

  std::map<int, vtkObject*> IdToObject;

  vtkObject* FindObject(int id)
  {
    std::map<int, vtkObject*>::iterator iter = this->IdToObject.find(id);
    if (iter != this->IdToObject.end())
    {
      return iter->second;
    }
    return 0;
  }

  vtkXMLElement* Top() { return this->Stack.front(); }

  void Push(vtkXMLElement* elem) { this->Stack.push_front(elem); }

  void Pop() { this->Stack.pop_front(); }

  std::list<vtkXMLElement*> Stack;

  // Stack to hold all the serializable objects; to support a weakPtr
  // to an object that is read before the object being referred to is created.
  // The ObjectStack holds an extra reference to all objects to prevent leaks
  // while supoorting this situtation (see comment in ReadObject).
  std::list<vtkSmartPointer<vtkObject> > ObjectStack;
};

vtkXMLArchiveReader::vtkXMLArchiveReader()
{
  this->Internal = new vtkXMLArchiveReaderInternals;
  vtkSerializationHelperMap::InstantiateDefaultHelpers();
  this->RootElement = 0;
}

vtkXMLArchiveReader::~vtkXMLArchiveReader()
{
  this->SetRootElement(0);
  delete this->Internal;
}

void vtkXMLArchiveReader::Serialize(
  istream& str, const char*, std::vector<vtkSmartPointer<vtkObject> >& objs)
{
  delete this->Internal;
  this->Internal = new vtkXMLArchiveReaderInternals;
  objs.clear();
  if (this->ParseStream(str))
  {
    this->Serialize(objs);
  }
}

void vtkXMLArchiveReader::Serialize(
  vtkXMLElement* rootElement, const char*, std::vector<vtkSmartPointer<vtkObject> >& objs)
{
  delete this->Internal;
  this->Internal = new vtkXMLArchiveReaderInternals;
  objs.clear();

  this->SetRootElement(rootElement);
  this->Serialize(objs);
}

void vtkXMLArchiveReader::Serialize(std::vector<vtkSmartPointer<vtkObject> >& objs)
{
  unsigned int nnested = this->RootElement->GetNumberOfNestedElements();
  for (unsigned int i = 0; i < nnested; i++)
  {
    vtkXMLElement* elem = this->RootElement->GetNestedElement(i);
    int id;
    if (elem->GetScalarAttribute("id", &id))
    {
      this->Internal->IdToElement[id] = elem;
    }
  }

  unsigned int version;
  if (this->RootElement->GetScalarAttribute("version", &version))
  {
    this->SetArchiveVersion(version);
  }

  this->Internal->Push(this->RootElement);
  this->Internal->ObjectStack.clear(); // just to be sure
  this->Serialize("RootObjects", objs);
  this->Internal->Pop();

  delete this->Internal;
  this->Internal = 0;
}

vtkObject* vtkXMLArchiveReader::ReadObject(int id, bool weakPtr)
{
  // There will never be an object with id 0
  if (id == 0)
  {
    return 0;
  }

  vtkObject* obj = this->Internal->FindObject(id);
  if (obj)
  {
    if (!weakPtr)
    {
      obj->Register(0);
    }
    return obj;
  }

  vtkXMLElement* elem = this->Internal->FindElement(id);
  if (!elem)
  {
    vtkErrorMacro("Could not find of id " << id) return 0;
  }
  const char* className = elem->GetAttribute("type");
  obj = vtkObject::SafeDownCast(vtkInstantiator::CreateInstance(className));
  if (!obj)
  {
    vtkErrorMacro("Could not create object of type " << className) return 0;
  }
  this->Internal->ObjectStack.push_back(obj);
  if (weakPtr)
  {
    // pointer to this object was weak, but the object hadn't been created
    // yet;  The ObjectStack will hold the only reference until non-weak
    // references are made to the object (and registered at the top of this
    // fn).  The extra reference held by the ObjectStack gets removed when
    // we are done reading (delete the this->Internal structure).
    obj->UnRegister(0);
  }
  this->Internal->IdToObject[id] = obj;
  this->Internal->Push(elem);
  vtkSerializableObject* serializableObject = vtkSerializableObject::SafeDownCast(obj);
  if (serializableObject)
  {
    serializableObject->Serialize(this);
  }
  else if (vtkInformation::SafeDownCast(obj))
  {
    this->Serialize(elem, vtkInformation::SafeDownCast(obj));
  }
  else
  {
    vtkSerializationHelperMap::Serialize(obj, this);
  }

  this->Internal->Pop();
  return obj;
}

int vtkXMLArchiveReader::ParseStream(istream& str)
{
  vtkSmartPointer<vtkModelXMLParser> parser = vtkSmartPointer<vtkModelXMLParser>::New();
  parser->SetStream(&str);
  int result = parser->Parse();
  this->SetRootElement(parser->GetRootElement());
  return result;
}

void vtkXMLArchiveReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

namespace
{
vtkXMLElement* BaseSerialize(vtkXMLArchiveReaderInternals* internal, const char* name)
{
  vtkXMLElement* root = internal->Top();
  if (!root)
  {
    vtkGenericWarningMacro("Serialize cannot be called outside serialization");
    return 0;
  }

  return root->FindNestedElementByName(name);
}
}

void vtkXMLArchiveReader::Serialize(const char* name,
  std::vector<vtkSmartPointer<vtkObject> >& objs, bool vtkNotUsed(weakPtr) /*=false*/)
{
  objs.clear();
  vtkXMLElement* elem = BaseSerialize(this->Internal, name);
  if (!elem)
  {
    return;
  }

  unsigned int nnested = elem->GetNumberOfNestedElements();
  for (unsigned int i = 0; i < nnested; i++)
  {
    vtkXMLElement* objElem = elem->GetNestedElement(i);
    int id;
    if (objElem->GetScalarAttribute("to_id", &id))
    {
      vtkObject* obj = this->ReadObject(id, false);
      if (obj)
      {
        objs.push_back(obj);
        // ReadObject incremented the ReferenceCount (weakPtr = false), or
        // created the object (ReferenceCount = 1); we then stuff it in the
        // vector , which is where we were trying to get it but has the
        // side effect of further incrementing the ReferenceCount.  To prevent
        // a leak we need to decerment the ReferenceCount.
        obj->UnRegister(0);
      }
    }
  }
}

void vtkXMLArchiveReader::Serialize(
  const char* name, std::map<int, std::vector<vtkSmartPointer<vtkObject> > >& map)
{
  map.clear();
  vtkXMLElement* elem = BaseSerialize(this->Internal, name);
  if (!elem)
  {
    return;
  }

  this->Internal->Push(elem);
  unsigned int nnested = elem->GetNumberOfNestedElements();
  for (unsigned int i = 0; i < nnested; i++)
  {
    vtkXMLElement* vecElem = elem->GetNestedElement(i);
    // Take out Key_ and convert to int
    int key = atoi(vecElem->GetName() + 4);
    this->Serialize(vecElem->GetName(), map[key]);
  }
  this->Internal->Pop();
}

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

void SerializeStringVectorKey(
  vtkInformation* info, vtkInformationStringVectorKey* key, vtkXMLElement* elem)
{
  int length = 0;
  if (!elem->GetScalarAttribute("length", &length) || length <= 0)
  {
    return;
  }

  const char* values = elem->GetAttribute("values");
  if (!values)
  {
    return;
  }

  int* lengths = new int[length];
  if (!elem->GetVectorAttribute("lengths", length, lengths))
  {
    delete[] lengths;
    return;
  }

  std::string valuesStr(values);
  size_t currentPos = 0;
  for (int i = 0; i < length; ++i)
  {
    size_t strLength = static_cast<size_t>(lengths[i]);
    std::string tmpStr = valuesStr.substr(currentPos, strLength);
    currentPos += strLength + 1; // skip over the semi-colon separator

    key->Set(info, tmpStr.c_str(), i);
  }
  delete[] lengths;
}

void SerializeKeyVectorKey(
  vtkInformation* info, vtkInformationKeyVectorKey* key, vtkXMLElement* elem)
{
  int length;
  if (!elem->GetScalarAttribute("length", &length))
  {
    return;
  }
  if (length <= 0)
  {
    return;
  }

  const char* values = elem->GetAttribute("values");
  if (!values)
  {
    return;
  }

  std::istringstream valueStr(values);
  for (int i = 0; i < length; ++i)
  {
    std::string keyName;
    valueStr >> keyName;

    vtkInformationKey* tmpKey = vtkInformationKeyMap::FindKey(keyName.c_str());
    if (tmpKey)
    {
      info->Append(key, tmpKey);
    }
    else
    {
      vtkGenericWarningMacro("Lookup failed for vtkInformationKey: " << keyName.c_str());
    }
  }
}
}

void vtkXMLArchiveReader::Serialize(const char* name, vtkInformation* info)
{
  this->Serialize(BaseSerialize(this->Internal, name), info);
}

void vtkXMLArchiveReader::Serialize(vtkXMLElement* elem, vtkInformation* info)
{
  if (!elem)
  {
    return;
  }

  info->Clear();
  unsigned int nnested = elem->GetNumberOfNestedElements();
  for (unsigned int i = 0; i < nnested; i++)
  {
    vtkXMLElement* keyElem = elem->GetNestedElement(i);
    vtkInformationKey* key = vtkInformationKeyMap::FindKey(keyElem->GetName());
    if (!key)
    {
      vtkErrorMacro("Lookup failed for vtkInformationKey: " << keyElem->GetName());
      continue;
    }

    if (key->IsA("vtkInformationIntegerKey"))
    {
      SerializeScalarKey<vtkInformationIntegerKey, int>(
        info, static_cast<vtkInformationIntegerKey*>(key), keyElem);
    }
    else if (key->IsA("vtkInformationDoubleKey"))
    {
      SerializeScalarKey<vtkInformationDoubleKey, double>(
        info, static_cast<vtkInformationDoubleKey*>(key), keyElem);
    }
    else if (key->IsA("vtkInformationIdTypeKey"))
    {
      SerializeScalarKey<vtkInformationIdTypeKey, vtkIdType>(
        info, static_cast<vtkInformationIdTypeKey*>(key), keyElem);
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
      SerializeVectorKey<vtkInformationDoubleVectorKey, double>(
        info, static_cast<vtkInformationDoubleVectorKey*>(key), keyElem);
    }
    else if (key->IsA("vtkInformationIntegerVectorKey"))
    {
      SerializeVectorKey<vtkInformationIntegerVectorKey, int>(
        info, static_cast<vtkInformationIntegerVectorKey*>(key), keyElem);
    }
    else if (key->IsA("vtkInformationStringVectorKey"))
    {
      SerializeStringVectorKey(info, static_cast<vtkInformationStringVectorKey*>(key), keyElem);
    }
    else if (key->IsA("vtkInformationKeyVectorKey"))
    {
      SerializeKeyVectorKey(info, static_cast<vtkInformationKeyVectorKey*>(key), keyElem);
    }
    else if (key->IsA("vtkInformationObjectBaseKey"))
    {
      int id;
      if (keyElem->GetScalarAttribute("to_id", &id))
      {
        vtkObject* obj = this->ReadObject(id, false);
        info->Set(static_cast<vtkInformationObjectBaseKey*>(key), obj);
        if (obj)
        {
          // ReadObject incremented the ReferenceCount (weakPtr = false), or
          // created the object (ReferenceCount = 1); we then stuff it in the
          // information object, which is where we were trying to get it but
          // has the side effect of further incrementing the ReferenceCount.
          // To prevent a leak we need to decrment the ReferenceCount.
          obj->UnRegister(0);
        }
      }
    }
    else if (key->IsA("vtkInformationObjectBaseVectorKey"))
    {
      vtkInformationObjectBaseVectorKey* vecKey =
        static_cast<vtkInformationObjectBaseVectorKey*>(key);
      unsigned int length;
      if (keyElem->GetScalarAttribute("length", &length))
      {
        unsigned long* ids = new unsigned long[length];
        if (keyElem->GetVectorAttribute("ids", length, ids))
        {
          vecKey->Resize(info, length);
          for (unsigned int k = 0; k < length; ++k)
          {
            vtkObject* obj = this->ReadObject(ids[k], false);
            vecKey->Set(info, obj, k);
            if (obj)
            {
              // ReadObject incremented the ReferenceCount (weakPtr = false), or
              // created the object (ReferenceCount = 1); we then stuff it in the
              // information object, which is where we were trying to get it but
              // has the side effect of further incrementing the ReferenceCount.
              // To prevent a leak we need to decrment the ReferenceCount.
              obj->UnRegister(0);
            }
          }
        }

        delete[] ids;
      }
    }
  }
}

void vtkXMLArchiveReader::Serialize(const char* name, vtkObject*& obj, bool weakPtr /*=false*/)
{
  vtkXMLElement* elem = BaseSerialize(this->Internal, name);
  if (!elem)
  {
    return;
  }

  obj = 0;
  int id;
  if (elem->GetScalarAttribute("to_id", &id))
  {
    obj = this->ReadObject(id, weakPtr);
  }
}

void vtkXMLArchiveReader::Serialize(const char* name, int& val)
{
  vtkXMLElement* elem = BaseSerialize(this->Internal, name);
  if (!elem)
  {
    return;
  }

  elem->GetScalarAttribute("value", &val);
}

void vtkXMLArchiveReader::Serialize(const char* name, int*& val, unsigned int& length)
{
  vtkXMLElement* elem = BaseSerialize(this->Internal, name);
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

void vtkXMLArchiveReader::Serialize(const char* name, unsigned long& val)
{
  vtkXMLElement* elem = BaseSerialize(this->Internal, name);
  if (!elem)
  {
    return;
  }

  elem->GetScalarAttribute("value", &val);
}

void vtkXMLArchiveReader::Serialize(const char* name, unsigned long*& val, unsigned int& length)
{
  vtkXMLElement* elem = BaseSerialize(this->Internal, name);
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

#if defined(VTK_USE_64BIT_IDS)
void vtkXMLArchiveReader::Serialize(const char* name, vtkIdType& val)
{
  vtkXMLElement* elem = BaseSerialize(this->Internal, name);
  if (!elem)
  {
    return;
  }

  elem->GetScalarAttribute("value", &val);
}

void vtkXMLArchiveReader::Serialize(const char* name, vtkIdType*& val, unsigned int& length)
{
  vtkXMLElement* elem = BaseSerialize(this->Internal, name);
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

void vtkXMLArchiveReader::Serialize(const char* name, double& val)
{
  vtkXMLElement* elem = BaseSerialize(this->Internal, name);
  if (!elem)
  {
    return;
  }

  elem->GetScalarAttribute("value", &val);
}

void vtkXMLArchiveReader::Serialize(const char* name, double*& val, unsigned int& length)
{
  vtkXMLElement* elem = BaseSerialize(this->Internal, name);
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

void vtkXMLArchiveReader::Serialize(const char* name, char*& str)
{
  vtkXMLElement* elem = BaseSerialize(this->Internal, name);
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

void vtkXMLArchiveReader::Serialize(const char* name, std::string& str)
{
  vtkXMLElement* elem = BaseSerialize(this->Internal, name);
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
