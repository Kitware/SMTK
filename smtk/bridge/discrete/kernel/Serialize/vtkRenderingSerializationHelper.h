//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkRenderingSerializationHelper - Concrete serialization helper for
// Rendering package.
// .SECTION Description
// Concrete class for serialization of vtkObjects in the Rendering package using
// vtkXMLArchiveWriter and vtkXMLArchiveReader. Only serialization of
// vtkCamera (and its subclasses) is currently implemented.
// .SECTION See Also
// vtkSerializationHelperMap vtkSerializationHelper

#ifndef __smtkdiscrete_vtkRenderingSerializationHelper_h
#define __smtkdiscrete_vtkRenderingSerializationHelper_h

#include "smtk/bridge/discrete/kernel/vtkSMTKDiscreteModelModule.h" // For export macro
#include "vtkSerializationHelper.h"


class vtkCamera;
class vtkSerializer;

class VTKSMTKDISCRETEMODEL_EXPORT vtkRenderingSerializationHelper : public vtkSerializationHelper
{
public:
  static vtkRenderingSerializationHelper *New();
  vtkTypeMacro(vtkRenderingSerializationHelper, vtkSerializationHelper);
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
