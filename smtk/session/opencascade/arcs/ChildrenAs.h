//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_session_opencascade_ChildrenAs_h
#define smtk_session_opencascade_ChildrenAs_h

#include "smtk/graph/Resource.h"
#include "smtk/session/opencascade/Shape.h"

#include "TopoDS_Iterator.hxx"
#include "TopoDS_Shape.hxx"

namespace smtk
{
namespace session
{
namespace opencascade
{

template <typename XToType>
class ChildrenAs
{
public:
  typedef Shape FromType;
  typedef XToType ToType;
  typedef std::vector<std::reference_wrapper<const ToType> > Container;

  ChildrenAs(const FromType& from)
    : m_from(from)
  {
  }

  const FromType& from() const { return m_from; }
  Container to() const
  {
    Container container;
    this->visit([&container](const ToType& toType) {
      container.push_back(std::cref(toType));
      return false;
    });
    return container;
  };

  bool visit(const std::function<bool(const ToType&)>& fn) const;

  bool visit(const std::function<bool(ToType&)>& fn);

  template <typename SelfType>
  class API
  {
  protected:
    const SelfType& self(const typename SelfType::FromType& lhs) const
    {
      auto& arcs = std::static_pointer_cast<smtk::graph::ResourceBase>(lhs.resource())->arcs();
      if (!arcs.template contains<SelfType>(lhs.id()))
      {
        arcs.template emplace<SelfType>(lhs.id(), lhs);
      }
      return arcs.template get<SelfType>().at(lhs.id());
    }

    SelfType& self(const typename SelfType::FromType& lhs)
    {
      auto& arcs = std::static_pointer_cast<smtk::graph::ResourceBase>(lhs.resource())->arcs();
      if (!arcs.template contains<SelfType>(lhs.id()))
      {
        arcs.template emplace<SelfType>(lhs.id(), lhs);
      }
      return arcs.template get<SelfType>().at(lhs.id());
    }

  public:
    typename SelfType::Container get(const typename SelfType::FromType& lhs) const
    {
      return self(lhs).to();
    }

    bool contains(const typename SelfType::FromType& lhs) const
    {
      return std::static_pointer_cast<smtk::graph::ResourceBase>(lhs.resource())
        ->arcs()
        .template get<SelfType>()
        .contains(lhs.id());
    }

    bool visit(
      const FromType& lhs, const std::function<bool(const typename SelfType::ToType&)>& fn) const
    {
      return self(lhs).visit(fn);
    }

    bool visit(const FromType& lhs, const std::function<bool(typename SelfType::ToType&)>& fn)
    {
      return self(lhs).visit(fn);
    }
  };

protected:
  const FromType& m_from;
};

typedef ChildrenAs<Shape> Children;
}
}
}

#include "smtk/session/opencascade/Resource.h"
#include "smtk/session/opencascade/Session.h"

namespace smtk
{
namespace session
{
namespace opencascade
{

template <typename XToType>
bool ChildrenAs<XToType>::visit(
  const std::function<bool(const typename ChildrenAs<XToType>::ToType&)>& fn) const
{
  auto resource = static_pointer_cast<Resource>(m_from.resource());
  auto session = resource->session();
  TopoDS_Iterator it;
  for (it.Initialize(*m_from.data()); it.More(); it.Next())
  {
    auto uid = session->findID(it.Value());
    if (!uid)
    {
      continue;
    }
    auto node = dynamic_cast<ToType*>(resource->find(uid).get());
    if (!node)
    {
      continue;
    }
    if (fn(*node))
    {
      return true;
    }
  }
  return false;
}

template <typename XToType>
bool ChildrenAs<XToType>::visit(
  const std::function<bool(typename ChildrenAs<XToType>::ToType&)>& fn)
{
  auto resource = static_pointer_cast<Resource>(m_from.resource());
  auto session = resource->session();
  TopoDS_Iterator it;
  for (it.Initialize(*m_from.data()); it.More(); it.Next())
  {
    auto uid = session->findID(it.Value());
    if (!uid)
    {
      continue;
    }
    auto node = dynamic_cast<ToType*>(resource->find(uid).get());
    if (!node)
    {
      continue;
    }
    if (fn(*node))
    {
      return true;
    }
  }
  return false;
}
}
}
}

#endif
