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
#include "vtkInformationKeyMap.h"

#include <vtkInformationKey.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>

#include <map>
#include <string>

vtkCxxRevisionMacro(vtkInformationKeyMap, "1774");
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

//----------------------------------------------------------------------------
vtkInformationKeyMap::vtkInformationKeyMap()
{
}

//----------------------------------------------------------------------------
vtkInformationKeyMap::~vtkInformationKeyMap()
{
}

//----------------------------------------------------------------------------
void vtkInformationKeyMap::RegisterKey(vtkInformationKey* key)
{
  std::string name = std::string(key->GetLocation()) + "." + key->GetName();
  vtkInformationKeyMapKeys.Keys[name] = key;
}

//----------------------------------------------------------------------------
vtkInformationKey* vtkInformationKeyMap::FindKey(const char* name)
{
  KeyMapType::iterator iter = vtkInformationKeyMapKeys.Keys.find(name);
  if (iter != vtkInformationKeyMapKeys.Keys.end())
    {
    return iter->second;
    }
  return 0;
}

//----------------------------------------------------------------------------
std::string vtkInformationKeyMap::GetFullName(vtkInformationKey* key)
{
  return std::string(key->GetLocation()) + "." + key->GetName();
}


//----------------------------------------------------------------------------
void vtkInformationKeyMap::RemoveAllKeys()
{
  vtkInformationKeyMapKeys.Keys.clear();
}

//----------------------------------------------------------------------------
void vtkInformationKeyMap::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
