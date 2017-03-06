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
#include "smtk/attribute/MeshItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/StringItemDefinition.h"

#include "smtk/model/AuxiliaryGeometry.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"
#include "smtk/model/Operator.h"
#include "smtk/mesh/Collection.h"
#include "smtk/mesh/Displace.h"
#include "smtk/mesh/MeshSet.h"
#include "smtk/mesh/Manager.h"
#include "smtk/mesh/PointSet.h"

#include "vtkDoubleArray.h"
#include "vtkFloatArray.h"
#include "vtkImageData.h"
#include "vtkPolyData.h"
#include "vtkUniformGrid.h"
#include <vtksys/SystemTools.hxx>

#include "BathymetryOperator_xml.h"

using namespace smtk::model;

namespace
{
class ElevationStructuredMetadataForVTKData :
    public smtk::mesh::ElevationStructuredMetadata
{
public:
  bool isBlanked(int i, int j) const
    {
      int pos[3] = {i,j,0};
      return m_uniformGrid->IsPointVisible(
        vtkStructuredData::ComputePointIdForExtent(m_uniformGrid->GetExtent(),
                                                   pos));
    }

  vtkUniformGrid* m_uniformGrid;
};
}

namespace smtk {
  namespace model {

bool internal_bathyToAssociatedMeshes(
  BathymetryHelper* bathyHelper, vtkDataSet* bathyData,
  const Model& srcModel, const bool &removing,
  const double &radius, const bool &useHighLimit,
  const double &eleHigh, const bool &useLowLimit, const double &eleLow,
  smtk::mesh::ManagerPtr meshMgr, smtk::mesh::MeshSets& modifiedMeshes,
    smtk::attribute::MeshItem::Ptr meshItem)
{
  bool ok = true;
  std::vector<smtk::mesh::CollectionPtr> meshCollections;
  if(!bathyHelper || !meshMgr )
    {
    return ok;
    }
  // decide we work on selected mesh or all meshes
  if (meshItem ==nullptr)
    {
    meshCollections =
    meshMgr->associatedCollections(srcModel);
    }
  else
    {
    // convert meshItem into meshCollections
    for (attribute::MeshItem::const_mesh_it mit = meshItem->begin();
         mit != meshItem->end(); ++mit)
      {
      meshCollections.push_back(mit->collection());
      }
    }
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
      // cache the original mesh points Z values first
      if(!bathyHelper->storeMeshPointsZ(*it))
        {
        return false;
        }

      if (vtkImageData* imageInput = vtkImageData::SafeDownCast(bathyData))
        {
        smtk::mesh::ElevationStructuredMetadata metadata;
        for (int i=0; i<4; i++)
          {
          metadata.m_extent[i] = imageInput->GetExtent()[i];
          metadata.m_bounds[i] = imageInput->GetBounds()[i];
          }
        smtk::mesh::ElevationControls clamp(useHighLimit, eleHigh,
                                            useLowLimit, eleLow);

        if (imageInput->GetScalarType() == VTK_FLOAT)
          {
          ok &= smtk::mesh::elevate(
            metadata,
            static_cast<float*>(imageInput->GetScalarPointer()),
            meshes.points(),
            radius,
            clamp);
          }
        else if (imageInput->GetScalarType() == VTK_DOUBLE)
          {
          ok &= smtk::mesh::elevate(
            metadata,
            static_cast<double*>(imageInput->GetScalarPointer(0)),
            meshes.points(),
            radius,
            clamp);
          }
        }
      else if (vtkUniformGrid* gridInput =
               vtkUniformGrid::SafeDownCast(bathyData))
        {
        ElevationStructuredMetadataForVTKData metadata;
        for (int i=0; i<4; i++)
          {
          metadata.m_extent[i] = gridInput->GetExtent()[i];
          metadata.m_bounds[i] = gridInput->GetBounds()[i];
          }
        metadata.m_uniformGrid = gridInput;
        smtk::mesh::ElevationControls clamp(useHighLimit, eleHigh,
                                            useLowLimit, eleLow);

        if (gridInput->GetScalarType() == VTK_FLOAT)
          {
          ok &= smtk::mesh::elevate(
            metadata,
            static_cast<float*>(gridInput->GetScalarPointer()),
            meshes.points(),
            radius,
            clamp);
          }
        else if (gridInput->GetScalarType() == VTK_DOUBLE)
          {
          ok &= smtk::mesh::elevate(
            metadata,
            static_cast<double*>(gridInput->GetScalarPointer(0)),
            meshes.points(),
            radius,
            clamp);
          }
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

        smtk::mesh::ElevationControls clamp(useHighLimit, eleHigh, useLowLimit, eleLow);
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
  this->bathyHelper = new smtk::model::BathymetryHelper();
}

BathymetryOperator::~BathymetryOperator()
{
  delete this->bathyHelper;
}

bool BathymetryOperator::ableToOperate()
{

  smtk::attribute::StringItem::Ptr optypeItem =
    this->specification()->findString("operation");
  std::string optype = optypeItem->value();
  // The auxiliary geometry and corresponding model must be valid
  if (optype != "Remove Bathymetry")
  {
    smtk::model::AuxiliaryGeometry auxGeo = this->specification()->findModelEntity("auxiliary geometry")->value();

    Model model = auxGeo.owningModel();
    bool isModelValid = model.isValid(),isAuxValid;
    if (!isModelValid)
    {
      smtkErrorMacro(this->log(), "No model specified!");
      return false;
    }
    isAuxValid = auxGeo.isValid();
    return isModelValid && isAuxValid;
  }
  // valid model for remove bathymetry
  else
  {
    Model model  = this->specification()->findModelEntity("model")->value().as<Model>();
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
  smtk::attribute::StringItem::Ptr optypeItem =
    this->specification()->findString("operation");
  std::string optype = optypeItem->value();
  std::string opsessionName = this->session()->name();

  // decide whether we want to apply to mesh or model or both.
  bool ApplyToModel(0), ApplyToMesh(0);
  if (optype == "Apply Bathymetry (Auto)")
  {
    ApplyToModel = (opsessionName == "discrete" ? 1 : 0);
    ApplyToMesh = 1;
  }
  else if (optype =="Apply Bathymetry (Model&Mesh)")
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
  smtk::attribute::MeshItem::Ptr meshItem = this->specification()->findMesh("mesh");
  double aveEleRadius = aveRItem ? aveRItem->value() : 0.0;
  double highElevation = highZItem ? highZItem->value() : 0.0;
  double lowElevation = lowZItem ? lowZItem->value() : 0.0;
  EntityRef inModel;
  smtk::model::AuxiliaryGeometry auxGeo;

  // Apply BO to model
  if (optype != "Remove Bathymetry")
  {
    auxGeo = this->specification()->
      findModelEntity("auxiliary geometry")->value();
    inModel = auxGeo.owningModel();
  }
  else
  {
    inModel = this->findModelEntity("model")->value();
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
    inModel.findEntitiesWithTessellation(entityrefMap,touched);
    }

  // loop through each entity and get all points
  std::map<smtk::model::EntityRef, smtk::model::EntityRef>::iterator cit;
  for (cit = entityrefMap.begin(); cit != entityrefMap.end(); ++cit)
    {
    vtkIdType npts = this->bathyHelper->GenerateRepresentationFromModel(masterModelPts.GetPointer(), cit->first);
    entityLenthList.push_back(npts);
    }

  // get points' z value as a vector from masterModelPts
  bathyHelper->GetZValuesFromMasterModelPts(masterModelPts.GetPointer(), zValues);

  if(optype == "Remove Bathymetry")
  {
    if(!inModel.hasFloatProperty(BO_elevation))
    {
      ok = true;
    }
    else
    {
      // set data for newModelPoints from your map!
      std::vector<double>* zPtr;
      zPtr = &inModel.floatProperty(BO_elevation);

      // update the z value in masterModelPoly then do a shallow copy
      ok = this->bathyHelper->SetZValuesIntoMasterModelPts(masterModelPts.GetPointer(),
                                                      zPtr);
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
  else if(ApplyToModel) // check if we want to apply to model
  {
    ok = true;
    filename = auxGeo.url();
    if(!filename.empty() && this->bathyHelper->loadBathymetryFile(filename) &&
       (bathyPoints = this->bathyHelper->bathymetryData(filename)))
    {
      vtkNew<vtkCMBApplyBathymetryFilter> filter;

      filter->SetElevationRadius(aveEleRadius);
      filter->SetHighestZValue(highElevation);
      filter->SetLowestZValue(lowElevation);
      filter->SetUseHighestZValue(highZItem->isEnabled());
      filter->SetUseLowestZValue(lowZItem->isEnabled());

      filter->SetInputData(0, masterModelPoly.GetPointer());
      filter->SetInputData(1, bathyPoints );
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
      return this->createResult(OPERATION_FAILED);
  }

  // update the value of ok for model.
  if ( (ApplyToMesh && !ApplyToModel) || optype == "Remove Bathymetry")
    ok = true;// only to mesh case
  OperatorResult result =
    this->createResult(
      ok ?  OPERATION_SUCCEEDED : OPERATION_FAILED);
  result->findModelEntity("tess_changed")->setValue(inModel);
  if(ok)
  {
    if(optype == "Remove Bathymetry")
    {
      inModel.removeFloatProperty(BO_elevation);
    }
    else if(ApplyToModel)
    {
      // check existance, so we only add it once.
      if (!inModel.hasFloatProperty(BO_elevation))
      {
        inModel.setFloatProperty(BO_elevation, zValues);
      }

    }
  }

  // Apply BO to mesh
  if (ApplyToMesh || optype == "Remove Bathymetry")
  {
    if (bathyPoints == NULL && ApplyToMesh) // get bathy points if we only apply to mesh
    {
      filename = auxGeo.url();
      if(!(!filename.empty() && this->bathyHelper->loadBathymetryFile(filename) && (bathyPoints = this->bathyHelper->bathymetryData(filename))))
      {
        return this->createResult(OPERATION_FAILED);
      }
    }
    smtk::mesh::MeshSets modifiedMeshes;
    // update associated meshes
    if(!internal_bathyToAssociatedMeshes(
       this->bathyHelper, bathyPoints,
       inModel.as<Model>(),
       optype == "Remove Bathymetry",
       aveEleRadius, highZItem ? highZItem->isEnabled() : false, highElevation,
       lowZItem ? lowZItem->isEnabled() : false, lowElevation,
       this->manager()->meshes(), modifiedMeshes, meshItem))
      {
      std::cerr << "ERROR: Failed to apply bathymetry to associated meshes." << std::endl;
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

  } // namespace model
} // namespace smtk


smtkImplementsModelOperator(
  VTKSMTKOPERATORSEXT_EXPORT,
  smtk::model::BathymetryOperator,
  apply_bathymetry,
  "apply bathymetry",
  BathymetryOperator_xml,
  smtk::model::Session);
