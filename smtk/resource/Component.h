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
//   A SMTK Component is a piece of a Resource such as an Attribute, ModelEntity,
//  or MeshEntity
// .SECTION See Also

#ifndef smtk_resource_Component_h
#define smtk_resource_Component_h

#include "smtk/resource/ComponentLinks.h"
#include "smtk/resource/PersistentObject.h"
#include "smtk/resource/Properties.h"

#include <string>

namespace smtk
{
namespace resource
{
class Resource;

class SMTKCORE_EXPORT Component : public PersistentObject
{
  friend class Resource;

public:
  typedef std::function<void(const ComponentPtr&)> Visitor;
  typedef detail::ComponentLinks Links;
  typedef detail::ComponentProperties Properties;

  smtkTypeMacro(smtk::resource::Component);
  smtkSuperclassMacro(smtk::resource::PersistentObject);
  smtkSharedFromThisMacro(smtk::resource::PersistentObject);

  virtual ~Component();
  virtual const ResourcePtr resource() const = 0;

  Links& links() override { return m_links; }
  const Links& links() const override { return m_links; }

  Properties& properties() override { return m_properties; }
  const Properties& properties() const override { return m_properties; }

protected:
  Component();

private:
  Links m_links;
  Properties m_properties;
};
}
}

#endif // smtk_resource_Component_h
