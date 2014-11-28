//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/bridge/cgm/operators/CreateFaceOperator.h"

#include "smtk/bridge/cgm/Bridge.h"
#include "smtk/bridge/cgm/CAUUID.h"
#include "smtk/bridge/cgm/Engines.h"
#include "smtk/bridge/cgm/TDUUID.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/StringItem.h"

#include "CGMApp.hpp"
#include "DagType.hpp"
#include "CubitAttribManager.hpp"
#include "CubitCompat.hpp"
#include "CubitDefines.h"
#include "DLIList.hpp"
#include "InitCGMA.hpp"
#include "GeometryModifyTool.hpp"
#include "GeometryQueryEngine.hpp"
#include "GeometryQueryTool.hpp"
#include "RefEntity.hpp"
#include "RefEntityFactory.hpp"
#include "RefGroup.hpp"
#include "RefEdge.hpp"
#include "RefFace.hpp"

#include "smtk/bridge/cgm/CreateFaceOperator_xml.h"

namespace smtk {
  namespace bridge {
    namespace cgm {

// local helper
bool CreateFaceOperator::ableToOperate()
{
  return this->specification()->isValid();
}

smtk::model::OperatorResult CreateFaceOperator::operateInternal()
{
  smtk::attribute::ModelEntityItem::Ptr edgesItem =
    this->findModelEntity("edges");
  smtk::attribute::IntItem::Ptr surfTypeItem =
    this->findInt("surface type");
  smtk::attribute::IntItem::Ptr colorItem =
    this->findInt("color");

  int color = colorItem->value();
  GeometryType surfType = static_cast<GeometryType>(
    surfTypeItem->concreteDefinition()->discreteValue(
      surfTypeItem->discreteIndex()));
  switch (surfType)
    {
  case PLANE_SURFACE_TYPE:
  case BEST_FIT_SURFACE_TYPE:
    break;
  default:
    std::cerr << "Bad surf type " << surfType << "\n";
    return this->createResult(smtk::model::OPERATION_FAILED);
    }
  DLIList<RefEdge*> edgeList;
  for (std::size_t i = 0; i < edgesItem->numberOfValues(); ++i)
    {
    RefEdge* edg = this->cgmEntityAs<RefEdge*>(edgesItem->value(i));
    if (!edg)
      {
      std::cerr << "One or more edges were invalid " << edgesItem->value(i).name() << "\n";
      return this->createResult(smtk::model::OPERATION_FAILED);
      }
    edgeList.push(edg);
    }
  if (edgeList.size() <= 0)
    {
    std::cerr << "No edges provided.\n";
    return this->createResult(smtk::model::OPERATION_FAILED);
    }

  bool isFree = true;
  bool checkEdges = true;
  RefFace* cgmFace = GeometryModifyTool::instance()->make_RefFace(surfType, edgeList, isFree, NULL, checkEdges);
  if (!cgmFace)
    {
    std::cerr << "Failed to create face\n";
    return this->createResult(smtk::model::OPERATION_FAILED);
    }

  // Assign color to match vertex API that requires a color.
  cgmFace->color(color);

  smtk::model::OperatorResult result = this->createResult(
    smtk::model::OPERATION_SUCCEEDED);
  smtk::attribute::ModelEntityItem::Ptr resultVert =
    result->findModelEntity("face");

  Bridge* bridge = this->cgmBridge();
  smtk::bridge::cgm::TDUUID* refId = smtk::bridge::cgm::TDUUID::ofEntity(cgmFace, true);
  smtk::common::UUID entId = refId->entityId();
  smtk::model::Cursor smtkEntry(this->manager(), entId);
  if (bridge->transcribe(smtkEntry, smtk::model::BRIDGE_EVERYTHING, false))
    resultVert->setValue(0, smtkEntry);

  return result;
}

    } // namespace cgm
  } //namespace bridge
} // namespace smtk

smtkImplementsModelOperator(
  smtk::bridge::cgm::CreateFaceOperator,
  cgm_create_face,
  "create face",
  CreateFaceOperator_xml,
  smtk::bridge::cgm::Bridge);
