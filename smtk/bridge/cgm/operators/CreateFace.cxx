//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/bridge/cgm/operators/CreateFace.h"

#include "smtk/bridge/cgm/CAUUID.h"
#include "smtk/bridge/cgm/Engines.h"
#include "smtk/bridge/cgm/Session.h"
#include "smtk/bridge/cgm/TDUUID.h"

#include "smtk/io/Logger.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/StringItem.h"

#include "CGMApp.hpp"
#include "CubitAttribManager.hpp"
#include "CubitCompat.hpp"
#include "CubitDefines.h"
#include "DLIList.hpp"
#include "DagType.hpp"
#include "GeometryModifyTool.hpp"
#include "GeometryQueryEngine.hpp"
#include "GeometryQueryTool.hpp"
#include "InitCGMA.hpp"
#include "RefEdge.hpp"
#include "RefEntity.hpp"
#include "RefEntityFactory.hpp"
#include "RefFace.hpp"
#include "RefGroup.hpp"

#include "smtk/bridge/cgm/CreateFace_xml.h"

namespace smtk
{
namespace bridge
{
namespace cgm
{

smtk::operation::OperationResult CreateFace::operateInternal()
{
  smtk::attribute::IntItem::Ptr surfTypeItem = this->findInt("surface type");
  smtk::attribute::IntItem::Ptr colorItem = this->findInt("color");
  int keepInputs = this->findInt("keep inputs")->value(0);

  int color = colorItem->value();
  GeometryType surfType = static_cast<GeometryType>(
    surfTypeItem->concreteDefinition()->discreteValue(surfTypeItem->discreteIndex()));
  switch (surfType)
  {
    case PLANE_SURFACE_TYPE:
    case BEST_FIT_SURFACE_TYPE:
      break;
    default:
      smtkInfoMacro(log(), "Bad surf type " << surfType << ".");
      return this->createResult(smtk::operation::Operation::OPERATION_FAILED);
  }
  DLIList<RefEdge*> edgeList;
  smtk::model::EntityRefArray expunged;
  if (!this->cgmEntities(
        *this->specification()->associations().get(), edgeList, keepInputs, expunged))
  {
    smtkInfoMacro(log(), "One or more edges were invalid.");
    return this->createResult(smtk::operation::Operation::OPERATION_FAILED);
  }

  bool isFree = true;
  bool checkEdges = true;
  RefFace* cgmFace =
    GeometryModifyTool::instance()->make_RefFace(surfType, edgeList, isFree, NULL, checkEdges);
  if (!cgmFace)
  {
    smtkInfoMacro(log(), "Failed to create face.");
    return this->createResult(smtk::operation::Operation::OPERATION_FAILED);
  }

  // Assign color to match vertex API that requires a color.
  cgmFace->color(color);

  smtk::operation::OperationResult result =
    this->createResult(smtk::operation::Operation::OPERATION_SUCCEEDED);

  DLIList<RefFace*> cgmFacesOut;
  cgmFacesOut.push(cgmFace);
  this->addEntitiesToResult(cgmFacesOut, result, CREATED);
  result->findModelEntity("expunged")->setValues(expunged.begin(), expunged.end());

  return result;
}

} // namespace cgm
} //namespace bridge
} // namespace smtk

smtkImplementsModelOperation(SMTKCGMSESSION_EXPORT, smtk::bridge::cgm::CreateFace, cgm_create_face,
  "create face", CreateFace_xml, smtk::bridge::cgm::Session);
