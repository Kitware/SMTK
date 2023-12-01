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

#include "smtk/graph/operators/CreateArcType.h"
#include "smtk/graph/operators/CreateArcType_xml.h"

#include "smtk/graph/Component.h"
#include "smtk/graph/Resource.h"
#include "smtk/graph/operators/CreateArc.h"
#include "smtk/graph/operators/DeleteArc.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/ReferenceItem.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/operation/MarkGeometry.h"
#include "smtk/operation/groups/ArcCreator.h"

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
namespace // anonymous
{

void appendArcTypes(
  std::unordered_set<smtk::string::Token>& arcTypes,
  const smtk::attribute::StringItem::Ptr& typeSource)
{
  for (const auto& arcType : *typeSource)
  {
    arcTypes.insert(arcType);
  }
}

} // anonymous namespace

CreateArcType::CreateArcType() = default;

CreateArcType::Result CreateArcType::operateInternal()
{
  auto resource = this->parameters()->associations()->valueAs<smtk::graph::ResourceBase>();
  smtk::string::Token arcTypeName = this->parameters()->findString("type name")->value();
  smtk::string::Token isDirected = this->parameters()->findString("directionality")->value();
  std::unordered_set<smtk::string::Token> fromArcTypes;
  std::unordered_set<smtk::string::Token> toArcTypes;
  Directionality directionality;
  switch (isDirected.id())
  {
    default:
    case "directed"_hash:
      directionality = Directionality::IsDirected;
      appendArcTypes(fromArcTypes, this->parameters()->findString("from node types"));
      appendArcTypes(toArcTypes, this->parameters()->findString("to node types"));
      break;
    case "undirected"_hash:
      directionality = Directionality::IsUndirected;
      appendArcTypes(fromArcTypes, this->parameters()->findString("end node types"));
      toArcTypes = fromArcTypes;
      break;
  }
  bool didAdd = resource->arcs().insertRuntimeArcType(
    resource.get(), arcTypeName, fromArcTypes, toArcTypes, directionality);

  auto result = this->createResult(
    didAdd ? smtk::operation::Operation::Outcome::SUCCEEDED
           : smtk::operation::Operation::Outcome::FAILED);
  if (didAdd)
  {
    // Mark the resource as modified.
    result->findResource("resource")->appendValue(resource);

    // Register an operation with the ArcCreator group for this arc type.
    Operation::Index opIndex = std::type_index(typeid(CreateArc)).hash_code();
    std::string arcDestinationItemName = "to node"; // for CreateArc
    if (auto mgrs = this->managers())
    {
      if (mgrs->contains<std::shared_ptr<CreateArcType::ArcCreationOperation>>())
      {
        auto indexer = mgrs->get<std::shared_ptr<CreateArcType::ArcCreationOperation>>();
        opIndex = indexer->operationForArcType(arcTypeName.data());
        arcDestinationItemName = indexer->destinationItemNameForArcType(arcTypeName.data());
      }
    }
    auto opMgr = this->manager();
    if (opMgr && opIndex)
    {
      smtk::operation::ArcCreator creatorGroup(opMgr);
      if (!creatorGroup.registerOperation(
            opIndex, { { arcTypeName.data() } }, arcDestinationItemName))
      {
        smtkErrorMacro(
          this->log(), "Could not register arc creator for \"" << arcTypeName.data() << "\" arcs.");
      }
    }
    if (!DeleteArc::registerDeleter(arcTypeName, resource.get(), opMgr))
    {
      smtkErrorMacro(
        this->log(), "Could not register arc deleter for \"" << arcTypeName.data() << "\" arcs.");
    }
  }
  return result;
}

const char* CreateArcType::xmlDescription() const
{
  return CreateArcType_xml;
}

} // namespace graph
} // namespace smtk
