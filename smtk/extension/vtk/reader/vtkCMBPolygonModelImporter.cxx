//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkCMBPolygonModelImporter.h"

#include "vtkAppendPolyData.h"
#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkCleanPolyData.h"
#include "vtkErrorCode.h"
#include "vtkGDALVectorReader.h"
#include "vtkIdTypeArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkNew.h"
#include "vtkObjectFactory.h"
#include "vtkPassArrays.h"
#include "vtkPolyData.h"
#include "vtkPolyDataNormals.h"
#include "vtkStringArray.h"
#include "vtkTable.h"

#include "smtk/extension/vtk/meshing/vtkDiscoverRegions.h"
#include "smtk/extension/vtk/meshing/vtkRegionsToLoops.h"
#include "smtk/extension/vtk/meshing/vtkSplitPlanarLines.h"
#include "vtkTableWriter.h"
#include "vtkXMLPolyDataWriter.h"

#include <vtksys/SystemTools.hxx>

#include <sys/stat.h>
#include <sys/types.h>

vtkStandardNewMacro(vtkCMBPolygonModelImporter);

vtkCMBPolygonModelImporter::vtkCMBPolygonModelImporter()
{
  this->FileName = NULL;
  this->SetNumberOfInputPorts(0);
  for (int i = 0; i < 4; ++i)
  {
    this->RelativeMargin[i] = 5.; // in percent
    this->AbsoluteMargin[i] = 1.;
    this->AbsoluteBounds[i] = i % 2 ? -1. : +1.;
  }
  this->BoundaryFile = NULL;
  this->BoundaryStyle = NONE;
}

vtkCMBPolygonModelImporter::~vtkCMBPolygonModelImporter()
{
  this->SetFileName(0);
  this->SetBoundaryFile(NULL);
}

void vtkCMBPolygonModelImporter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "File Name: " << (this->FileName ? this->FileName : "(none)") << "\n";
  os << indent << "BoundaryStyle: " << this->BoundaryStyle << "\n";
}

void vtkCMBPolygonModelImporter::SetRelativeMarginString(const char* text)
{
  double vals[4];
  if (this->GetMarginFromString(text, vals))
  {
    this->SetRelativeMargin(vals);
  }
}

void vtkCMBPolygonModelImporter::SetAbsoluteMarginString(const char* text)
{
  double vals[4];
  if (this->GetMarginFromString(text, vals))
  {
    this->SetAbsoluteMargin(vals);
  }
}

void vtkCMBPolygonModelImporter::SetAbsoluteBoundsString(const char* text)
{
  double vals[4];
  int numVals = this->GetMarginFromString(text, vals);
  if (numVals != 4)
  { // Assume we should reset the bounds to be invalid
    vals[0] = vals[2] = +1.;
    vals[1] = vals[3] = -1.;
  }
  this->SetAbsoluteBounds(vals);
}

int vtkCMBPolygonModelImporter::GetMarginFromString(const char* text, double vals[4])
{
  int numVals = sscanf(text, "%lf, %lf, %lf, %lf", vals, vals + 1, vals + 2, vals + 3);
  switch (numVals)
  {
    case 1:
      vals[1] = vals[2] = vals[3] = vals[0];
      break;
    case 2:
      vals[3] = vals[2] = vals[1];
      vals[1] = vals[0];
      break;
    case 3:
      vals[3] = vals[2];
      break;
    case 4:
      break;
    default:
    {
      vtkErrorMacro(<< "Unable to parse string into 1 or more comma-separated values: \"" << text
                    << "\"");
    }
  }
  return numVals;
}

int vtkCMBPolygonModelImporter::RequestInformation(vtkInformation* vtkNotUsed(request),
  vtkInformationVector** vtkNotUsed(inputVector), vtkInformationVector* vtkNotUsed(outputVector))
{
  if (!this->FileName)
  {
    vtkErrorMacro("FileName has to be specified!");
    return 0;
  }

  return 1;
}

int vtkCMBPolygonModelImporter::RequestData(vtkInformation* vtkNotUsed(request),
  vtkInformationVector** vtkNotUsed(inputVector), vtkInformationVector* outputVector)
{
  vtkInformation* outInfo = outputVector->GetInformationObject(0);
  vtkPolyData* output = vtkPolyData::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));
  std::string fileNameStr = this->FileName;
  vtksys::SystemTools::ConvertToUnixSlashes(fileNameStr);
  std::string fullName = vtksys::SystemTools::CollapseFullPath(fileNameStr.c_str());
  struct stat fs;
  if (stat(fileNameStr.c_str(), &fs) != 0)
  {
    vtkErrorMacro(<< "Unable to open file: " << fileNameStr);
    this->SetErrorCode(vtkErrorCode::CannotOpenFileError);
    return 0;
  }
  if (fileNameStr.find(".shp") != std::string::npos)
  {
    vtkNew<vtkGDALVectorReader> rdr;

    vtkNew<vtkCleanPolyData> cln;
    /*    vtkNew<vtkAppendPolyData> app;
    vtkNew<vtkSplitPlanarLines> slf;
    vtkNew<vtkDiscoverRegions> drg;
    vtkNew<vtkRegionsToLoops> rtl;
    vtkNew<vtkPolyDataNormals> pdn;
    vtkNew<vtkXMLPolyDataWriter> wri;
    vtkNew<vtkTableWriter> twr;
*/
    rdr->SetFileName(fileNameStr.c_str());
    rdr->AddFeatureIdsOn();
    rdr->SetAppendFeatures(1);
    rdr->Update();

    cln->SetInputConnection(rdr->GetOutputPort());
    //    cln->PointMergingOn();
    cln->Update();
    vtkMultiBlockDataSet* mbds = vtkMultiBlockDataSet::SafeDownCast(cln->GetOutputDataObject(0));
    // Now we do some manual processing:
    // (a) create a polydata for a bounding box
    // (b) add a pedigree ID indicating the original ID of the geometry.
    vtkPolyData* rdp = mbds ? vtkPolyData::SafeDownCast(mbds->GetBlock(0)) : NULL;
    /*
    // Remove the "label" attribute if it exists... string arrays
    // can cause problems.
    vtkNew<vtkPassArrays> psa;
    psa->SetInputDataObject(rdp);
    psa->RemoveArraysOn();
    psa->UseFieldTypesOn();
    psa->AddFieldType(vtkDataObject::CELL);
    psa->AddCellDataArray("label");
    psa->Update();
*/
    if (rdp)
      output->ShallowCopy(rdp);
  }
  else // unrecognized file type
  {
    vtkErrorMacro(<< "Unrecognized file type: " << fileNameStr);
    this->SetErrorCode(vtkErrorCode::UnrecognizedFileTypeError);
    return 0;
  }

  // Append File name to output
  vtkNew<vtkStringArray> filenameFD;
  filenameFD->SetName("FileName");
  filenameFD->InsertNextValue(fullName);
  output->GetFieldData()->AddArray(filenameFD.GetPointer());
  return 1;
}
