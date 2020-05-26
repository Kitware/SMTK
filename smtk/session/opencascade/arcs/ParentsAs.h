//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_session_opencascade_ParentsAs_h
#define smtk_session_opencascade_ParentsAs_h

#include "smtk/graph/Resource.h"
#include "smtk/session/opencascade/Shape.h"

#include "TopoDS_Shape.hxx"
#include <TopExp.hxx>

namespace smtk
{
namespace session
{
namespace opencascade
{

template <typename XToType>
class ParentsAs
{
public:
  typedef Shape FromType;
  typedef XToType ToType;
  typedef std::vector<std::reference_wrapper<const ToType> > Container;

  ParentsAs(const FromType& from)
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
  }

  bool visit(const std::function<bool(const ToType&)>&) const;

  bool visit(const std::function<bool(ToType&)>&);

  template <typename SelfType>
  class API
  {
  protected:
    SelfType& self(const typename SelfType::FromType& lhs) const
    {
      auto& arcs = std::static_pointer_cast<smtk::graph::ResourceBase>(lhs.resource())->arcs();
      if (!arcs.template contains<SelfType>(lhs.id()))
      {
        arcs.template emplace<SelfType>(lhs.id(), lhs);
      }
      return arcs.template get<SelfType>().at(lhs.id());
    }

  public:
    Container get(const typename SelfType::FromType& lhs) const { return self(lhs).to(); }

    bool contains(const typename SelfType::FromType& lhs) const
    {
      return std::static_pointer_cast<smtk::graph::ResourceBase>(lhs.resource())
        ->arcs()
        .template get<SelfType>()
        .contains(lhs.id());
    }

    bool visit(const FromType& lhs, std::function<bool(const typename SelfType::ToType&)> fn) const
    {
      return self(lhs).visit(fn);
    }

    bool visit(const FromType& lhs, std::function<bool(typename SelfType::ToType&)> fn)
    {
      return self(lhs).visit(fn);
    }
  };

private:
  const FromType& m_from;
};

typedef ParentsAs<Shape> Parents;
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
bool ParentsAs<XToType>::visit(
  const std::function<bool(const typename ParentsAs<XToType>::ToType&)>& fn) const
{
  auto resource = static_pointer_cast<smtk::session::opencascade::Resource>(m_from.resource());
  auto session = resource->session();

  if (auto shape = m_from.data())
  {
    // If our shape is a compound, it has no parent
    auto shapeType = shape->ShapeType();
    if (shapeType == TopAbs_COMPOUND)
    {
      return true;
    }

    TopTools_IndexedDataMapOfShapeListOfShape map;
    TopExp::MapShapesAndAncestors(resource->compound(), shapeType,
      static_cast<TopAbs_ShapeEnum>(static_cast<int>(shapeType) - 1), map);

    const TopTools_ListOfShape& parents = map.FindFromKey(*shape);

    for (const TopoDS_Shape& parent : parents)
    {
      auto uid = session->findID(parent);
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
  }
  return false;
}

template <typename XToType>
bool ParentsAs<XToType>::visit(const std::function<bool(typename ParentsAs<XToType>::ToType&)>& fn)
{
  auto resource = static_pointer_cast<smtk::session::opencascade::Resource>(m_from.resource());
  auto session = resource->session();

  if (auto shape = m_from.data())
  {
    // If our shape is a compound, it has no parent
    auto shapeType = shape->ShapeType();
    if (shapeType == TopAbs_COMPOUND)
    {
      return true;
    }

    TopTools_IndexedDataMapOfShapeListOfShape map;
    TopExp::MapShapesAndAncestors(
      *shape, shapeType, static_cast<TopAbs_ShapeEnum>(static_cast<int>(shapeType) - 1), map);

    Standard_Integer i, nE = map.Extent();
    for (i = 1; i < nE; i++)
    {
      auto uid = session->findID(map.FindKey(i));
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
  }
  return false;
}
}
}
}

#endif
