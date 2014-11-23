//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/bridge/cgm/TranslateOperator.h"

#include "smtk/bridge/cgm/Bridge.h"
#include "smtk/bridge/cgm/CAUUID.h"
#include "smtk/bridge/cgm/Engines.h"
#include "smtk/bridge/cgm/TDUUID.h"

#include "smtk/model/CellEntity.h"
#include "smtk/model/Manager.h"
#include "smtk/model/ModelEntity.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/DoubleItem.h"
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
#include "Body.hpp"

#include "smtk/bridge/cgm/TranslateOperator_xml.h"

using namespace smtk::model;

namespace smtk {
  namespace bridge {
    namespace cgm {

// local helper
bool TranslateOperator::ableToOperate()
{
  return this->specification()->isValid();
}

smtk::model::OperatorResult TranslateOperator::operateInternal()
{
  smtk::attribute::DoubleItemPtr offset = this->findDouble("offset");
  int keepInputs = this->findInt("keep inputs")->value();
  ModelEntities bodiesIn = this->associatedEntitiesAs<ModelEntities>();

  ModelEntities::iterator it;
  DLIList<Body*> cgmBodiesIn;
  DLIList<Body*> cgmBodiesOut;
  Body* cgmBody;
  DLIList<RefFace*> allCGMFaces;
  DLIList<RefFace*> cgmFaces;
  for (it = bodiesIn.begin(); it != bodiesIn.end(); ++it)
    {
    cgmBody = dynamic_cast<Body*>(this->cgmEntity(*it));
    if (cgmBody)
      {
      cgmBodiesIn.append(cgmBody);
      cgmBody->ref_faces(cgmFaces);
      allCGMFaces += cgmFaces;
      cgmFaces.clean_out(); // just in case... but it appears that ref_faces() copies over cgmFaces each time.
      }
    if (!keepInputs)
      this->manager()->eraseModel(*it);
    }

  int nb = cgmBodiesIn.size();
  CubitVector delta(offset->value(0),offset->value(1),offset->value(2));

  DLIList<Body*> all_bodies;
  CubitStatus status = GeometryModifyTool::instance()->tweak_move(
    allCGMFaces, delta, all_bodies);
  if (status != CUBIT_SUCCESS || all_bodies.size() != nb)
    {
    std::cerr
      << "Failed to translate bodies or wrong number"
      << " (" << all_bodies.size() << " != " << nb << ")"
      << " of resulting bodies.\n";
    return this->createResult(smtk::model::OPERATION_FAILED);
    }

  smtk::model::OperatorResult result = this->createResult(
    smtk::model::OPERATION_SUCCEEDED);
  smtk::attribute::ModelEntityItem::Ptr resultBodies =
    result->findModelEntity("bodies");

  Bridge* bridge = this->cgmBridge();
  int numBodiesOut = cgmBodiesOut.size();
  resultBodies->setNumberOfValues(numBodiesOut);

  for (int i = 0; i < numBodiesOut; ++i)
    {
    cgmBody = cgmBodiesOut.get_and_step();
    if (!cgmBody)
      continue;

    smtk::bridge::cgm::TDUUID* refId = smtk::bridge::cgm::TDUUID::ofEntity(cgmBody, true);
    smtk::common::UUID entId = refId->entityId();
    smtk::model::Cursor smtkEntry(this->manager(), entId);
    if (bridge->transcribe(smtkEntry, smtk::model::BRIDGE_EVERYTHING, false))
      resultBodies->setValue(i, smtkEntry);
    }

  return result;
}

    } // namespace cgm
  } //namespace bridge
} // namespace smtk

smtkImplementsModelOperator(
  smtk::bridge::cgm::TranslateOperator,
  cgm_translate,
  "translate",
  TranslateOperator_xml,
  smtk::bridge::cgm::Bridge);
