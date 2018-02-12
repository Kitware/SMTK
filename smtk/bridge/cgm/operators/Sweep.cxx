//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/bridge/cgm/operators/Sweep.h"

#include "smtk/bridge/cgm/CAUUID.h"
#include "smtk/bridge/cgm/Engines.h"
#include "smtk/bridge/cgm/Session.h"
#include "smtk/bridge/cgm/TDUUID.h"

#include "smtk/io/Logger.h"

#include "smtk/model/CellEntity.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/StringItem.h"

#include "Body.hpp"
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
#include "RefGroup.hpp"

#include "smtk/bridge/cgm/Sweep_xml.h"

using namespace smtk::model;
using smtk::attribute::ModelEntityItemPtr;

namespace smtk
{
namespace bridge
{
namespace cgm
{

smtk::operation::NewOpResult Sweep::operateInternal()
{
  // 0 = extrude, 1 = revolve, 2 = helix, 3 = sweep along curve:
  int sweepOp = this->findInt("construction method")->value();
  int keepInputs = this->findInt("keep inputs")->value();
  smtk::model::EntityRefArray expunged;
  DLIList<RefEntity*> cgmThingsToSweep;
  DLIList<RefEdge*> cgmSweepPath;
  bool ok = true;
  ok &= this->cgmEntities(
    *this->specification()->associations().get(), cgmThingsToSweep, keepInputs, expunged);
  if (sweepOp == 3) // sweep along curve
    ok &= this->cgmEntities(
      *this->findModelEntity("sweep path").get(), cgmSweepPath, keepInputs, expunged);

  if (!ok)
    return this->createResult(smtk::operation::Operator::OPERATION_FAILED);

  DLIList<Body*> cgmResults;
  CubitStatus s;
  switch (sweepOp)
  {
    case 0: // extrude
    {
      s = CUBIT_FAILURE;
      smtk::attribute::DoubleItemPtr sweepDistItem = this->findDouble("sweep distance");
      double sweepDist = sweepDistItem->isEnabled() ? sweepDistItem->value() : 0.0;
      smtk::attribute::DoubleItemPtr extrudeDirItem = this->findDouble("extrusion direction");
      double draftAngle = this->findDouble("draft angle")->value();
      int draftType = this->findInt("draft type")->value();
      // Sweep along a direction (if given)
      if (extrudeDirItem->isEnabled())
      {
        std::vector<double> sweepDir(extrudeDirItem->begin(), extrudeDirItem->end());
        double sweepDirMag2 = 0.;
        for (int i = 0; i < 3; ++i)
          sweepDirMag2 += sweepDir[i] * sweepDir[i];
        if (sweepDirMag2 != 0.)
        {
          // Scale the sweep direction to match the distance specified:
          if (sweepDistItem->isEnabled() && sweepDist != 0.0)
          {
            double scaleFactor = sweepDist / sqrt(sweepDirMag2);
            for (int i = 0; i < 3; ++i)
              sweepDir[i] *= scaleFactor;
          }
          // Perform a sweep along a specified direction
          s = GeometryModifyTool::instance()->sweep_translational(cgmThingsToSweep,
            CubitVector(&sweepDir[0]), draftAngle, draftType,
            /* switch_side, unused */ CUBIT_FALSE,
            /* rigid, unused */ CUBIT_FALSE,
            /* anchor_entity */ CUBIT_FALSE, keepInputs, cgmResults);
        }
      }
      // Sweep direction unspecified. Try a perpendicular sweep.
      if (s == CUBIT_FAILURE && !extrudeDirItem->isEnabled() && sweepDistItem->isEnabled() &&
        sweepDist != 0.0)
      {
        s = GeometryModifyTool::instance()->sweep_perpendicular(cgmThingsToSweep, sweepDist,
          draftAngle, draftType,
          /* switch_side, unused */ CUBIT_FALSE,
          /* rigid, unused */ CUBIT_FALSE,
          /* anchor_entity */ CUBIT_FALSE, keepInputs, cgmResults);
      }
      if (s == CUBIT_FAILURE)
      {
        smtkInfoMacro(log(), "At least one of the \"extrusion direction\" and \"sweep distance\" "
                             "items must be enabled and set to a non-zero value.");
      }
    }
    break;
    case 1: // revolve
    {
      double sweepAngle = this->findDouble("sweep angle")->value();
      smtk::attribute::DoubleItemPtr sweepBaseItem = this->findDouble("axis base point");
      smtk::attribute::DoubleItemPtr sweepAxisItem = this->findDouble("axis of revolution");
      std::vector<double> sweepBase(sweepBaseItem->begin(), sweepBaseItem->end());
      std::vector<double> sweepAxis(sweepAxisItem->begin(), sweepAxisItem->end());
      s = GeometryModifyTool::instance()->sweep_rotational(cgmThingsToSweep,
        CubitVector(&sweepBase[0]), CubitVector(&sweepAxis[0]), sweepAngle, cgmResults,
        /* anchor_entity, unused */ false, keepInputs);
    }
    break;
    case 2: // helix
    {
      double helixAngle = this->findDouble("helix angle")->value();
      double pitch = this->findDouble("pitch")->value();
      bool righthanded = this->findInt("handedness")->value() ? true : false;
      smtk::attribute::DoubleItemPtr sweepBaseItem = this->findDouble("axis base point");
      smtk::attribute::DoubleItemPtr sweepAxisItem = this->findDouble("axis of revolution");
      std::vector<double> sweepBase(sweepBaseItem->begin(), sweepBaseItem->end());
      std::vector<double> sweepAxis(sweepAxisItem->begin(), sweepAxisItem->end());
      CubitVector cbtSweepBase(&sweepBase[0]);
      CubitVector cbtSweepAxis(&sweepAxis[0]);
      s = GeometryModifyTool::instance()->sweep_helical(cgmThingsToSweep, cbtSweepBase,
        cbtSweepAxis, pitch, helixAngle, righthanded,
        /* anchor_entity, unused */ CUBIT_FALSE, keepInputs, cgmResults);
    }
    break;
    case 3: // sweep along path
    {
      s = GeometryModifyTool::instance()->sweep_along_curve(cgmThingsToSweep, cgmSweepPath,
        cgmResults,
        /* anchor_entity, unused */ CUBIT_FALSE, keepInputs,
        /* draft_angle, unused */ 0.0,
        /* draft_type, unused */ 0,
        /* rigid, unused */ CUBIT_FALSE);
    }
    break;
    default:
      smtkInfoMacro(log(), "Unexpected sweep construction method " << sweepOp << ".");
      s = CUBIT_FAILURE;
      break;
  }

  if (s != CUBIT_SUCCESS)
  {
    smtkInfoMacro(log(), "Failed to perform sweep (status " << s << ").");
    return this->createResult(smtk::operation::Operator::OPERATION_FAILED);
  }

  smtk::operation::NewOpResult result =
    this->createResult(smtk::operation::Operator::OPERATION_SUCCEEDED);

  this->addEntitiesToResult(cgmResults, result, CREATED);
  result->findModelEntity("expunged")->setValues(expunged.begin(), expunged.end());

  return result;
}

} // namespace cgm
} //namespace bridge
} // namespace smtk

smtkImplementsModelOperator(SMTKCGMSESSION_EXPORT, smtk::bridge::cgm::Sweep, cgm_sweep, "sweep",
  Sweep_xml, smtk::bridge::cgm::Session);
