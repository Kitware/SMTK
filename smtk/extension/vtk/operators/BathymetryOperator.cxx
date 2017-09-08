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

#include "BathymetryHelper.h"
#include "smtk/extension/vtk/filter/vtkCMBApplyBathymetryFilter.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/MeshItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/StringItemDefinition.h"
#include "smtk/attribute/VoidItem.h"

#include "smtk/mesh/Collection.h"
#include "smtk/mesh/Manager.h"

#include "smtk/model/AuxiliaryGeometry.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"
#include "smtk/model/Operator.h"

#include "vtkDoubleArray.h"
#include "vtkFloatArray.h"
#include "vtkImageData.h"
#include "vtkPolyData.h"
#include "vtkUniformGrid.h"
#include <vtksys/SystemTools.hxx>

#include "BathymetryOperator_xml.h"

using namespace smtk::model;

namespace smtk
{
namespace model
{

BathymetryOperator::BathymetryOperator()
{
  this->bathyHelper = new smtk::model::BathymetryHelper();
}

BathymetryOperator::~BathymetryOperator()
{
  delete this->bathyHelper;
}

bool BathymetryOperator::ableToOperate()
{

  smtk::attribute::StringItem::Ptr optypeItem = this->specification()->findString("operation");
  std::string optype = optypeItem->value();
  // The auxiliary geometry and corresponding model must be valid
  if (optype != "Remove Bathymetry")
  {
    smtk::model::AuxiliaryGeometry auxGeo =
      this->specification()->findModelEntity("auxiliary geometry")->value();

    Model model = auxGeo.owningModel();
    bool isModelValid = model.isValid(), isAuxValid;
    if (!isModelValid)
    {
      smtkErrorMacro(this->log(), "No model specified!");
      return false;
    }
    if (optype == "Apply Bathymetry (Model&Mesh)" || optype == "Apply Bathymetry (Mesh Only)")
    {

      smtk::attribute::MeshItem::Ptr meshItem = this->specification()->findMesh("mesh");
      if (meshItem->values().size() <= 0)
      { // User should pick a mesh
        smtkErrorMacro(this->log(), "No mesh specified!");
        return false;
      }
    }
    isAuxValid = auxGeo.isValid();
    return isModelValid && isAuxValid;
  }
  // valid model for remove bathymetry
  else
  {
    Model model = this->specification()->findModelEntity("model")->value().as<Model>();
    if (!model.isValid())
    {
      smtkErrorMacro(this->log(), "No model specified to remove!");
      return false;
    }
    return true;
  }
}

OperatorResult BathymetryOperator::operateInternal()
{
  // Set up the common info for model and mesh
  smtk::attribute::StringItem::Ptr optypeItem = this->specification()->findString("operation");
  std::string optype = optypeItem->value();
  std::string opsessionName = this->session()->name();

  // decide whether we want to apply to mesh or model or both.
  bool ApplyToModel(0), ApplyToMesh(0);
  if (optype == "Apply Bathymetry (Auto)")
  {
    ApplyToModel = (opsessionName == "discrete" ? 1 : 0);
    ApplyToMesh = 1;
  }
  else if (optype == "Apply Bathymetry (Model&Mesh)")
  {
    ApplyToModel = 1;
    ApplyToMesh = 1;
  }
  else if (optype == "Apply Bathymetry (Model Only)")
  {
    ApplyToModel = 1;
  }
  else if (optype == "Apply Bathymetry (Mesh Only)")
  {
    ApplyToMesh = 1;
  }

  // gather info for bathymetry filter
  bool ok = false;
  std::string filename;
  vtkNew<vtkPolyData> newModelPoints;
  vtkDataSet* bathyPoints = NULL;
  smtk::attribute::DoubleItemPtr aveRItem =
    this->specification()->findDouble("averaging elevation radius");
  smtk::attribute::DoubleItemPtr highZItem =
    this->specification()->findDouble("set highest elevation");
  smtk::attribute::DoubleItemPtr lowZItem =
    this->specification()->findDouble("set lowest elevation");
  smtk::attribute::VoidItemPtr invertScalarsItem =
    this->specification()->findVoid("invert scalars");
  smtk::attribute::MeshItem::Ptr meshItem = this->specification()->findMesh("mesh");
  double aveEleRadius = aveRItem ? aveRItem->value() : 0.0;
  double highElevation = highZItem ? highZItem->value() : 0.0;
  double lowElevation = lowZItem ? lowZItem->value() : 0.0;
  bool invertScalars = invertScalarsItem ? invertScalarsItem->isEnabled() : false;
  EntityRef inModel;
  smtk::model::AuxiliaryGeometry auxGeo;

  // Apply BO to model
  if (optype != "Remove Bathymetry")
  {
    auxGeo = this->specification()->findModelEntity("auxiliary geometry")->value();
    inModel = auxGeo.owningModel();
  }
  else
  {
    inModel = this->findModelEntity("model")->value();
  }

  smtk::mesh::MeshList meshes;

  if ((ApplyToMesh && !meshItem) || optype == "Remove Bathymetry")
  { // Try to find the meshes on the model if not specified
    std::vector<smtk::mesh::CollectionPtr> meshCollections =
      this->manager()->meshes()->associatedCollections(inModel);

    for (auto it = meshCollections.begin(); it != meshCollections.end(); it++)
    {
      meshes.push_back((*it)->meshes());
    }
  }
  else
  {
    meshes = meshItem->values();
  }

  // Apply BO to mesh
  if ((ApplyToMesh || optype == "Remove Bathymetry") && (meshes.size() > 0))
  {
    if (optype == "Remove Bathymetry")
    {
      smtk::model::OperatorPtr undoWarpMesh = this->session()->op("undo warp mesh");
      undoWarpMesh->specification()->findMesh("mesh")->appendValues(meshes);

      smtk::model::OperatorResult undoWarpMeshResult = undoWarpMesh->operate();

      if (undoWarpMeshResult->findInt("outcome")->value() != smtk::model::OPERATION_SUCCEEDED)
      {
        return this->createResult(OPERATION_FAILED);
      }
    }
    else
    {
      smtk::model::OperatorPtr elevateMesh = this->session()->op("elevate mesh");

      elevateMesh->specification()->findMesh("mesh")->appendValues(meshes);
      elevateMesh->specification()->findModelEntity("auxiliary geometry")->setValue(auxGeo);
      elevateMesh->specification()->findDouble("radius")->setValue(aveEleRadius);
      elevateMesh->specification()->findDouble("max elevation")->setValue(highElevation);
      elevateMesh->specification()->findDouble("min elevation")->setValue(lowElevation);
      elevateMesh->specification()->findVoid("invert scalars")->setIsEnabled(invertScalars);

      smtk::model::OperatorResult elevateMeshResult = elevateMesh->operate();

      if (elevateMeshResult->findInt("outcome")->value() != smtk::model::OPERATION_SUCCEEDED)
      {
        return this->createResult(OPERATION_FAILED);
      }
    }
  }

  // masterModelPts holds all points from vertices, edges and faces
  vtkNew<vtkPoints> masterModelPts;
  masterModelPts->SetDataTypeToDouble();
  vtkNew<vtkPolyData> masterModelPoly;
  masterModelPoly->SetPoints(masterModelPts.GetPointer());

  if (!masterModelPoly.GetPointer())
  {
    return this->createResult(OPERATION_FAILED);
  }

  // a list to hold all the length  of entities
  std::vector<vtkIdType> entityLenthList;
  std::vector<double> zValues; // for remove bathymetry usage

  // create an entityrefMap to get all the entities with tessellation
  std::map<smtk::model::EntityRef, smtk::model::EntityRef> entityrefMap;
  if (1)
  {
    std::set<smtk::model::EntityRef> touched; // make this go out of scope soon.
    inModel.findEntitiesWithTessellation(entityrefMap, touched);
  }

  // loop through each entity and get all points
  std::map<smtk::model::EntityRef, smtk::model::EntityRef>::iterator cit;
  for (cit = entityrefMap.begin(); cit != entityrefMap.end(); ++cit)
  {
    vtkIdType npts =
      this->bathyHelper->GenerateRepresentationFromModel(masterModelPts.GetPointer(), cit->first);
    entityLenthList.push_back(npts);
  }

  // get points' z value as a vector from masterModelPts
  bathyHelper->GetZValuesFromMasterModelPts(masterModelPts.GetPointer(), zValues);

  if (optype == "Remove Bathymetry")
  {
    if (!inModel.hasFloatProperty(BO_elevation))
    {
      ok = true;
    }
    else
    {
      // set data for newModelPoints from your map!
      std::vector<double>* zPtr;
      zPtr = &inModel.floatProperty(BO_elevation);

      // update the z value in masterModelPoly then do a shallow copy
      ok = this->bathyHelper->SetZValuesIntoMasterModelPts(masterModelPts.GetPointer(), zPtr);
      newModelPoints->ShallowCopy(masterModelPoly.GetPointer());
      // Sending the point back to Tessellation
      vtkIdType startingIndex(0);
      vtkPoints* points = newModelPoints->GetPoints();

      std::vector<vtkIdType>::iterator indexList = entityLenthList.begin();
      for (cit = entityrefMap.begin(); cit != entityrefMap.end(); ++cit, ++indexList)
      {
        //set the points back
        this->bathyHelper->CopyCoordinatesToTessellation(points, cit->first, startingIndex);
        startingIndex += *indexList;
      }
    }
  }
  else if (ApplyToModel) // check if we want to apply to model
  {
    ok = true;
    filename = auxGeo.url();
    if (!filename.empty() && this->bathyHelper->loadBathymetryFile(filename) &&
      (bathyPoints = this->bathyHelper->bathymetryData(filename)))
    {
      vtkNew<vtkCMBApplyBathymetryFilter> filter;

      filter->SetElevationRadius(aveEleRadius);
      filter->SetHighestZValue(highElevation);
      filter->SetLowestZValue(lowElevation);
      filter->SetUseHighestZValue(highZItem->isEnabled());
      filter->SetUseLowestZValue(lowZItem->isEnabled());
      filter->SetInvertScalars(invertScalarsItem->isEnabled());

      filter->SetInputData(0, masterModelPoly.GetPointer());
      filter->SetInputData(1, bathyPoints);
      filter->SetNoOP(false);
      filter->Update();
      newModelPoints->ShallowCopy(filter->GetOutputDataObject(0));
      // Sending the point back to Tessellation
      vtkIdType startingIndex(0);
      vtkPoints* points = newModelPoints->GetPoints();

      std::vector<vtkIdType>::iterator indexList = entityLenthList.begin();
      for (cit = entityrefMap.begin(); cit != entityrefMap.end(); ++cit, ++indexList)
      {
        //set the points back
        this->bathyHelper->CopyCoordinatesToTessellation(points, cit->first, startingIndex);
        startingIndex += *indexList;
      }
    }
    else
    {
      return this->createResult(OPERATION_FAILED);
    }
  }

  // update the value of ok for model.
  if ((ApplyToMesh && !ApplyToModel) || optype == "Remove Bathymetry")
  {
    ok = true; // only to mesh case
  }

  OperatorResult result = this->createResult(ok ? OPERATION_SUCCEEDED : OPERATION_FAILED);
  result->findModelEntity("tess_changed")->setValue(inModel);
  if (ok)
  {
    if (optype == "Remove Bathymetry")
    {
      inModel.removeFloatProperty(BO_elevation);
    }
    else if (ApplyToModel)
    {
      // check existance, so we only add it once.
      if (!inModel.hasFloatProperty(BO_elevation))
      {
        inModel.setFloatProperty(BO_elevation, zValues);
      }
    }
  }

  return result;
}

} // namespace model
} // namespace smtk

smtkImplementsModelOperator(VTKSMTKOPERATORSEXT_EXPORT, smtk::model::BathymetryOperator,
  apply_bathymetry, "apply bathymetry", BathymetryOperator_xml, smtk::model::Session);
