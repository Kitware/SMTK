//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_markup_SideSet_h
#define smtk_markup_SideSet_h

#include "smtk/markup/SpatialData.h"

#include "smtk/common/Visit.h"
#include "smtk/markup/AssignedIds.h" // For cell and side IDs.
#include "smtk/markup/BoundaryOperator.h"
#include "smtk/markup/IdSpace.h"

namespace smtk
{
namespace markup
{

/// An adaptation of subsets for representing subsets of boundaries of spatial data.
class SMTKMARKUP_EXPORT SideSet : public smtk::markup::SpatialData
{
public:
  smtkTypeMacro(smtk::markup::SideSet);
  smtkSuperclassMacro(smtk::markup::SpatialData);

  template<typename... Args>
  SideSet(Args&&... args)
    : smtk::markup::SpatialData(std::forward<Args>(args)...)
  {
  }

  ~SideSet() override;

  /// Provide an initializer for resources to call after construction.
  void initialize(const nlohmann::json& data, smtk::resource::json::Helper& helper) override;

  bool setDomain(const std::weak_ptr<smtk::markup::AssignedIds>& domain);
  const std::weak_ptr<smtk::markup::AssignedIds>& domain() const;
  std::weak_ptr<smtk::markup::AssignedIds>& domain();

  bool setBoundaryOperator(const std::weak_ptr<smtk::markup::BoundaryOperator>& boundaryOperator);
  const std::weak_ptr<smtk::markup::BoundaryOperator>& boundaryOperator() const;
  std::weak_ptr<smtk::markup::BoundaryOperator>& boundaryOperator();

  /// The keys are entries of the primary IdSpace; the values are entries of the boundary-operator's domain.
  bool setSides(
    const std::multimap<smtk::markup::AssignedIds::IdType, smtk::markup::AssignedIds::IdType>&
      sides);
  const std::multimap<smtk::markup::AssignedIds::IdType, smtk::markup::AssignedIds::IdType>& sides()
    const;
  std::multimap<smtk::markup::AssignedIds::IdType, smtk::markup::AssignedIds::IdType>& sides();

protected:
  std::weak_ptr<smtk::markup::AssignedIds> m_domain;
  std::weak_ptr<smtk::markup::BoundaryOperator> m_boundaryOperator;
  std::multimap<smtk::markup::AssignedIds::IdType, smtk::markup::AssignedIds::IdType> m_sides;
};

} // namespace markup
} // namespace smtk

#endif // smtk_markup_SideSet_h
