//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_graph_Arc_h
#define smtk_graph_Arc_h

#include "smtk/common/CompilerInformation.h"

namespace smtk
{
namespace graph
{

/// A basic arc type that restricts its endpoints.
///
/// The endpoint nodes must be types derived from smtk::graph::Component and
/// are specified as template parameters.
template<typename from_type, typename to_type>
class SMTK_ALWAYS_EXPORT Arc
{
public:
  typedef to_type ToType;
  typedef from_type FromType;

  /// Force arcs to connect components of the proper type at construction.
  Arc(const FromType& from, ToType& to)
    : m_from(from)
    , m_to(to)
  {
  }

  /// Return the origin (from) and destination (to) components of the arc.
  const FromType& from() const { return m_from; }
  const ToType& to() const { return m_to; }
  ToType& to() { return m_to; }

  /// An API for accessing this class's information using
  /// smtk::graph::Component's API.
  template<typename SelfType>
  class API
  {
    static_assert(
      std::is_base_of<Arc, SelfType>::value,
      "Invalid cast: cannot access Arc from unrelated type.");

  protected:
    const SelfType& self(const FromType& lhs) const
    {
      return std::static_pointer_cast<smtk::graph::ResourceBase>(lhs.resource())
        ->arcs()
        .template get<SelfType>()
        .at(lhs.id());
    }

    SelfType& self(const FromType& lhs)
    {
      return std::static_pointer_cast<smtk::graph::ResourceBase>(lhs.resource())
        ->arcs()
        .template get<SelfType>()
        .at(lhs.id());
    }

  public:
    const ToType& get(const FromType& lhs) const { return self(lhs).to(); }
    ToType& get(const FromType& lhs) { return self(lhs).to(); }
    bool contains(const FromType& lhs) const
    {
      return std::static_pointer_cast<smtk::graph::ResourceBase>(lhs.resource())
        ->arcs()
        .template get<SelfType>()
        .contains(lhs.id());
    }
  };

private:
  const FromType& m_from;
  ToType& m_to;
};
} // namespace graph
} // namespace smtk

#endif
