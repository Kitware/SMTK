//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/session/opencascade/arcs/Parents.h"

#include "smtk/session/opencascade/Resource.h"

#include "TopoDS_Shape.hxx"
#include <TopExp.hxx>

namespace smtk
{
namespace session
{
namespace opencascade
{

Parents::Container Parents::to() const
{
  Container container;
  this->visit([&container](const Parents::ToType& toType) {
    container.push_back(std::cref(toType));
    return false;
  });
  return container;
};

bool Parents::visit(std::function<bool(const Parents::ToType&)> fn) const
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
      auto node = dynamic_cast<Shape*>(resource->find(uid).get());
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

bool Parents::visit(std::function<bool(Parents::ToType&)> fn)
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
      auto node = dynamic_cast<Shape*>(resource->find(uid).get());
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
