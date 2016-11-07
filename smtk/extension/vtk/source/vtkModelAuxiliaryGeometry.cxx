//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/vtk/source/vtkModelAuxiliaryGeometry.h"

#include "smtk/model/AuxiliaryGeometry.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"
#include "smtk/extension/vtk/filter/vtkImageSpacingFlip.h"

#include "vtkGDALRasterReader.h"
#include "vtkImageData.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkNew.h"
#include "vtkOBJReader.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"
#include "vtkPolyDataNormals.h"
#include "vtkUnstructuredGrid.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkXMLPolyDataReader.h"
#include "vtkXMLUnstructuredGridReader.h"
#include "vtkXMLImageDataReader.h"
#include "vtkXMLMultiBlockDataReader.h"

SMTK_THIRDPARTY_PRE_INCLUDE
#include "boost/filesystem.hpp"
SMTK_THIRDPARTY_POST_INCLUDE

#include <inttypes.h>
#include <stdlib.h>
#include <errno.h>

using namespace smtk::model;

vtkStandardNewMacro(vtkModelAuxiliaryGeometry);
vtkCxxSetObjectMacro(vtkModelAuxiliaryGeometry,CachedOutput,vtkMultiBlockDataSet);

vtkModelAuxiliaryGeometry::vtkModelAuxiliaryGeometry()
{
  this->SetNumberOfInputPorts(0);
  this->CachedOutput = NULL;
  this->AuxiliaryEntityID = NULL;
}

vtkModelAuxiliaryGeometry::~vtkModelAuxiliaryGeometry()
{
  this->SetCachedOutput(NULL);
  this->SetAuxiliaryEntityID(NULL);
}

void vtkModelAuxiliaryGeometry::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "Auxiliary Entity: " << this->AuxiliaryEntityID << "\n";
  os << indent << "CachedOutput: " << this->CachedOutput << "\n";
}

/// Set the SMTK model to be displayed.
void vtkModelAuxiliaryGeometry::SetModelManager(smtk::model::ManagerPtr model)
{
  if (this->ModelMgr == model)
    {
    return;
    }
  this->ModelMgr = model;
  this->Modified();
}

/// Get the SMTK model being displayed.
smtk::model::ManagerPtr vtkModelAuxiliaryGeometry::GetModelManager()
{
  return this->ModelMgr;
}

/// Indicate that the model has changed and should have its VTK representation updated.
void vtkModelAuxiliaryGeometry::Dirty()
{
  // This both clears the output and marks this filter
  // as modified so that RequestData() will run the next
  // time the representation is updated:
  this->SetCachedOutput(NULL);
}

/// Generate a data object representing the entity. It may be polydata, image data, or a multiblock dataset.
vtkSmartPointer<vtkDataObject> vtkModelAuxiliaryGeometry::GenerateRepresentationFromModel(
  const smtk::model::AuxiliaryGeometry& aux,
  bool genNormals)
{
  std::string url;
  if (aux.isValid() && !(url = aux.url()).empty())
    {
    return this->GenerateRepresentationFromURL(aux, genNormals);
    }
  return vtkSmartPointer<vtkDataObject>();
}

template<typename T, typename U>
vtkSmartPointer<T> ReadData(const smtk::model::AuxiliaryGeometry& auxGeom)
{
  vtkNew<U> rdr;
  rdr->SetFileName(auxGeom.url().c_str());
  rdr->Update();
  vtkSmartPointer<T> data = vtkSmartPointer<T>::New();
  data->ShallowCopy(rdr->GetOutput());
  return data;
}
std::string vtkModelAuxiliaryGeometry::GetAuxiliaryFileType(
  const smtk::model::AuxiliaryGeometry& auxGeom)
{
  std::string fileType;
  if (auxGeom.hasStringProperty("type"))
    {
    const StringList& prop(auxGeom.stringProperty("type"));
    if (!prop.empty())
      {
      fileType = prop[0];
      }
    }
  if (fileType.empty())
    {
    fileType = vtkModelAuxiliaryGeometry::InferFileTypeFromFileName(auxGeom.url());
    }
  return fileType;
}
/// Create a reader and copy its output into a new data object to serve as the representation for auxiliary geometry.
vtkSmartPointer<vtkDataObject> vtkModelAuxiliaryGeometry::GenerateRepresentationFromURL(
  const smtk::model::AuxiliaryGeometry& auxGeom,
  bool genNormals)
{
  (void)genNormals;
  std::string fileType = vtkModelAuxiliaryGeometry::GetAuxiliaryFileType(auxGeom);
  if (fileType == "vtp") { return ReadData<vtkPolyData, vtkXMLPolyDataReader>(auxGeom); }
  else if (fileType == "vtu") { return ReadData<vtkUnstructuredGrid, vtkXMLUnstructuredGridReader>(auxGeom); }
  else if (fileType == "vti") { return ReadData<vtkImageData, vtkXMLImageDataReader>(auxGeom); }
  else if (fileType == "vtm") { return ReadData<vtkMultiBlockDataSet, vtkXMLMultiBlockDataReader>(auxGeom); }
  else if (fileType == "obj") { return ReadData<vtkPolyData, vtkOBJReader>(auxGeom); }
  else if (fileType == "dem" || fileType == "tif" || fileType == "tiff")
  {
    vtkSmartPointer<vtkImageData> outImage = ReadData<vtkImageData, vtkGDALRasterReader>(auxGeom);
    if(outImage.GetPointer())
    {
      vtkNew<vtkImageSpacingFlip> flipImage;
      flipImage->SetInputData(outImage);
      flipImage->Update();
      vtkSmartPointer<vtkImageData> data = vtkSmartPointer<vtkImageData>::New();
      data->ShallowCopy(flipImage->GetOutput());
      return data;
    }   
  }

  return vtkSmartPointer<vtkDataObject>();
}

std::string vtkModelAuxiliaryGeometry::InferFileTypeFromFileName(const std::string& fname)
{
  ::boost::filesystem::path fp(fname);
  return fp.extension().string().substr(1);
}

//----------------------------------------------------------------------------
// Fill in the WholeExtent and spacing information from the image block
int vtkModelAuxiliaryGeometry::RequestInformation (
  vtkInformation * vtkNotUsed(request),
  vtkInformationVector **vtkNotUsed(inputVector),
  vtkInformationVector *outputVector)
{
  if (!this->ModelMgr || !this->AuxiliaryEntityID || !this->AuxiliaryEntityID[0])
  {
    // the filter is not properly set up yet
    return 1;
  }

  smtk::common::UUID uid(this->AuxiliaryEntityID);
  smtk::model::AuxiliaryGeometry auxGeoEntity(this->ModelMgr, uid);
  if(auxGeoEntity.isValid() && auxGeoEntity.hasUrl())
  {
    std::string fileType = vtkModelAuxiliaryGeometry::GetAuxiliaryFileType(
                           auxGeoEntity);
    if (fileType == "vti" || fileType == "dem"
        || fileType == "tif" || fileType == "tiff")
    {
      // add some temp parameters so that downstream filters could use them.
      int tmpext[6] = {0, -1, 0, -1, 0, -1};
      vtkInformation *outInfo = outputVector->GetInformationObject(0);
      outInfo->Set(vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT(), tmpext, 6);
    }
  }

  return 1;
}

/// Generate polydata from an smtk::model with tessellation information.
int vtkModelAuxiliaryGeometry::RequestData(
  vtkInformation* vtkNotUsed(request),
  vtkInformationVector** vtkNotUsed(inInfo),
  vtkInformationVector* outputVector)
{
  vtkInformation *outInfo = outputVector->GetInformationObject(0);
  vtkMultiBlockDataSet* output = vtkMultiBlockDataSet::GetData(outputVector, 0);
  if (!output)
  {
    vtkErrorMacro("No output dataset");
    return 0;
  }

  if (!this->ModelMgr)
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
  smtk::model::AuxiliaryGeometry auxGeoEntity(this->ModelMgr, uid);
  if(!auxGeoEntity.isValid() || !auxGeoEntity.hasUrl())
  {
    vtkErrorMacro("No valid AuxiliaryEntity");
    return 0;        
  }

  // Destroy the cache if the parameters have changed since it was generated.
  if (this->CachedOutput && this->GetMTime() > this->CachedOutput->GetMTime())
  {
    this->SetCachedOutput(NULL);
  }

  if (!this->CachedOutput)
  { 
    // See if the model has any instructions about
    // whether to generate surface normals.
    bool modelRequiresNormals = false;
    if ( auxGeoEntity.owningModel().hasIntegerProperty("generate normals"))
    {
      const IntegerList& prop(auxGeoEntity.owningModel().integerProperty("generate normals"));
      if (!prop.empty() && prop[0])
      {
        modelRequiresNormals = true;
      }
    }

    // create vtkDataObject by reading the Url property of AuxiliaryGeometry.
    vtkSmartPointer<vtkDataObject> auxRep =
      this->GenerateRepresentationFromModel(auxGeoEntity, modelRequiresNormals);
    if (auxRep.GetPointer())
    {
    vtkImageData* imgOut = vtkImageData::SafeDownCast(auxRep);
    // If this is an image data, make the output info include its extent and spacing.
    // This is important for downstream image filters to properly process the multiblock
    if(imgOut)
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
