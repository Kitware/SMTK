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
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/StringItemDefinition.h"

#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"
#include "smtk/model/Operator.h"

#include "vtkModelItem.h"
#include "vtkModel.h"
#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelWrapper.h"
#include "vtkPolyData.h"
#include <vtksys/SystemTools.hxx>

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
  smtk::model::Model model;
  bool isModelValid =
    // The SMTK model must be valid
    (model = this->specification()->findModelEntity("model")->value().as<smtk::model::Model>()).isValid() &&
    // The CMB discrete model must exist:
    this->discreteSession()->findModelEntity(model.entity());

  smtk::attribute::StringItem::Ptr optypeItem =
    this->specification()->findString("operation");
  std::string optype = optypeItem->value();
  if(optype == "Apply Bathymetry")
    {
    std::string filename = this->specification()->findFile("bathymetryfile")->value();
    isModelValid = !filename.empty();
    }
  return isModelValid;
}

OperatorResult BathymetryOperator::operateInternal()
{
  Session* opsession = this->discreteSession();

  smtk::model::EntityRef inModel =
    this->specification()->findModelEntity("model")->value();

  vtkDiscreteModelWrapper* modelWrapper =
    opsession->findModelEntity(inModel.entity());
  if (!modelWrapper)
    {
    return this->createResult(OPERATION_FAILED);
    }

  BathymetryHelper* bathyHelper = opsession->bathymetryHelper();
  vtkPolyData* masterModelPoly = bathyHelper->findOrShallowCopyModelPoly(
    inModel.entity(), opsession);
  if (!masterModelPoly)
    {
    return this->createResult(OPERATION_FAILED);
    }

  bool ok = false;
  std::string filename;
  vtkNew<vtkPolyData> newModelPoints;
  smtk::attribute::StringItem::Ptr optypeItem =
    this->specification()->findString("operation");
  std::string optype = optypeItem->value();
  if(optype == "Remove Bathymetry")
    {
    if(!bathyHelper->hasModelBathymetry(inModel.entity()))
      // nothing to do, return success
      return this->createResult(OPERATION_SUCCEEDED);

    newModelPoints->ShallowCopy(masterModelPoly);
    }
  else if(optype == "Apply Bathymetry")
    {
    filename = this->specification()->findFile("bathymetryfile")->value();

    vtkPointSet* bathyPoints = NULL;
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

      filter->SetInputData(0, masterModelPoly);
      filter->SetInputData(1, bathyPoints);
      filter->SetNoOP(false);
      filter->Update();
      newModelPoints->ShallowCopy(filter->GetOutputDataObject(0));
      }
    else 
      return this->createResult(OPERATION_FAILED);
    }

  this->m_op->SetModelPoints(newModelPoints.GetPointer());
  this->m_op->Operate(modelWrapper);
  ok = this->m_op->GetOperateSucceeded();
  OperatorResult result =
    this->createResult(
      ok ?  OPERATION_SUCCEEDED : OPERATION_FAILED);
  if(ok)
    {
    if(optype == "Remove Bathymetry")
      bathyHelper->removeModelBathymetry(inModel.entity());
    else if(optype == "Apply Bathymetry")
      bathyHelper->addModelBathymetry(inModel.entity(), filename);

    opsession->manager()->eraseModel(inModel);
    opsession->transcribe(inModel, SESSION_EVERYTHING, false);

    this->addEntityToResult(result, inModel, MODIFIED);
    result->findModelEntity("tess_changed")->setValue(inModel);
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
  discrete_edit_bathymetry,
  "edit bathymetry",
  BathymetryOperator_xml,
  smtk::bridge::discrete::Session);
