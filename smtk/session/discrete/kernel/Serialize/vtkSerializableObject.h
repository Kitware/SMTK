//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkSerializableObject - Superclass for serializable objects
// .SECTION Description
// vtkSerializableObject works with vtkSerializer and its subclasses
// to provide a serialization framework. The user asks a vtkSerializer to
// write or read an archive consisting of a collection of vtkSerializableObjects.
// The serializer then reads/writes the archive in collaboration with the
// serializable objects.
// When reading an archive, the serializer creates the appropriate objects
// (usually using the instantiator) and then calls Serialize() on them.
// It is then the responsability of these objects to restore their state
// by calling functions provided by the serializer. For example, this may
// look like:
// \code
// void vtkObjectTreeNodeBase::Serialize(vtkSerializer* ser)
// {
// ser->Serialize("Attributes", this->Attributes);
// ser->Serialize("Properties", this->Properties);
// vtkSerializableObject *parent = this->Parent;
// ser->Serialize("Parent", parent, true); // true indicates it is a weak ptr
// if (ser->IsWriting())
//   {
//   std::vector< vtkSmartPointer<vtkSerializableObject> > myVector =
//     vtkSerializer::ToBase<std::vector<vtkSmartPointer<vtkObjectTreeNodeBase> > >(
//        *this->Children );
//   ser->Serialize("Children", myVector);
//   }
// else
//   {
//   this->Parent = vtkObjectTreeNodeBase::SafeDownCast(parent);
//   std::vector< vtkSmartPointer<vtkSerializableObject> > myVector;
//   ser->Serialize("Children", myVector);
//   vtkSerializer::FromBase<vtkObjectTreeNodeBase>(myVector, *this->Children);
//   }
// \endcode
// When writing the archive, the serializer calls Serialize() on the "root"
// objects (the ones passed by the user) which then use the serializer methods
// to write themselves in the archive.
//
// Note that there is only one Serialize() method that does reading and writing.
// This works because the methods in vtkSerializer take references to data
// member and can read or write them as appropriate. In cases where the class
// needs to do something different during reading or writing, you can separate
// the implementation using if (ser->IsWriting()) {..} else {..} as shown
// in the example above.
//
// You can also implement more sophisticated serialization code by using
// temporaries during reading and writing and copying them to the actual data
// members yourself. Something like:
// \code
// void vtkSomeSerializableObject::Serialize(vtkSerializer* ser)
// {
//   int foo;
//   if (ser->IsWriting)
//     {
//     // ...
//     }
//   else
//     {
//     int foo;
//     ser->Serialize(foo);
//     // Based on the value of foo, do something to data members
//     }
// \endcode
//
// The serializer also supports versioning. You can get the version of the
// archive being written or read using vtkSerializer::GetArchiveVersion()

#ifndef __smtkdiscrete_vtkSerializableObject_h
#define __smtkdiscrete_vtkSerializableObject_h

#include "smtk/session/discrete/kernel/vtkSMTKDiscreteModelModule.h" // For export macro
#include "vtkObject.h"

class vtkSerializer;

class VTKSMTKDISCRETEMODEL_EXPORT vtkSerializableObject : public vtkObject
{
public:
  static vtkSerializableObject* New();
  vtkTypeMacro(vtkSerializableObject, vtkObject);

  // Description:
  // Reads the state of an instance from an archive OR
  // writes the state of an instance to an archive. See
  // the documentation for this class for details.
  virtual void Serialize(vtkSerializer*) {}

protected:
  vtkSerializableObject();
  ~vtkSerializableObject() override;

private:
  vtkSerializableObject(const vtkSerializableObject&); // Not implemented.
  void operator=(const vtkSerializableObject&);        // Not implemented.
};

#endif
