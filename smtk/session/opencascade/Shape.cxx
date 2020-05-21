//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/session/opencascade/Shape.h"

#include "smtk/session/opencascade/Resource.h"
#include "smtk/session/opencascade/Session.h"

#include "TopoDS_Iterator.hxx"
#include "TopoDS_Shape.hxx"

namespace smtk
{
namespace session
{
namespace opencascade
{

const TopoDS_Shape* Shape::data() const
{
  return this->occResource()->session()->findShape(this->id());
}

TopoDS_Shape* Shape::data()
{
  return const_cast<TopoDS_Shape*>(const_cast<const Shape*>(this)->data());
}

Resource* Shape::occResource() const
{
  return dynamic_cast<Resource*>(this->resource().get());
}

void Shape::visitSubshapes(Visitor visitor)
{
  auto resource = this->occResource();
  auto session = resource->session();
  auto shape = session->findShape(this->id());
  if (!shape)
  {
    return;
  }
  TopoDS_Iterator it;
  for (it.Initialize(*shape); it.More(); it.Next())
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
    if (visitor(node))
    {
      break;
    }
  }
}
}
}
}
