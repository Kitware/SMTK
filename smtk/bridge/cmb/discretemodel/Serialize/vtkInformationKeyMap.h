//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
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

#ifndef __smtkcmb_vtkInformationKeyMap_h
#define __smtkcmb_vtkInformationKeyMap_h

#include "vtkSMTKDiscreteModelModule.h" // For export macro
#include "vtkObject.h"


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
