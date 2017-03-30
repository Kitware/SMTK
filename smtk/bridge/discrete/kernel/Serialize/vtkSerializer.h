//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkSerializer - Abstract superclass of input and output archivers
// .SECTION Description
// vtkSerializer and its sub-classes are used to serialize and de-serialize
// a collection of objects to/from an io stream. The serializers can work
// with vtkSerializableObject and its subclasses, or vtkObjects in general
// that have a subclass of vtkSerializationHelper registered with
// vtkSerializationHelperMap. It can walk a object graph and
// serialize/deserialize all objects contained in it. Circular references are
// supported.
// In order to (most easily) add serialization support to your class, subclass
// from vtkSerializableObject and re-implement its Serialize() method using
// methods available in the serializer.  Otherwise create a subclass
// vtkSerializationHelper and register it with the serialization manager
// See vtkSerializableObject for details.
// .SECTION See Also
// vtkSerializableObject vtkSerializationHelper vtkSerializationHelperMap

#ifndef __smtkdiscrete_vtkSerializer_h
#define __smtkdiscrete_vtkSerializer_h

#include "smtk/bridge/discrete/kernel/vtkSMTKDiscreteModelModule.h" // For export macro
#include "vtkObject.h"

#include "vtkSmartPointer.h" // For collections
#include <map>               // For maps
#include <vector>            // For vectors

class vtkInformation;

class VTKSMTKDISCRETEMODEL_EXPORT vtkSerializer : public vtkObject
{
public:
  vtkTypeMacro(vtkSerializer, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // This method returns true if the serializer is an output
  // serializer (writer)
  virtual bool IsWriting() = 0;

  // Description:
  // Serializes a single integer.
  virtual void Serialize(const char* name, int& val) = 0;

  // Description:
  // Serializes an array.
  virtual void Serialize(const char* name, int*& val, unsigned int& length) = 0;

  // Description:
  // Serializes a single unsigned long.
  virtual void Serialize(const char* name, unsigned long& val) = 0;

  // Description:
  // Serializes an array.
  virtual void Serialize(const char* name, unsigned long*& val, unsigned int& length) = 0;

// Description:
// Serializes a single vtkIdType.
#if defined(VTK_USE_64BIT_IDS)
  virtual void Serialize(const char* name, vtkIdType& val) = 0;
#endif

// Description:
// Serializes an array.
#if defined(VTK_USE_64BIT_IDS)
  virtual void Serialize(const char* name, vtkIdType*& val, unsigned int& length) = 0;
#endif

  // Description:
  // Serializes a single double.
  virtual void Serialize(const char* name, double& val) = 0;

  // Description:
  // Serializes an array.
  virtual void Serialize(const char* name, double*& val, unsigned int& length) = 0;

  // Description:
  // Serializes a string.
  virtual void Serialize(const char* name, char*& str) = 0;

  // Description:
  // Serializes a string.
  virtual void Serialize(const char* name, std::string& str) = 0;

  // Description:
  // Serializes a vtkObject.
  virtual void Serialize(const char* name, vtkObject*& obj, bool weakPtr = false) = 0;

  // Description:
  // Serializes a vtkInformationObject. Note that only keys registered
  // with the vtkInformationKeyMap are restored.
  virtual void Serialize(const char* name, vtkInformation* info) = 0;

  // Description:
  // Serializes a vector of vtkObjects.
  virtual void Serialize(
    const char* name, std::vector<vtkSmartPointer<vtkObject> >& objs, bool weakPtr = false) = 0;

  // Description:
  // Serializes a map from int to vector of vtkObjects.
  virtual void Serialize(
    const char* name, std::map<int, std::vector<vtkSmartPointer<vtkObject> > >& objs) = 0;

  // Description:
  // Set/Get the archive version. Make sure to set the version before
  // writing to an archive. When reading an archive, the version is read
  // from the input stream.
  vtkSetMacro(ArchiveVersion, unsigned int);
  vtkGetMacro(ArchiveVersion, unsigned int);

  // Description:
  // Helper function to make is easier to write containers of sub-classes
  // of vtkObject. For example:
  // \code
  // std::Container<vtkSmartPointer<vtkSubclass> > sub;
  // std::vector<vtkSmartPointer<vtkObject> > vec =
  //    vtkSerializer::ToBase<Container>(sub);
  //  ser->Serialize("Vector", vec);
  // \endcode
  // The container must support forward iteration, begin(), end(), and
  // push_back.
  template <typename Container>
  static std::vector<vtkSmartPointer<vtkObject> > ToBase(Container& from)
  {
    std::vector<vtkSmartPointer<vtkObject> > retVal;
    typename Container::iterator iter = from.begin();
    for (; iter != from.end(); iter++)
    {
      retVal.push_back(*iter);
    }
    return retVal;
  }

  // Description:
  // Helper function to make it easier to read into containers of sub-classes
  // of vtkObject. For example:
  // \code
  // std::Container<vtkSmartPointer<vtkSubclass> > sub;
  // std::vector<vtkSmartPointer<vtkObject> >  vec;
  // ser->Serialize("Vector", vec);
  //  vtkSerializer::FromBase<vtkConceptualModelEntityItem,Container>(vec, sub);
  // \endcode
  // The container must support forward iteration, begin(), end(), and
  // push_back.
  template <typename T, typename Container>
  static void FromBase(std::vector<vtkSmartPointer<vtkObject> >& from, Container& to)
  {
    std::vector<vtkSmartPointer<vtkObject> >::iterator iter = from.begin();
    for (; iter != from.end(); iter++)
    {
      //      to.push_back(static_cast<T*>(iter->GetPointer()));
      to.insert(to.end(), static_cast<T*>(iter->GetPointer()));
    }
  }

  // Description:
  // Helper function to make it easier to write maps of containers of sub-classes
  // of vtkObject. For example:
  // \code
  // std::map<int, std::vector<vtkSmartPointer<vtkObject> > > map =
  //  vtkSerializer::ToBase<Container>(this->Internal->Associations);
  //  ser->Serialize("Associations", map);
  // \endcode
  // The container must support forward iteration, begin(), end(), and
  // push_back.
  template <typename Container>
  static std::map<int, std::vector<vtkSmartPointer<vtkObject> > > ToBase(
    std::map<int, Container>& from)
  {
    std::map<int, std::vector<vtkSmartPointer<vtkObject> > > retVal;
    typename std::map<int, Container>::iterator iter = from.begin();
    for (; iter != from.end(); iter++)
    {
      retVal[iter->first] = (ToBase<Container>(iter->second));
    }
    return retVal;
  }

  // Description:
  // Helper function to make it easier to read maps of containers of sub-classes
  // of vtkObject. For example:
  // \code
  // std::map<int, std::vector<vtkSmartPointer<vtkObject> > > map;
  // ser->Serialize("Associations", map);
  // vtkSerializer::FromBase<vtkConceptualModelEntityItem,Container>(map, this->Internal->Associations);
  // \endcode
  // The container must support forward iteration, begin(), end(), and
  // push_back.
  template <typename T, typename Container>
  static void FromBase(
    std::map<int, std::vector<vtkSmartPointer<vtkObject> > >& from, std::map<int, Container>& to)
  {
    std::map<int, std::vector<vtkSmartPointer<vtkObject> > >::iterator iter = from.begin();
    for (; iter != from.end(); iter++)
    {
      std::vector<vtkSmartPointer<vtkObject> >& f = iter->second;
      Container& t = to[iter->first];
      FromBase<T, Container>(f, t);
    }
  }

protected:
  vtkSerializer();
  ~vtkSerializer() {}

private:
  vtkSerializer(const vtkSerializer&);  // Not implemented.
  void operator=(const vtkSerializer&); // Not implemented.

  unsigned int ArchiveVersion;
};

#endif
