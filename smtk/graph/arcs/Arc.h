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
#include "smtk/common/WeakReferenceWrapper.h"

#include "smtk/graph/arcs/Inverse.h"

namespace smtk
{
namespace graph
{

/// A basic arc type that restricts its endpoints.
///
/// The endpoint nodes must be types derived from smtk::graph::Component and
/// are specified as template parameters.
template<typename from_type, typename to_type, typename DerivedType = void>
class SMTK_ALWAYS_EXPORT Arc
{
public:
  typedef to_type ToType;
  typedef from_type FromType;
  typedef Inverse<DerivedType> InverseHandler;
  typedef smtk::WeakReferenceWrapper<ToType> Container;
  typedef ToType* iterator;
  typedef const ToType* const_iterator;

  /// Constructor used by API to implicitly insert an Arc.
  /// This is required for creating inverse arcs.
  Arc(const from_type& from)
    : m_from(from)
    , m_to()
  {
  }

  /// Force arcs to connect components of the proper type at construction.
  Arc(const FromType& from, ToType& to)
    : m_from(from)
    , m_to(smtk::weakRef<ToType>(to))
  {
    if (!InverseHandler::insert(to, const_cast<FromType&>(from)))
    {
      throw std::logic_error("Failed to insert inverse during Arc initialization.");
    }
  }

  Arc& operator=(ToType& to)
  {
    if (m_to != smtk::weakRef<ToType>(to))
    {
      this->erase(m_to);
      if (!this->insert(to).second)
      {
        throw std::logic_error("Failed to insert inverse durring Arc assignment.");
      }
    }
    return *this;
  }

  /// Return the origin (from) and destination (to) components of the arc.
  const FromType& from() const { return m_from; }
  const ToType& to() const { return m_to.get(); }
  ToType& to() { return m_to.get(); }

  iterator begin() { return m_to ? &m_to.get() : nullptr; }
  const_iterator begin() const { return m_to ? &m_to.get() : nullptr; }
  iterator end() { return m_to ? (&(m_to.get()) + 1) : nullptr; }
  const_iterator end() const { return m_to ? &m_to.get() + 1 : nullptr; }

  std::size_t count() const { return m_to ? 1 : 0; }

  std::pair<iterator, bool> insert(ToType& to, bool inverse = true)
  {
    bool canInsert = m_to.expired() || to.id() == m_to.get().id();

    std::pair<iterator, bool> result(end(), canInsert);

    if (canInsert)
    {
      if (inverse)
      {
        result.second = InverseHandler::insert(to, const_cast<FromType&>(m_from));
      }

      if (result.second)
      {
        result.second = m_to.expired();
        m_to = smtk::weakRef(to);
        result.first = begin();
      }
    }

    return result;
  }

  std::size_t erase(const ToType& to, bool inverse = true)
  {
    std::size_t numArcsErased = 0;
    if (!m_to.expired() && m_to == smtk::weakRef<const ToType>(to))
    {
      if (inverse)
      {
        numArcsErased += InverseHandler::erase(m_to, m_from);
      }
      ++numArcsErased;
      m_to.reset();
    }
    return numArcsErased;
  }

  void clear()
  {
    this->erase(m_to);
    m_to.reset();
  }

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
      auto& arcs = std::static_pointer_cast<smtk::graph::ResourceBase>(lhs.resource())->arcs();
      if (!arcs.template contains<SelfType>(lhs.id()))
      {
        arcs.template emplace<SelfType>(lhs.id(), lhs);
      }
      return arcs.template get<SelfType>().at(lhs.id());
    }

  public:
    const SelfType& get(const FromType& lhs) const { return self(lhs); }
    SelfType& get(const FromType& lhs) { return self(lhs); }
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
  Container m_to;
};
} // namespace graph
} // namespace smtk

#endif
