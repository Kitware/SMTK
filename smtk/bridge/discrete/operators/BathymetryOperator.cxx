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
#include "smtk/extension/vtk/filter/vtkCMBApplyBathymetryFilter.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/MeshItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/StringItemDefinition.h"

#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"
#include "smtk/model/Operator.h"
#include "smtk/mesh/Collection.h"
#include "smtk/mesh/Displace.h"
#include "smtk/mesh/MeshSet.h"
#include "smtk/mesh/Manager.h"
#include "smtk/mesh/PointSet.h"

#include "vtkModelItem.h"
#include "vtkModel.h"
#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelWrapper.h"
#include "vtkDoubleArray.h"
#include "vtkFloatArray.h"
#include "vtkPolyData.h"
#include <vtksys/SystemTools.hxx>

#include "BathymetryOperator_xml.h"

using namespace smtk::model;

namespace smtk {
  namespace bridge {
  namespace discrete {

bool internal_bathyToAssociatedMeshes(
  BathymetryHelper* bathyHelper, vtkDataSet* bathyData,
  const smtk::model::Model& srcModel, const bool &removing,
  const double &radius, const bool &useHighLimit, 
  const double &eleHigh, const bool &useLowLimit, const double &eleLow,
  smtk::mesh::ManagerPtr meshMgr, smtk::mesh::MeshSets& modifiedMeshes)
{
  bool ok = true;
  if(!bathyHelper || !meshMgr )
    {
    return ok;
    }
  std::vector<smtk::mesh::CollectionPtr> meshCollections =
    meshMgr->associatedCollections(srcModel);
  if(meshCollections.size() == 0)
    {
    return ok;
    }
  if (!removing && !bathyData)
    {
    std::cerr << "No bathymetry dataset to use for meshes!\n";
    return false;
    }

  std::vector<smtk::mesh::CollectionPtr>::iterator it;
  for(it = meshCollections.begin(); it != meshCollections.end(); ++it)
    {
    smtk::mesh::MeshSet meshes = (*it)->meshes();
    if(removing)
      {
      // set back the cached Z values
      ok &= bathyHelper->resetMeshPointsZ(*it);
      }
    else
      {
      vtkNew<vtkPoints> bathyPoints;
      if(!bathyHelper->computeBathymetryPoints(bathyData, bathyPoints.GetPointer()) ||
         bathyPoints->GetNumberOfPoints() == 0)
        {
        std::cerr << "Failed to compuate bathymetry points to use for meshes!\n";
        return false;
        }

      // cache the original mesh points Z values first
      if(!bathyHelper->storeMeshPointsZ(*it))
        {
        return false;
        }
      smtk::mesh::elevation::clamp_controls clamp(useHighLimit, eleHigh, useLowLimit, eleLow);
      vtkDataArray* pointCoords = bathyPoints->GetData();
      if (pointCoords->GetDataType() == VTK_FLOAT)
        {
        vtkFloatArray *floatArray = static_cast<vtkFloatArray *>(pointCoords);
        ok &= smtk::mesh::elevate( floatArray->GetPointer(0),
                             bathyPoints->GetNumberOfPoints(),
                             meshes.points(),
                             radius,
                             clamp);
        }
      else if (pointCoords->GetDataType() == VTK_DOUBLE)
        {
        vtkDoubleArray *doubleArray = static_cast<vtkDoubleArray *>(pointCoords);
        ok &= smtk::mesh::elevate( doubleArray->GetPointer(0),
                               bathyPoints->GetNumberOfPoints(),
                               meshes.points(),
                               radius,
                               clamp);
        }

      }

    if(ok)
      {
      modifiedMeshes.insert(meshes);
      }
    }

  return ok;
}

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
  vtkDataSet* bathyPoints = NULL;
  smtk::attribute::StringItem::Ptr optypeItem =
    this->specification()->findString("operation");
  smtk::attribute::DoubleItemPtr aveRItem =
    this->specification()->findDouble("averaging elevation radius");
  smtk::attribute::DoubleItemPtr highZItem =
    this->specification()->findDouble("set highest elevation");
  smtk::attribute::DoubleItemPtr lowZItem =
    this->specification()->findDouble("set lowest elevation");
  double aveEleRadius = aveRItem ? aveRItem->value() : 0.0;
  double highElevation = highZItem ? highZItem->value() : 0.0;
  double lowElevation = lowZItem ? lowZItem->value() : 0.0;

  std::string optype = optypeItem->value();
  if(optype == "Remove Bathymetry")
    {
    if(!bathyHelper->hasModelBathymetry(inModel.entity()))
      // nothing to do, return success
      return this->createResult(OPERATION_SUCCEEDED);

    newModelPoints->ShallowCopy(masterModelPoly);
    }
  else// if(optype == "Apply Bathymetry")
    {

    filename = this->specification()->findFile("bathymetryfile")->value();
    if(!filename.empty() && bathyHelper->loadBathymetryFile(filename) &&
       (bathyPoints = bathyHelper->bathymetryData(filename)))
      {
      vtkNew<vtkCMBApplyBathymetryFilter> filter;

      filter->SetElevationRadius(aveEleRadius);
      filter->SetHighestZValue(highElevation);
      filter->SetLowestZValue(lowElevation);
      filter->SetUseHighestZValue(highZItem->isEnabled());
      filter->SetUseLowestZValue(lowZItem->isEnabled());

      filter->SetInputData(0, masterModelPoly);
      filter->SetInputData(1, bathyPoints );
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

    opsession->retranscribeModel(inModel);

    smtk::mesh::MeshSets modifiedMeshes;
    // update associated meshes
    if(!internal_bathyToAssociatedMeshes(
       bathyHelper, bathyPoints,
       inModel.as<smtk::model::Model>(),
       optype == "Remove Bathymetry",
       aveEleRadius, highZItem ? highZItem->isEnabled() : false, highElevation,
       lowZItem ? lowZItem->isEnabled() : false, lowElevation,
       this->manager()->meshes(), modifiedMeshes))
      {
      std::cout << "ERROR: Failed to apply bathymetry to associated meshes." << std::endl;
      }

    this->addEntityToResult(result, inModel, MODIFIED);
    result->findModelEntity("tess_changed")->setValue(inModel);

    if (modifiedMeshes.size() > 0)
      {
      smtk::attribute::MeshItemPtr resultMeshes =
        result->findMesh("mesh_modified");
      if(resultMeshes)
        resultMeshes->appendValues(modifiedMeshes);
      }

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
