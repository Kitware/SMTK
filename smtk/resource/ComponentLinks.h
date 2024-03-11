//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_resource_ComponentLinks_h
#define smtk_resource_ComponentLinks_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"

#include "smtk/common/Links.h"
#include "smtk/common/UUID.h"

#include "smtk/resource/Links.h"

namespace smtk
{
namespace resource
{
class Component;
class Resource;

namespace detail
{
struct SMTKCORE_EXPORT ComponentLinkBase
{
  virtual ~ComponentLinkBase() = default;
};

/// The ComponentLinks class is a component-specific API for manipulating
/// unidirectional links from a component to other Resources and Components.
/// Internally, the class is stateless; all calls are performed through the
/// component's resource.
class SMTKCORE_EXPORT ComponentLinks : public Links
{
public:
  typedef smtk::common::
    Links<smtk::common::UUID, smtk::common::UUID, smtk::common::UUID, int, ComponentLinkBase>
      Data;

  friend class smtk::resource::Component;

private:
  ComponentLinks(const Component*);

  Resource* leftHandSideResource() override;
  const Resource* leftHandSideResource() const override;

  const smtk::common::UUID& leftHandSideComponentId() const override;

  const Component* m_component;
};
} // namespace detail
} // namespace resource
} // namespace smtk

#endif // smtk_resource_ComponentLinks_h
