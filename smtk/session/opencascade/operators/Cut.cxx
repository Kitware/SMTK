//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/session/opencascade/operators/Cut.h"
#include "smtk/session/opencascade/Resource.h"
#include "smtk/session/opencascade/Session.h"
#include "smtk/session/opencascade/Shape.h"

#include "smtk/operation/MarkGeometry.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ReferenceItem.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/ResourceItem.h"

#include "smtk/resource/Manager.h"

#include "smtk/common/Paths.h"

#include "smtk/session/opencascade/Cut_xml.h"

#include "BRepAlgoAPI_Cut.hxx"
#include "BRepPrimAPI_MakeBox.hxx"
#include "BRepTools.hxx"
#include "TopoDS_Iterator.hxx"
#include "TopoDS_Shape.hxx"
#include "gp_Pnt.hxx" // "gp/gp_Pnt.hxx"

namespace smtk
{
namespace session
{
namespace opencascade
{

Cut::Result Cut::operateInternal()
{
  smtk::session::opencascade::Resource::Ptr resource;
  smtk::session::opencascade::Session::Ptr session;
  auto result = this->createResult(Cut::Outcome::FAILED);
  this->prepareResourceAndSession(result, resource, session);

  auto workpieceItem = this->parameters()->associations();
  auto toolpieceItem = this->parameters()->findComponent("tools");

  auto created = result->findComponent("created");
  auto modified = result->findComponent("modified");
  auto expunged = result->findComponent("expunged");

  std::set<smtk::common::UUID> allIds;
  TopTools_ListOfShape workpieces;
  std::size_t nw = workpieceItem->numberOfValues();
  for (std::size_t ii = 0; ii < nw; ++ii)
  {
    auto wp = workpieceItem->value(ii);
    auto wps = session->findShape(wp->id());
    if (wps)
    {
      allIds.insert(wp->id());
      workpieces.Append(*wps);
    }
  }

  TopTools_ListOfShape toolpieces;
  std::size_t nt = toolpieceItem->numberOfValues();
  for (std::size_t ii = 0; ii < nt; ++ii)
  {
    auto tp = toolpieceItem->value(ii);
    auto tps = session->findShape(tp->id());
    if (tps)
    {
      allIds.insert(tp->id());
      toolpieces.Append(*tps);
    }
  }

  BRepAlgoAPI_Cut cutter;
  cutter.SetArguments(workpieces);
  cutter.SetTools(toolpieces);

  // Debug
  std::cout << "Cutter set to " << (cutter.NonDestructive() ? "non-destructive" : "destructive")
            << " "
            << "glue: " << static_cast<int>(cutter.Glue()) << " "
            << (cutter.CheckInverted() ? "check-inverted" : "non-checking") << "\n";

  // This performs the operation:
  cutter.Build();

  cutter.SimplifyResult(
    /*unifyEdges*/ true, /*unifyFaces*/ true); //, /*angleTol = Precision::Angular*/

  // Now process the results
  operation::MarkGeometry geom(resource);
  for (const auto& id : allIds)
  {
    auto shapeIn = session->findShape(id);
    const TopTools_ListOfShape& modShapes = cutter.Modified(*shapeIn);
    for (const auto& shape : modShapes)
    {
      auto uid = session->findID(shape);
      if (uid)
      {
        auto modShape = resource->find(uid);
        modified->appendValue(modShape);
        geom.markModified(modShape);
      }
      else
      {
        std::cerr << "Grrk! unknown shape marked modified.\n";
        auto topNode = resource->createShape<Shape>();
        session->addShape(topNode->id(), *shapeIn);
        created->appendValue(topNode);
        geom.markModified(topNode);
        std::ostringstream shapeName;
        shapeName << "Shape Grrk!";
        topNode->setName(shapeName.str());

        this->iterateChildren(*topNode, result);
      }
    }

    const TopTools_ListOfShape& newShapes = cutter.Generated(*shapeIn);
    std::size_t ii = 0;
    for (auto& shape : newShapes)
    {
      auto topNode = resource->createShape<Shape>();
      session->addShape(topNode->id(), shape);
      created->appendValue(topNode);
      geom.markModified(topNode);
      std::ostringstream shapeName;
      shapeName << "Shape " << ii++;
      topNode->setName(shapeName.str());

      this->iterateChildren(*topNode, result);
    }

    if (cutter.IsDeleted(*shapeIn))
    {
      // Remove the workpiece/tool shape that was wholly deleted by the operation.
      auto delShape = std::dynamic_pointer_cast<Shape>(resource->find(id));
      geom.markModified(delShape);
      expunged->appendValue(delShape);
      resource->remove(delShape);
      session->removeShape(id);
    }
    // TODO
    // Remove child shapes that were deleted by the operation.
  }

  result->findInt("outcome")->setValue(static_cast<int>(Cut::Outcome::SUCCEEDED));
  return result;
}

const char* Cut::xmlDescription() const
{
  return Cut_xml;
}

} // namespace opencascade
} // namespace session
} // namespace smtk
