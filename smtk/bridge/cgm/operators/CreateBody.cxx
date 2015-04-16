//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/bridge/cgm/operators/CreateBody.h"

#include "smtk/bridge/cgm/Session.h"
#include "smtk/bridge/cgm/CAUUID.h"
#include "smtk/bridge/cgm/Engines.h"
#include "smtk/bridge/cgm/TDUUID.h"

#include "smtk/io/Logger.h"

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
#include "Body.hpp"
#include "RefEntity.hpp"
#include "RefEntityFactory.hpp"
#include "RefGroup.hpp"
#include "RefEdge.hpp"
#include "RefFace.hpp"
#include "RefVolume.hpp"

#include "smtk/bridge/cgm/CreateBody_xml.h"

namespace smtk {
  namespace bridge {
    namespace cgm {

smtk::model::OperatorResult CreateBody::operateInternal()
{
  int keepInputs = this->findInt("keep inputs")->value();

  // Accept a color to match vertex creation API that requires a color.
  smtk::attribute::IntItem::Ptr colorItem = this->findInt("color");
  int color = colorItem->value();

  DLIList<RefEntity*> entList;
  smtk::model::EntityRefArray expunged;
  if (!this->cgmEntities(*this->specification()->associations().get(), entList, keepInputs, expunged))
    {
    smtkInfoMacro(log(), "Could not find CGM entities for input cells.");
    return this->createResult(smtk::model::OPERATION_FAILED);
    }

  DLIList<RefEdge*> edgeList;
  DLIList<RefFace*> faceList;
  DLIList<RefVolume*> volumeList;
  for (int i = 0; i < entList.size(); ++i)
    {
    RefEntity* ent = entList.get_and_step();
    RefEdge* edge;
    RefFace* face;
    RefVolume* volume;
    if ((edge = dynamic_cast<RefEdge*>(ent)))
      {
      edgeList.push(edge);
      }
    else if ((face = dynamic_cast<RefFace*>(ent)))
      {
      faceList.push(face);
      }
    else if ((volume = dynamic_cast<RefVolume*>(ent)))
      {
      volumeList.push(volume);
      }
    else
      {
      smtkInfoMacro(log(), "An input entity was not an edge, face, or volume.");
      return this->createResult(smtk::model::OPERATION_FAILED);
      }
    }

  DLIList<Body*> cgmBodies;
  Body* bod;
  if (edgeList.size())
    {
    // maybe BEST_FIT_SURFACE_TYPE would be better?
    bod = GeometryModifyTool::instance()->make_Body(PLANE_SURFACE_TYPE, edgeList, /*refFace*/ NULL);
    if (!bod)
      {
      smtkInfoMacro(log(), "Could not create planar sheet-body from " << edgeList.size() << " edge(s).");
      return this->createResult(smtk::model::OPERATION_FAILED);
      }
    bod->color(color);
    cgmBodies.push(bod);
    }
  for (int i = 0; i < faceList.size(); ++i)
    {
    RefFace* face = faceList.get_and_step();
    bod = GeometryModifyTool::instance()->make_Body(face, /* extended_from: */ CUBIT_FALSE);
    if (!bod)
      {
      smtkInfoMacro(log(),
        "Could not create planar sheet-body from face "
        << (i + 1) << " of " << edgeList.size() << ".");
      return this->createResult(smtk::model::OPERATION_FAILED);
      }
    bod->color(color);
    cgmBodies.push(bod);
    }
  if (volumeList.size())
    {
    bod = GeometryModifyTool::instance()->make_Body(volumeList);
    if (!bod)
      {
      smtkInfoMacro(log(), "Could not create body from " << volumeList.size() << " volume(s).");
      return this->createResult(smtk::model::OPERATION_FAILED);
      }
    bod->color(color);
    cgmBodies.push(bod);
    }

  smtk::model::OperatorResult result = this->createResult(
    smtk::model::OPERATION_SUCCEEDED);

  this->addEntitiesToResult(cgmBodies, result, CREATED);
  result->findModelEntity("expunged")->setValues(expunged.begin(), expunged.end());

  return result;
}

    } // namespace cgm
  } //namespace bridge
} // namespace smtk

smtkImplementsModelOperator(
  CGMSMTK_EXPORT,
  smtk::bridge::cgm::CreateBody,
  cgm_create_body,
  "create body",
  CreateBody_xml,
  smtk::bridge::cgm::Session);
