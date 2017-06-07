//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/vtk/operators/ElevateMesh.h"

#include "smtk/extension/vtk/filter/vtkImageSpacingFlip.h"
#include "smtk/extension/vtk/reader/vtkCMBGeometryReader.h"
#include "smtk/extension/vtk/reader/vtkLASReader.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/MeshItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/VoidItem.h"

#include "smtk/mesh/ApplyToMesh.h"
#include "smtk/mesh/CellField.h"
#include "smtk/mesh/Collection.h"
#include "smtk/mesh/Manager.h"
#include "smtk/mesh/MeshSet.h"
#include "smtk/mesh/PointField.h"

#include "smtk/mesh/interpolation/PointCloud.h"
#include "smtk/mesh/interpolation/RadialAverage.h"
#include "smtk/mesh/interpolation/StructuredGrid.h"

#include "smtk/model/AuxiliaryGeometry.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Session.h"

#include "vtkAppendPoints.h"
#include "vtkCompositeDataIterator.h"
#include "vtkDoubleArray.h"
#include "vtkFloatArray.h"
#include "vtkGDALRasterReader.h"
#include "vtkImageData.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkNew.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkUniformGrid.h"
#include "vtkXMLImageDataReader.h"

#include <vtksys/SystemTools.hxx>

#include <array>
#include <cmath>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace
{
// Copied from BathymetryHelper.cxx. This should be refactored to be cleaner.
vtkDataSet* loadAuxiliaryGeometry(const std::string& filename)
{
  if (filename.empty())
  {
    std::cerr << "File name is empty!\n";
    return nullptr;
  }

  vtkDataSet* dataOutput = NULL;
  std::string ext = vtksys::SystemTools::GetFilenameLastExtension(filename);
  std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

  if (ext == ".pts" || ext == ".xyz" || ext == ".bin" || ext == ".vtk" || ext == ".vtp" ||
    ext == ".2dm" || ext == ".3dm" || ext == ".tin" || ext == ".poly" || ext == ".smesh" ||
    ext == ".obj" || ext == ".fac" || ext == ".sol" || ext == ".stl")
  {
    vtkNew<vtkCMBGeometryReader> reader;
    reader->SetFileName(filename.c_str());
    reader->SetPrepNonClosedSurfaceForModelCreation(false);
    reader->SetEnablePostProcessMesh(false);
    reader->Update();

    vtkPolyData* polyOutput = vtkPolyData::New();
    polyOutput->ShallowCopy(reader->GetOutput());
    dataOutput = polyOutput;
  }
  else if (ext == ".dem" || ext == ".tif" || ext == ".tiff")
  {
    vtkNew<vtkGDALRasterReader> reader;
    reader->SetFileName(filename.c_str());

    vtkNew<vtkImageSpacingFlip> flipImage;
    flipImage->SetInputConnection(reader->GetOutputPort());
    flipImage->Update();

    vtkUniformGrid* ugOutput = vtkUniformGrid::New();
    ugOutput->ShallowCopy(flipImage->GetOutput());
    dataOutput = ugOutput;
  }
  else if (ext == ".las")
  {
    vtkNew<vtkLASReader> reader;
    reader->SetFileName(filename.c_str());
    reader->Update();

    vtkMultiBlockDataSet* readout = reader->GetOutput();
    vtkNew<vtkAppendPoints> appendPoints;

    vtkCompositeDataIterator* iter = readout->NewIterator();
    for (iter->InitTraversal(); !iter->IsDoneWithTraversal(); iter->GoToNextItem())
    {
      vtkPolyData* blockPoly = vtkPolyData::SafeDownCast(iter->GetCurrentDataObject());
      if (!blockPoly)
      {
        vtkGenericWarningMacro(<< "This block from LAS reader is not a polydata!\n");
        continue;
      }
      appendPoints->AddInputData(blockPoly);
    }
    iter->Delete();
    appendPoints->Update();

    vtkPolyData* polyOutput = vtkPolyData::New();
    polyOutput->ShallowCopy(appendPoints->GetOutput());
    dataOutput = polyOutput;
  }
  else if (ext == ".vti")
  {
    vtkNew<vtkXMLImageDataReader> reader;
    reader->SetFileName(filename.c_str());
    reader->Update();

    vtkImageData* imgOutput = vtkImageData::New();
    imgOutput->ShallowCopy(reader->GetOutput());
    dataOutput = imgOutput;
  }
  else
  {
    vtkGenericWarningMacro(<< "File type is not supported!\n");
    return nullptr;
  }

  return dataOutput;
}
}

namespace smtk
{
namespace mesh
{

bool ElevateMesh::ableToOperate()
{
  if (!this->ensureSpecification())
  {
    return false;
  }

  smtk::attribute::MeshItem::Ptr meshItem = this->specification()->findMesh("mesh");
  if (!meshItem || meshItem->numberOfValues() == 0)
  {
    return false;
  }

  return true;
}

smtk::model::OperatorResult ElevateMesh::operateInternal()
{
  // Access the external data to use in determining elevation values
  smtk::attribute::ModelEntityItem::Ptr auxGeoItem =
    this->specification()->findModelEntity("auxiliary geometry");

  // Access the mesh to elevate
  smtk::attribute::MeshItem::Ptr meshItem = this->specification()->findMesh("mesh");

  // Access the radius parameter
  smtk::attribute::DoubleItem::Ptr radiusItem = this->findDouble("radius");

  // Access the min elevation parameter
  smtk::attribute::DoubleItem::Ptr minElevationItem = this->findDouble("min elevation");

  // Access the max elevation parameter
  smtk::attribute::DoubleItem::Ptr maxElevationItem = this->findDouble("max elevation");

  // Access the invert scalars parameter
  smtk::attribute::VoidItem::Ptr invertScalarsItem = this->findVoid("invert scalars");

  // Construct a function that inverts and clips its input, according to the input parameters
  std::function<double(double)> postProcess;
  {
    double prefactor = invertScalarsItem->isEnabled() ? -1. : 1.;

    if (minElevationItem->isEnabled() && maxElevationItem->isEnabled())
    {
      double minElevation = minElevationItem->value();
      double maxElevation = maxElevationItem->value();
      postProcess = [=](double input) {
        double output = prefactor * input;
        return (
          output < minElevation ? minElevation : (output > maxElevation ? maxElevation : output));
      };
    }
    else if (minElevationItem->isEnabled())
    {
      double minElevation = minElevationItem->value();
      postProcess = [=](double input) {
        double output = prefactor * input;
        return (output < minElevation ? minElevation : output);
      };
    }
    else if (maxElevationItem->isEnabled())
    {
      double maxElevation = maxElevationItem->value();
      postProcess = [=](double input) {
        double output = prefactor * input;
        return (output > maxElevation ? maxElevation : output);
      };
    }
    else
    {
      postProcess = [=](double input) { return prefactor * input; };
    }
  }

  // Get the auxiliary geometry and check its validity
  smtk::model::AuxiliaryGeometry auxGeo = auxGeoItem->value();
  if (!auxGeo.isValid())
  {
    smtkErrorMacro(this->log(), "Invalid auxiliary geometry.");
    return this->createResult(smtk::model::OPERATION_FAILED);
  }

  // Convert the auxiliary geometry from a file name to a vtkDataset
  vtkDataSet* externalData = loadAuxiliaryGeometry(auxGeo.url());
  if (!externalData)
  {
    smtkErrorMacro(this->log(), "Could not read auxiliary geometry.");
    return this->createResult(smtk::model::OPERATION_FAILED);
  }

  // Construct a function that takes an input point and returns a value
  // according to the average of the locus of points in the external data that,
  // when projected onto the x-y plane, are within a radius of the input
  std::function<double(std::array<double, 3>)> radialAverage;

  {
    if (vtkImageData* imageInput = vtkImageData::SafeDownCast(externalData))
    {
      smtk::mesh::StructuredGrid structuredgrid(imageInput->GetExtent(), imageInput->GetOrigin(),
        imageInput->GetSpacing(),
        [=](int i, int j) { return imageInput->GetScalarComponentAsDouble(i, j, 0, 0); });
      radialAverage = smtk::mesh::RadialAverage(structuredgrid, radiusItem->value());
    }
    else if (vtkUniformGrid* gridInput = vtkUniformGrid::SafeDownCast(externalData))
    {
      smtk::mesh::StructuredGrid structuredgrid(gridInput->GetExtent(), gridInput->GetOrigin(),
        gridInput->GetSpacing(),
        [=](int i, int j) { return gridInput->GetScalarComponentAsDouble(i, j, 0, 0); },
        [=](int i, int j) {
          int pos[3] = { i, j, 0 };
          return gridInput->IsPointVisible(
                   vtkStructuredData::ComputePointIdForExtent(gridInput->GetExtent(), pos)) != 0;
        });
      radialAverage = smtk::mesh::RadialAverage(structuredgrid, radiusItem->value());
    }
    else
    {
      std::function<std::array<double, 3>(std::size_t)> coordinates = [&](std::size_t i) {
        double pt[3];
        externalData->GetPoint(i, pt);
        return std::array<double, 3>({ { pt[0], pt[1], 0. } });
      };

      std::function<double(std::size_t)> data;

      // Check for elevation data. If it exists, use it. Otherwise, just use the z-coordinate of the data
      vtkDataArray* elevationData = externalData->GetPointData()->GetScalars("Elevation");
      if (elevationData)
      {
        data = [=](std::size_t i) { return elevationData->GetTuple1(i); };
      }
      else
      {
        data = [&](std::size_t i) {
          double pt[3];
          externalData->GetPoint(i, pt);
          return pt[2];
        };
      }

      smtk::mesh::PointCloud pointcloud(externalData->GetNumberOfPoints(), coordinates, data);
      radialAverage = smtk::mesh::RadialAverage(
        this->manager()->meshes()->makeCollection(), pointcloud, radiusItem->value());
    }
  }

  // Access the attribute associated with the modified meshes
  smtk::model::OperatorResult result = this->createResult(smtk::model::OPERATION_SUCCEEDED);
  smtk::attribute::MeshItem::Ptr modifiedMeshes = result->findMesh("mesh_modified");
  modifiedMeshes->setNumberOfValues(meshItem->numberOfValues());

  // Access the attribute associated with the changed tessellation
  smtk::attribute::ModelEntityItem::Ptr modifiedEntities = result->findModelEntity("tess_changed");
  modifiedEntities->setNumberOfValues(meshItem->numberOfValues());

  std::function<std::array<double, 3>(std::array<double, 3>)> fn = [&](std::array<double, 3> x) {
    return std::array<double, 3>({ { x[0], x[1], postProcess(radialAverage(x)) } });
  };

  // apply the interpolator to the meshes and populate the result attributes
  for (std::size_t i = 0; i < meshItem->numberOfValues(); i++)
  {
    smtk::mesh::MeshSet mesh = meshItem->value(i);

    smtk::mesh::applyWarp(fn, mesh, true);

    modifiedMeshes->appendValue(mesh);

    smtk::model::EntityRefArray entities;
    bool entitiesAreValid = mesh.modelEntities(entities);
    if (entitiesAreValid && !entities.empty())
    {
      smtk::model::Model model = entities[0].owningModel();
      this->addEntityToResult(result, model, MODIFIED);
      modifiedEntities->appendValue(model);
    }
  }

  return result;
}
}
}

#include "smtk/extension/vtk/operators/ElevateMesh_xml.h"

smtkImplementsModelOperator(VTKSMTKOPERATORSEXT_EXPORT, smtk::mesh::ElevateMesh, elevate_mesh,
  "elevate mesh", ElevateMesh_xml, smtk::model::Session);
