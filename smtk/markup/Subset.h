//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_markup_Subset_h
#define smtk_markup_Subset_h

#include "smtk/markup/DiscreteGeometry.h"

#include "smtk/markup/AssignedIds.h" // for member IDs
#include "smtk/markup/IdSpace.h"     // for domain access

namespace smtk
{
namespace markup
{
namespace arcs
{
struct BoundariesToShapes;
}

class SMTKMARKUP_EXPORT Subset : public smtk::markup::DiscreteGeometry
{
public:
  smtkTypeMacro(smtk::markup::Subset);
  smtkSuperclassMacro(smtk::markup::DiscreteGeometry);

  template<typename... Args>
  Subset(Args&&... args)
    : smtk::markup::DiscreteGeometry(std::forward<Args>(args)...)
  {
  }

  template<typename... Args>
  Subset(const std::shared_ptr<AssignedIds>& ids, Args&&... args)
    : smtk::markup::DiscreteGeometry(std::forward<Args>(args)...)
    , m_ids(ids)
  {
    if (m_ids)
    {
      m_idSpace = ids->space()->name();
    }
  }

  ~Subset() override;

  void initialize(const nlohmann::json& data, smtk::resource::json::Helper& helper) override;

  smtk::string::Token domainName() const { return m_idSpace; }
  bool setDomainName(smtk::string::Token idSpace);

  /// Return the IdSpace named by \a domainName().
  std::shared_ptr<IdSpace> domain() const;

  const AssignedIds& ids() const { return *m_ids; }

#if 0
  // The arcs we care about are the things this subset references.
  // The trick is that these arcs should be implicit.
  /// Return arcs pointing to parent (higher-dimensional) shapes that this shape bounds.
  ArcEndpointInterface<arcs::BoundariesToShapes, ConstArc, OutgoingArc> parents() const;
  ArcEndpointInterface<arcs::BoundariesToShapes, NonConstArc, OutgoingArc> parents();

  /// Return arcs pointing to children (lower-dimensional) shapes that bound this shape.
  ArcEndpointInterface<arcs::BoundariesToShapes, ConstArc, IncomingArc> children() const;
  ArcEndpointInterface<arcs::BoundariesToShapes, NonConstArc, IncomingArc> children();
#endif

protected:
  smtk::string::Token m_idSpace;
  std::shared_ptr<AssignedIds> m_ids;
};

} // namespace markup
} // namespace smtk

#endif // smtk_markup_Subset_h
