//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "BathymetryOperator.h"

#include "smtk/bridge/discrete/Session.h"
#include "smtk/bridge/discrete/BathymetryHelper.h"
#include "smtk/bridge/discrete/operation/vtkCMBApplyBathymetryFilter.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/DoubleItem.h"

#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"
#include "smtk/model/Operator.h"

#include "vtkModelItem.h"
#include "vtkModel.h"
#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelWrapper.h"
#include "vtkPolyData.h"

#include "BathymetryOperator_xml.h"

using namespace smtk::model;

namespace smtk {
  namespace bridge {

  namespace discrete {

BathymetryOperator::BathymetryOperator()
{
}

bool BathymetryOperator::ableToOperate()
{
  std::string filename = this->specification()->findString("bathymetrysource")->value();
  if (filename.empty())
    return false;

  smtk::model::Model model;
  return
    // The SMTK model must be valid
    (model = this->specification()->findModelEntity("model")->value().as<smtk::model::Model>()).isValid() &&
    // The CMB model must exist:
    this->discreteSession()->findModelEntity(model.entity())
    ;
}

OperatorResult BathymetryOperator::operateInternal()
{
  Session* opsession = this->discreteSession();

  smtk::model::EntityRef inModel =
    this->specification()->findModelEntity("model")->value();
  std::string filename = this->specification()->findString("bathymetrysource")->value();
  vtkDiscreteModelWrapper* modelWrapper =
    opsession->findModelEntity(inModel.entity());
  if (!modelWrapper)
    {
    return this->createResult(OPERATION_FAILED);
    }

  bool ok = false;
  vtkPointSet* bathyPoints = NULL;
  BathymetryHelper* bathyHelper = opsession->bathymetryHelper();
  if(bathyHelper->loadBathymetryFile(filename) &&
     (bathyPoints = bathyHelper->bathymetryData(filename)))
    {
    vtkNew<vtkCMBApplyBathymetryFilter> filter;
    smtk::attribute::DoubleItemPtr aveRItem =
      this->specification()->findDouble("averaging elevation radius");
    smtk::attribute::DoubleItemPtr highZItem =
      this->specification()->findDouble("set highest elevation");
    smtk::attribute::DoubleItemPtr lowZItem =
      this->specification()->findDouble("set lowest elevation");
    double aveEleRadius = aveRItem->value();
    double highElevation = highZItem->value();
    double lowElevation = lowZItem->value();

    filter->SetElevationRadius(aveEleRadius);
    filter->SetHighestZValue(highElevation);
    filter->SetLowestZValue(lowElevation);
    filter->SetUseHighestZValue(highZItem->isEnabled());
    filter->SetUseLowestZValue(lowZItem->isEnabled());
    vtkNew<vtkPolyData> tmpModelPoly;
    tmpModelPoly->Initialize();
    tmpModelPoly->SetPoints(modelWrapper->GetModel()->GetMesh().SharePointsPtr());
    filter->SetInputData(0, tmpModelPoly.GetPointer());
    filter->SetInputData(1, bathyPoints);
    filter->SetNoOP(false);
    filter->Update();

    this->m_op->SetModelPoints(vtkPointSet::SafeDownCast(
                               filter->GetOutputDataObject(0)));
    this->m_op->Operate(modelWrapper);
    ok = this->m_op->GetOperateSucceeded();
    }

  OperatorResult result =
    this->createResult(
      ok ?  OPERATION_SUCCEEDED : OPERATION_FAILED);
  if(ok)
    {
    this->addEntityToResult(result, inModel, MODIFIED);
    }

  return result;
}

Session* BathymetryOperator::discreteSession() const
{
  return dynamic_cast<Session*>(this->session());
}

    } // namespace discrete
  } // namespace bridge

} // namespace smtk

smtkImplementsModelOperator(
  SMTKDISCRETESESSION_EXPORT,
  smtk::bridge::discrete::BathymetryOperator,
  discrete_create_edges,
  "edit bathymetry",
  BathymetryOperator_xml,
  smtk::bridge::discrete::Session);
