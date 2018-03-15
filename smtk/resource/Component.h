//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME Component.h - Abstract base class for SMTK Resource Components
// .SECTION Description
//   A SMTK Component is a peice of a Resource such as an Attribute, ModelEntity,
//  or MeshEntity
// .SECTION See Also

#ifndef smtk_resource_Component_h
#define smtk_resource_Component_h

#include "smtk/resource/PersistentObject.h"

#include <string>

namespace smtk
{
namespace resource
{

class SMTKCORE_EXPORT Component : public PersistentObject
{
  friend class Resource;

public:
  typedef std::function<void(const ComponentPtr&)> Visitor;

  smtkTypeMacro(Component);
  smtkSuperclassMacro(smtk::resource::PersistentObject);
  smtkSharedFromThisMacro(smtk::resource::PersistentObject);
  virtual ~Component();
  virtual const ResourcePtr resource() const = 0;

protected:
  Component();
};
}
}

#endif // smtk_resource_Component_h
