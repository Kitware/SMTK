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
// .NAME vtkCommonSerializationHelper - Concrete serialization helper for
// Common package vtkObjects.
// .SECTION Description
// Concrete class for serialization of vtkObjects in the Common package using
// vtkXMLArchiveWriter and vtkXMLArchiveReader.  Only serialization of
// vtkTransform, vtkIdTypeArray, vtkIntArray, and vtkDoubleArray is currently
// implemented.
// .SECTION See Also
// vtkSerializationHelperMap vtkSerializationHelper

#ifndef __vtkCommonSerializationHelper_h
#define __vtkCommonSerializationHelper_h

#include "vtkDiscreteModelModule.h" // For export macro
#include "vtkSerializationHelper.h"
#include "cmbSystemConfig.h"

class vtkDataArray;
class vtkSerializer;
class vtkTransform;

class VTKDISCRETEMODEL_EXPORT vtkCommonSerializationHelper : public vtkSerializationHelper
{
public:
  static vtkCommonSerializationHelper *New();
  vtkTypeMacro(vtkCommonSerializationHelper, vtkSerializationHelper);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // This member registers ALL the classes supported by this helper with the
  // vtkSerializationHelperMap, which manages all the helpers.  Should be
  // called after construction unless you only want to enable support for
  // a subset (in which case it must be done "manually" by calling
  // vtkSerializationHelperMap::RegisterHelperForClass()
  virtual void RegisterWithHelperMap();

  // Description:
  // Unregister this helper (remove each class type/helper pair) with the
  // vtkSerializationHelperMap
  virtual void UnRegisterWithHelperMap();

  // Description:
  // Get the value for the "type" attribute of the specfied object type
  virtual const char *GetSerializationType(vtkObject *object);

  // Description:
  // Serialize the input object.  Returns 1 if successful.
  virtual int Serialize(vtkObject *object, vtkSerializer *serializer);

  //BTX
protected:
  vtkCommonSerializationHelper();
  ~vtkCommonSerializationHelper() {}

  // Description:
  // Serialize a vtkTransform
  void SerializeTransform(vtkTransform *transform, vtkSerializer *serializer);

  // Description:
  // Serialize vtkIdTypeArray, vtkIntArray, or vtkDoubleArray
  void SerializeDataArray(vtkDataArray *dataArray, vtkSerializer *serializer);

private:
  vtkCommonSerializationHelper(const vtkCommonSerializationHelper&);  // Not implemented.
  void operator=(const vtkCommonSerializationHelper&);  // Not implemented.
  //ETX
};

#endif
