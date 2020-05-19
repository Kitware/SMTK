//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/session/opencascade/arcs/Children.h"

#include "smtk/session/opencascade/Resource.h"

#include "TopoDS_Iterator.hxx"
#include "TopoDS_Shape.hxx"

namespace smtk
{
namespace session
{
namespace opencascade
{

Children::Container Children::to() const
{
  Container container;
  this->visit([&container](const Children::ToType& toType) {
    container.push_back(std::cref(toType));
    return false;
  });
  return container;
};

bool Children::visit(std::function<bool(const Children::ToType&)> fn) const
{
  auto resource = static_pointer_cast<smtk::session::opencascade::Resource>(m_from.resource());
  auto session = resource->session();
  TopoDS_Iterator it;
  for (it.Initialize(*m_from.data()); it.More(); it.Next())
  {
    auto uid = session->findID(it.Value());
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
  return false;
}

bool Children::visit(std::function<bool(Children::ToType&)> fn)
{
  auto resource = static_pointer_cast<smtk::session::opencascade::Resource>(m_from.resource());
  auto session = resource->session();
  TopoDS_Iterator it;
  for (it.Initialize(*m_from.data()); it.More(); it.Next())
  {
    auto uid = session->findID(it.Value());
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
  return false;
}
}
}
}
