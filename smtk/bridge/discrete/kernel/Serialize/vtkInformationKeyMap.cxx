//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkInformationKeyMap.h"

#include <vtkInformationKey.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>

#include <map>
#include <string>

vtkStandardNewMacro(vtkInformationKeyMap);

namespace
{
typedef std::map<std::string, vtkSmartPointer<vtkInformationKey> >KeyMapType;
struct vtkInformationKeyMapInternals
{
  KeyMapType Keys;
};

vtkInformationKeyMapInternals vtkInformationKeyMapKeys;
}

vtkInformationKeyMap::vtkInformationKeyMap()
{
}

vtkInformationKeyMap::~vtkInformationKeyMap()
{
}

void vtkInformationKeyMap::RegisterKey(vtkInformationKey* key)
{
  std::string name = std::string(key->GetLocation()) + "." + key->GetName();
  vtkInformationKeyMapKeys.Keys[name] = key;
}

vtkInformationKey* vtkInformationKeyMap::FindKey(const char* name)
{
  KeyMapType::iterator iter = vtkInformationKeyMapKeys.Keys.find(name);
  if (iter != vtkInformationKeyMapKeys.Keys.end())
    {
    return iter->second;
    }
  return 0;
}

std::string vtkInformationKeyMap::GetFullName(vtkInformationKey* key)
{
  return std::string(key->GetLocation()) + "." + key->GetName();
}

void vtkInformationKeyMap::RemoveAllKeys()
{
  vtkInformationKeyMapKeys.Keys.clear();
}

void vtkInformationKeyMap::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
