//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkSerializationHelperMap.h"

#include "vtkCommonSerializationHelper.h"
#include "vtkObjectFactory.h"
#include "vtkSerializationHelper.h"
#include "vtkSmartPointer.h"
#include "vtkToolkits.h"
#include <map>
#include <string>

#ifdef VTK_USE_RENDERING
 #include "vtkRenderingSerializationHelper.h"
#endif

vtkStandardNewMacro(vtkSerializationHelperMap);



bool vtkSerializationHelperMap::DefaultHelpersInstantiated = false;

namespace
{
typedef std::map<std::string, vtkSmartPointer<vtkSerializationHelper> > ClassHelperMapType;
struct vtkSerializationHelperMapInternals
{
  ClassHelperMapType ClassMap;
};

vtkSerializationHelperMapInternals vtkSerializationHelperMapClassMap;
}

vtkSerializationHelperMap::vtkSerializationHelperMap()
{
}

vtkSerializationHelperMap::~vtkSerializationHelperMap()
{
}

void vtkSerializationHelperMap::InstantiateDefaultHelpers()
{
  if (!DefaultHelpersInstantiated)
    {
    DefaultHelpersInstantiated = true;
    vtkSmartPointer<vtkCommonSerializationHelper> commonHelper =
      vtkSmartPointer<vtkCommonSerializationHelper>::New();
    commonHelper->RegisterWithHelperMap();
#ifdef VTK_USE_RENDERING
    vtkSmartPointer<vtkRenderingSerializationHelper> renderingHelper =
      vtkSmartPointer<vtkRenderingSerializationHelper>::New();
    renderingHelper->RegisterWithHelperMap();
#endif
    }
}

void vtkSerializationHelperMap::RegisterHelperForClass(const char *classType,
                                                          vtkSerializationHelper* helper)
{
  vtkSerializationHelperMapClassMap.ClassMap[classType] = helper;
}

void vtkSerializationHelperMap::UnRegisterHelperForClass(const char *classType,
                                                            vtkSerializationHelper* helper)
{
  ClassHelperMapType::iterator iter =
    vtkSerializationHelperMapClassMap.ClassMap.find(classType);
  if (iter != vtkSerializationHelperMapClassMap.ClassMap.end() &&
    iter->second == helper)
    {
    vtkSerializationHelperMapClassMap.ClassMap.erase(iter);
    }
}

void vtkSerializationHelperMap::RemoveAllHelpers()
{
  vtkSerializationHelperMapClassMap.ClassMap.clear();
}


bool vtkSerializationHelperMap::IsSerializable(vtkObject *obj)
{
  ClassHelperMapType::iterator iter =
    vtkSerializationHelperMapClassMap.ClassMap.find(obj->GetClassName());
  if (iter != vtkSerializationHelperMapClassMap.ClassMap.end())
    {
    return true;
    }

  return false;
}

int vtkSerializationHelperMap::Serialize(vtkObject *object,
                                            vtkSerializer *serializer)
{
  ClassHelperMapType::iterator iter =
    vtkSerializationHelperMapClassMap.ClassMap.find(object->GetClassName());
  if (iter == vtkSerializationHelperMapClassMap.ClassMap.end())
    {
    vtkGenericWarningMacro("Unable to serialize object: " << object->GetClassName());
    return 0;
    }

  iter->second->Serialize(object, serializer);

  return 1;
}

const char *vtkSerializationHelperMap::GetSerializationType(vtkObject *object)
{
  ClassHelperMapType::iterator iter =
    vtkSerializationHelperMapClassMap.ClassMap.find(object->GetClassName());
  if (iter == vtkSerializationHelperMapClassMap.ClassMap.end())
    {
    vtkGenericWarningMacro("Unable to get serialization type: " << object->GetClassName());
    return 0;
    }

  return iter->second->GetSerializationType(object);
}

vtkSerializationHelper* vtkSerializationHelperMap::GetHelper(const char *classType)
{
  ClassHelperMapType::iterator iter =
    vtkSerializationHelperMapClassMap.ClassMap.find(classType);
  if (iter == vtkSerializationHelperMapClassMap.ClassMap.end())
    {
    return 0;
    }

  return iter->second;
}

void vtkSerializationHelperMap::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
