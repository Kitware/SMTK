//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkSerializationHelperMap - Map from class name to helper object.
// .SECTION Description
// This class is used to map from class name to a helper class that has been
// registered to serialize the class.
//
// Note, helper classes must be manually instantiated, at which time
// the class types supported by the helper should be registered with the
// vtkSerializationHelperMap (by calling RegisterWithHelperMap on the
// helper).  The "known" helper classes (those for the VTK/VTKEdge kits)
// should be added to InstantiateDefaultHelpers(), which is called by both
// the vtkXMLArchiveReader and vtkXMLArchiveWriter during their
// construction.  This class then manages destruction of the helper since the
// map holds a vtkSmartPointer to the helper for each supported class type.
//
// .SECTION See Also
// vtkSerializationHelper

#ifndef __smtkdiscrete_vtkSerializationHelperMap_h
#define __smtkdiscrete_vtkSerializationHelperMap_h

#include "smtk/bridge/discrete/kernel/vtkSMTKDiscreteModelModule.h" // For export macro
#include "vtkObject.h"

class vtkSerializationHelper;
class vtkSerializer;
class vtkXMLElement;

class VTKSMTKDISCRETEMODEL_EXPORT vtkSerializationHelperMap : public vtkObject
{
public:
  static vtkSerializationHelperMap* New();
  vtkTypeMacro(vtkSerializationHelperMap, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Instantiate all "known" (supporting core VTK and VTKEdge) helpers
  // and then register them with the helper map.
  static void InstantiateDefaultHelpers();

  // Description:
  // Register (add) a class type / helper with the map.
  static void RegisterHelperForClass(const char* classType, vtkSerializationHelper* helper);

  // Description:
  // UnRegister (remove) a class type / helper in the map, if the registered
  // helper matches.
  static void UnRegisterHelperForClass(const char* classType, vtkSerializationHelper* helper);

  // Description:
  // Serialize the input object using a registered helper.  If successful (map
  // entry exists), returns 1 (0 if not).
  static int Serialize(vtkObject* obj, vtkSerializer* serializer);

  // Description:
  // Get the value for the "type" attribute (generally the ClassName but may be
  // name of Superclass) of the specfied object type from a registerd helper
  // for the object Clas
  static const char* GetSerializationType(vtkObject* object);

  // Description:
  // Returns true if the object class type is registered with the map, and thus
  // can be serialized.
  static bool IsSerializable(vtkObject* obj);

  // Description:
  // Return the serialization helper registered for the indicated class (if one
  // has been registerd); otherwise return NULL
  static vtkSerializationHelper* GetHelper(const char* classType);

  // Description:
  // Removes all helpers from the map.
  static void RemoveAllHelpers();

protected:
  vtkSerializationHelperMap();
  ~vtkSerializationHelperMap();

private:
  vtkSerializationHelperMap(const vtkSerializationHelperMap&); // Not implemented.
  void operator=(const vtkSerializationHelperMap&);            // Not implemented.

  static bool DefaultHelpersInstantiated;
};

#endif
