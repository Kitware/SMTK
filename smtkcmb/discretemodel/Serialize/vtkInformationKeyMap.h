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
// .NAME vtkInformationKeyMap - Map from string to information key instances
// .SECTION Description
// This class is used to access information keys given a string in the
// form KeyLocation.KeyName. For example: vtkDataObject.DATA_OBJECT
// (note the lack of () at the end).
// You have to manually register the keys in the map before they can
// be accessed.
// Note that all of the key instances are stored using smart pointers.
// Make sure to call RemoveAllKeys() before exit to avoid leak warnings
// from vtkDebugLeaks.

#ifndef __vtkInformationKeyMap_h
#define __vtkInformationKeyMap_h

#include "vtkSMTKDiscreteModelModule.h" // For export macro
#include "vtkObject.h"
#include "cmbSystemConfig.h"

#include <string>

//BTX
class vtkInformationKey;
//ETX

class VTKSMTKDISCRETEMODEL_EXPORT vtkInformationKeyMap : public vtkObject
{
public:
  static vtkInformationKeyMap *New();
  vtkTypeMacro(vtkInformationKeyMap,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Register a key with the map. The key will be KeyLocation::KeyName.
  static void RegisterKey(vtkInformationKey* key);

  // Description:
  // Lookup a key instance registered with the map using its location
  // and name.
  static vtkInformationKey* FindKey(const char* name);

  // Description:
  // Given a key instance, returns its name used to store it in the
  // map. Implemented as:
  // key->GetLocation() + "." + key->GetName()
  static std::string GetFullName(vtkInformationKey* key);

  // Description:
  // Removes all keys from the map.
  static void RemoveAllKeys();

protected:
  vtkInformationKeyMap();
  ~vtkInformationKeyMap();

private:
  vtkInformationKeyMap(const vtkInformationKeyMap&);  // Not implemented.
  void operator=(const vtkInformationKeyMap&);  // Not implemented.
};

#endif
