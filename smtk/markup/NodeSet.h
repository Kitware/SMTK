//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_markup_NodeSet_h
#define smtk_markup_NodeSet_h

#include "smtk/markup/SpatialData.h"

#include "smtk/common/Visit.h"
#include "smtk/markup/AssignedIds.h" // For cell and side IDs.
#include "smtk/markup/IdSpace.h"

namespace smtk
{
namespace markup
{

/// An adaptation of subsets for representing subsets of points.
///
/// The domain of a node set will always be points while the domain of a side
/// set may vary (subsets of a discrete IdSpace or a continuous ParameterSpace).
class SMTKMARKUP_EXPORT NodeSet : public smtk::markup::SpatialData
{
public:
  smtkTypeMacro(smtk::markup::NodeSet);
  smtkSuperclassMacro(smtk::markup::SpatialData);

  template<typename... Args>
  NodeSet(Args&&... args)
    : smtk::markup::SpatialData(std::forward<Args>(args)...)
  {
  }

  ~NodeSet() override;

  /// Provide an initializer for resources to call after construction.
  void initialize(const nlohmann::json& data, smtk::resource::json::Helper& helper) override;

  bool setDomain(const std::weak_ptr<smtk::markup::AssignedIds>& domain);
  const std::weak_ptr<smtk::markup::AssignedIds>& domain() const;
  std::weak_ptr<smtk::markup::AssignedIds>& domain();

  /// The keys are entries of the primary IdSpace; the values are entries of the boundary-operator's domain.
  bool setNodes(
    const std::multimap<smtk::markup::AssignedIds::IdType, smtk::markup::AssignedIds::IdType>&
      nodes);
  const std::multimap<smtk::markup::AssignedIds::IdType, smtk::markup::AssignedIds::IdType>& nodes()
    const;
  std::multimap<smtk::markup::AssignedIds::IdType, smtk::markup::AssignedIds::IdType>& nodes();

protected:
  std::weak_ptr<smtk::markup::AssignedIds> m_domain;
  std::multimap<smtk::markup::AssignedIds::IdType, smtk::markup::AssignedIds::IdType> m_nodes;
};

} // namespace markup
} // namespace smtk

#endif // smtk_markup_NodeSet_h
