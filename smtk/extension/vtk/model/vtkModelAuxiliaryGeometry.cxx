//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/vtk/model/vtkModelAuxiliaryGeometry.h"
#include "smtk/extension/vtk/model/vtkModelAuxiliaryGeometry.txx"

#include "smtk/extension/vtk/model/vtkAuxiliaryGeometryExtension.h"
#include "smtk/extension/vtk/source/vtkModelMultiBlockSource.h"
#include "smtk/model/AuxiliaryGeometry.h"
#include "smtk/model/Model.h"
#include "smtk/model/Resource.h"

#include "vtkImageData.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkNew.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"
#include "vtkPolyDataNormals.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkTransform.h"
#include "vtkTransformFilter.h"
#include "vtkUnstructuredGrid.h"

SMTK_THIRDPARTY_PRE_INCLUDE
#include "boost/filesystem.hpp"
SMTK_THIRDPARTY_POST_INCLUDE

#include <cerrno>
#include <cinttypes>
#include <cstdlib>

using namespace smtk::model;

vtkStandardNewMacro(vtkModelAuxiliaryGeometry);
vtkCxxSetObjectMacro(vtkModelAuxiliaryGeometry, CachedOutput, vtkMultiBlockDataSet);
smtkImplementTracksAllInstances(vtkModelAuxiliaryGeometry);

vtkModelAuxiliaryGeometry::vtkModelAuxiliaryGeometry()
{
  this->SetNumberOfInputPorts(0);
  this->CachedOutput = nullptr;
  this->AuxiliaryEntityID = nullptr;
  this->linkInstance();
  this->AuxGeomHelper = vtkAuxiliaryGeometryExtension::create();
}

vtkModelAuxiliaryGeometry::~vtkModelAuxiliaryGeometry()
{
  this->unlinkInstance();
  this->SetCachedOutput(nullptr);
  this->SetAuxiliaryEntityID(nullptr);
}

void vtkModelAuxiliaryGeometry::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "Auxiliary Entity: " << this->AuxiliaryEntityID << "\n";
  os << indent << "CachedOutput: " << this->CachedOutput << "\n";
}

/// Set the SMTK model to be displayed.
void vtkModelAuxiliaryGeometry::SetModelResource(smtk::model::ResourcePtr model)
{
  if (this->ModelResource == model)
  {
    return;
  }
  this->ModelResource = model;
  this->Modified();
}

/// Get the SMTK model being displayed.
smtk::model::ResourcePtr vtkModelAuxiliaryGeometry::GetModelResource()
{
  return this->ModelResource;
}

/// Indicate that the model has changed and should have its VTK representation updated.
void vtkModelAuxiliaryGeometry::Dirty()
{
  // This both clears the output and marks this filter
  // as modified so that RequestData() will run the next
  // time the representation is updated:
  this->SetCachedOutput(nullptr);
}

// Fill in the WholeExtent and spacing information from the image block
int vtkModelAuxiliaryGeometry::RequestInformation(
  vtkInformation* /*request*/,
  vtkInformationVector** /*inputVector*/,
  vtkInformationVector* outputVector)
{
  if (!this->ModelResource || !this->AuxiliaryEntityID || !this->AuxiliaryEntityID[0])
  {
    // the filter is not properly set up yet
    return 1;
  }

  smtk::common::UUID uid(this->AuxiliaryEntityID);
  smtk::model::AuxiliaryGeometry auxGeoEntity(this->ModelResource, uid);
  if (auxGeoEntity.isValid() && auxGeoEntity.hasURL())
  {
    std::string fileType = vtkAuxiliaryGeometryExtension::getAuxiliaryFileType(auxGeoEntity);
    if (fileType == "vti" || fileType == "dem" || fileType == "tif" || fileType == "tiff")
    {
      // add some temp parameters so that downstream filters could use them.
      int tmpext[6] = { 0, -1, 0, -1, 0, -1 };
      vtkInformation* outInfo = outputVector->GetInformationObject(0);
      outInfo->Set(vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT(), tmpext, 6);
    }
  }

  return 1;
}

/// Generate polydata from an smtk::model with tessellation information.
int vtkModelAuxiliaryGeometry::RequestData(
  vtkInformation* /*request*/,
  vtkInformationVector** /*inInfo*/,
  vtkInformationVector* outputVector)
{
  vtkInformation* outInfo = outputVector->GetInformationObject(0);
  vtkMultiBlockDataSet* output = vtkMultiBlockDataSet::GetData(outputVector, 0);
  if (!output)
  {
    vtkErrorMacro("No output dataset");
    return 0;
  }

  if (!this->ModelResource)
  {
    vtkErrorMacro("No input model");
    return 0;
  }
  if (!this->AuxiliaryEntityID || !this->AuxiliaryEntityID[0])
  {
    vtkErrorMacro("No input AuxiliaryEntityID");
    return 0;
  }

  smtk::common::UUID uid(this->AuxiliaryEntityID);
  smtk::model::AuxiliaryGeometry auxGeoEntity(this->ModelResource, uid);
  if (!auxGeoEntity.isValid() || !auxGeoEntity.hasURL())
  {
    vtkErrorMacro("No valid AuxiliaryEntity");
    return 0;
  }

  // Destroy the cache if the parameters have changed since it was generated.
  if (this->CachedOutput && this->GetMTime() > this->CachedOutput->GetMTime())
  {
    this->SetCachedOutput(nullptr);
  }

  if (!this->CachedOutput)
  {
    // create vtkDataObject by asking our helper to read it.
    vtkSmartPointer<vtkDataObject> auxRep;
    std::vector<double> bbox;
    if (this->AuxGeomHelper->canHandleAuxiliaryGeometry(auxGeoEntity, bbox))
    {
      auxRep = vtkAuxiliaryGeometryExtension::fetchCachedGeometry(auxGeoEntity);
    }
    if (auxRep.GetPointer())
    {
      vtkImageData* imgOut = vtkImageData::SafeDownCast(auxRep);
      // If this is an image data, make the output info include its extent and spacing.
      // This is important for downstream image filters to properly process the multiblock
      if (imgOut)
      {
        int ext[6];
        double spacing[3];
        imgOut->GetExtent(ext);
        imgOut->GetSpacing(spacing);
        outInfo->Set(vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT(), ext, 6);
        outInfo->Set(vtkDataObject::SPACING(), spacing, 3);
      }

      vtkNew<vtkMultiBlockDataSet> result;
      result->SetNumberOfBlocks(1);
      result->SetBlock(0, auxRep);
      this->SetCachedOutput(result.GetPointer());
    }
    else
    {
      vtkErrorMacro("Failed to generate output data object!");
      return 0;
    }
  }

  output->ShallowCopy(this->CachedOutput);

  return 1;
}
