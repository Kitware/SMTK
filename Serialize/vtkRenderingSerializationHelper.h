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
// .NAME vtkRenderingSerializationHelper - Concrete serialization helper for
// Rendering package.
// .SECTION Description
// Concrete class for serialization of vtkObjects in the Rendering package using
// vtkXMLArchiveWriter and vtkXMLArchiveReader. Only serialization of
// vtkCamera (and its subclasses) is currently implemented.
// .SECTION See Also
// vtkSerializationHelperMap vtkSerializationHelper

#ifndef __vtkRenderingSerializationHelper_h
#define __vtkRenderingSerializationHelper_h

#include "vtkSerializationHelper.h"

class vtkCamera;
class vtkSerializer;

class VTK_EXPORT vtkRenderingSerializationHelper : public vtkSerializationHelper
{
public:
  static vtkRenderingSerializationHelper *New();
  vtkTypeRevisionMacro(vtkRenderingSerializationHelper, vtkSerializationHelper);
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
  vtkRenderingSerializationHelper();
  ~vtkRenderingSerializationHelper() {}

  // Description:
  // Serialize a vtkCamera
  void SerializeCamera(vtkCamera *camera, vtkSerializer *serializer);

private:
  vtkRenderingSerializationHelper(const vtkRenderingSerializationHelper&);  // Not implemented.
  void operator=(const vtkRenderingSerializationHelper&);  // Not implemented.
  //ETX
};

#endif
