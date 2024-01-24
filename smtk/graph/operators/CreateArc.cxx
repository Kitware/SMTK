//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/io/Logger.h"

#include "smtk/graph/operators/CreateArc.h"
#include "smtk/graph/operators/CreateArc_xml.h"

#include "smtk/graph/Component.h"
#include "smtk/graph/Resource.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/ReferenceItem.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/operation/MarkGeometry.h"

#include "smtk/resource/Manager.h"

#include "smtk/common/Paths.h"

#include <sstream>

// On Windows MSVC 2015+, something is included that defines
// a macro named ERROR to be 0. This causes smtkErrorMacro()
// to expand into garbage (because smtk::io::Logger::ERROR
// gets expanded to smtk::io::Logger::0).
#ifdef ERROR
#undef ERROR
#endif

using namespace smtk::string::literals;

namespace smtk
{
namespace graph
{

CreateArc::CreateArc() = default;

bool CreateArc::ableToOperate()
{
  bool ok = this->Superclass::ableToOperate();

  smtk::string::Token arcTypeName;
  smtk::graph::Component::Ptr from;
  smtk::graph::Component::Ptr to;
  ok &= this->fetchNodesAndCheckArcType(arcTypeName, from, to);

  return ok;
}

CreateArc::Result CreateArc::operateInternal()
{
  smtk::string::Token arcTypeName;
  smtk::graph::Component::Ptr fromNode;
  smtk::graph::Component::Ptr toNode;
  if (!this->fetchNodesAndCheckArcType(arcTypeName, fromNode, toNode))
  {
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  bool didConnect = fromNode->outgoing(arcTypeName).connect(toNode.get());
  auto result = this->createResult(
    didConnect ? smtk::operation::Operation::Outcome::SUCCEEDED
               : smtk::operation::Operation::Outcome::FAILED);
  if (didConnect)
  {
    // Mark nodes modified.
    result->findComponent("modified")->appendValue(fromNode);
    result->findComponent("modified")->appendValue(toNode);
  }
  return result;
}

bool CreateArc::fetchNodesAndCheckArcType(
  smtk::string::Token& arcTypeName,
  smtk::graph::Component::Ptr& from,
  smtk::graph::Component::Ptr& to)
{
  bool ok = true;
  // 1. Check that num(assoc) + num(to node) = 2
  ok &=
    (this->parameters()->associations()->numberOfValues() +
       this->parameters()->findReference("to node")->numberOfValues() ==
     2);
  // 2. Check that arc type is undirected if num(assoc) = 2
  smtk::graph::ArcImplementationBase* arcsOfType = nullptr;
  smtk::graph::ResourceBase* resource = nullptr;
  from = this->parameters()->associations()->valueAs<smtk::graph::Component>(0);
  if (from)
  {
    arcTypeName = this->parameters()->findString("arc type")->value();
    resource = from->parentResourceAs<smtk::graph::ResourceBase>();
    if (resource)
    {
      arcsOfType = resource->arcs().at<smtk::graph::ArcImplementationBase>(arcTypeName);
      if (arcsOfType)
      {
        ok &= (arcsOfType->directionality() == Directionality::IsDirected &&
               this->parameters()->associations()->numberOfValues() == 1) ||
          (arcsOfType->directionality() == Directionality::IsUndirected);
      }
      else
      {
        ok = false;
      }
    }
    else
    {
      ok = false;
    }
  }
  else
  {
    ok = false;
  }
  // 3. Check that arc accepts from and to nodes (assuming arc type exists).
  if (ok && arcsOfType)
  {
    if (this->parameters()->associations()->numberOfValues() > 1)
    {
      to = this->parameters()->associations()->valueAs<smtk::graph::Component>(1);
    }
    else
    {
      to = this->parameters()->findReference("to node")->valueAs<smtk::graph::Component>();
    }
    ok &= arcsOfType->acceptsRuntime(from.get(), to.get());
  }
  return ok;
}

const char* CreateArc::xmlDescription() const
{
  return CreateArc_xml;
}

} // namespace graph
} // namespace smtk
