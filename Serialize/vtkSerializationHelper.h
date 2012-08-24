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
// .NAME vtkSerializationHelper - Superclass for serialization helpers
// .SECTION Description
// Abstract class for serialization of vtkObjects with the
// vtkXMLArchiveWriter and vtkXMLArchiveReader.  Subclasses must
// implement RegisterWithHelperMap (which registers each class type supported
// by the helper with the vtkSerializationHelperMap), GetSerializationType
// (returns the "type" to set as the attribute for the element), and Serialize
// (which does the actual serialization) and UnRegisterWithHelperMap().
// .SECTION See Also
// vtkSerializationHelperMap

#ifndef __vtkSerializationHelper_h
#define __vtkSerializationHelper_h

#include "vtkDiscreteModelModule.h" // For export macro
#include "vtkObject.h"

class vtkSerializer;

class VTKDISCRETEMODEL_EXPORT vtkSerializationHelper : public vtkObject
{
public:
  vtkTypeMacro(vtkSerializationHelper,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // This member registers ALL the classes supported by this helper with the
  // vtkSerializationHelperMap, which manages all the helpers.  Should be
  // called after construction unless you only want to enable support for
  // a subset (in which case it must be done "manually" by calling
  // vtkSerializationHelperMap::RegisterHelperForClass()
  virtual void RegisterWithHelperMap() = 0;

  // Description:
  // Unregister the helper (remove each class type/helper pair) with the
  // vtkSerializationHelperMap
  virtual void UnRegisterWithHelperMap() = 0;

  // Description:
  // Get the value for the "type" attribute of the specfied object type
  virtual const char *GetSerializationType(vtkObject *object) = 0;

  // Description:
  // Serialize the input object.  Returns 1 if successful.
  virtual int Serialize(vtkObject *object, vtkSerializer *serializer) = 0;

protected:
  vtkSerializationHelper() {}
  ~vtkSerializationHelper() {}

private:
  vtkSerializationHelper(const vtkSerializationHelper&);  // Not implemented.
  void operator=(const vtkSerializationHelper&);  // Not implemented.
};

#endif
