//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkCMBGeometry2DReader.h"

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

#include "vtkXMLPolyDataWriter.h"
#include "vtkTableWriter.h"
#include "smtk/extension/vtk/meshing/vtkDiscoverRegions.h"
#include "smtk/extension/vtk/meshing/vtkRegionsToLoops.h"
#include "smtk/extension/vtk/meshing/vtkSplitPlanarLines.h"

#include <vtksys/SystemTools.hxx>

#include <sys/types.h>
#include <sys/stat.h>

namespace smtk {
  namespace vtk {

vtkStandardNewMacro(vtkCMBGeometry2DReader);

vtkCMBGeometry2DReader::vtkCMBGeometry2DReader()
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

vtkCMBGeometry2DReader::~vtkCMBGeometry2DReader()
{
  this->SetFileName(0);
  this->SetBoundaryFile(NULL);
}

void vtkCMBGeometry2DReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "File Name: "
     << (this->FileName ? this->FileName : "(none)") << "\n";
  os << indent << "BoundaryStyle: " << this->BoundaryStyle << "\n";
}

void vtkCMBGeometry2DReader::SetRelativeMarginString(const char* text)
{
  double vals[4];
  if (this->GetMarginFromString(text, vals))
    {
    this->SetRelativeMargin(vals);
    }
}

void vtkCMBGeometry2DReader::SetAbsoluteMarginString(const char* text)
{
  double vals[4];
  if (this->GetMarginFromString(text, vals))
    {
    this->SetAbsoluteMargin(vals);
    }
}

void vtkCMBGeometry2DReader::SetAbsoluteBoundsString(const char* text)
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

int vtkCMBGeometry2DReader::GetMarginFromString(const char* text, double vals[4])
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
      vtkErrorMacro(
        << "Unable to parse string into 1 or more comma-separated values: \""
        << text << "\"");
      }
    }
  return numVals;
}

int vtkCMBGeometry2DReader::RequestInformation(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **vtkNotUsed(inputVector),
  vtkInformationVector *vtkNotUsed(outputVector))
{
  if (!this->FileName)
    {
    vtkErrorMacro("FileName has to be specified!");
    return 0;
    }

  return 1;
}

int vtkCMBGeometry2DReader::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **vtkNotUsed(inputVector),
  vtkInformationVector *outputVector)
{
  vtkInformation *outInfo = outputVector->GetInformationObject(0);
  vtkPolyData *output = vtkPolyData::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));
  std::string fileNameStr =this->FileName;
  vtksys::SystemTools::ConvertToUnixSlashes(fileNameStr);
  std::string fullName = vtksys::SystemTools::CollapseFullPath(fileNameStr.c_str());
  struct stat fs;
  if (stat(fileNameStr.c_str(), &fs) != 0)
    {
    vtkErrorMacro(<< "Unable to open file: "<< fileNameStr);
    this->SetErrorCode( vtkErrorCode::CannotOpenFileError );
    return 0;
    }
  if (fileNameStr.find(".shp") != std::string::npos)
    {
    vtkNew<vtkGDALVectorReader> rdr;
    vtkNew<vtkCleanPolyData> cln;
    vtkNew<vtkAppendPolyData> app;
    vtkNew<vtkSplitPlanarLines> slf;
    vtkNew<vtkDiscoverRegions> drg;
    vtkNew<vtkRegionsToLoops> rtl;
    vtkNew<vtkPolyDataNormals> pdn;
    vtkNew<vtkXMLPolyDataWriter> wri;
    vtkNew<vtkTableWriter> twr;

    rdr->SetFileName(fileNameStr.c_str());
    //rdr->AddFeatureIdsOn();
    rdr->SetAppendFeatures(1);
    rdr->Update();

    cln->SetInputConnection(rdr->GetOutputPort());
    cln->Update();
    vtkMultiBlockDataSet* mbds =
      vtkMultiBlockDataSet::SafeDownCast(
        cln->GetOutputDataObject(0));
    // Now we do some manual processing:
    // (a) create a polydata for a bounding box
    // (b) add a pedigree ID indicating the original ID of the geometry.
    vtkPolyData* rdp = mbds ?
      vtkPolyData::SafeDownCast(mbds->GetBlock(0)) : NULL;

    // Remove the "label" attribute if it exists... string arrays
    // can cause problems.
    vtkNew<vtkPassArrays> psa;
    psa->SetInputDataObject(rdp);
    psa->RemoveArraysOn();
    psa->UseFieldTypesOn();
    psa->AddFieldType(vtkDataObject::CELL);
    psa->AddCellDataArray("label");
    psa->Update();

    /*
    wri->SetInputConnection(psa->GetOutputPort());
    wri->SetDataModeToAscii();
    wri->SetFileName("/tmp/gdal.vtp");
    wri->Write();
    */

    if (this->BoundaryStyle != NONE)
      {
      // ctp will hold a clipping polygon either created manually
      // as a bounding rectangle or read from a separate shapefile.
      vtkNew<vtkPolyData> ctp;
      vtkNew<vtkPoints> ctpPts;
      vtkNew<vtkCellArray> ctpLin;
      // Add pedigree IDs to the data we are about to append
      vtkNew<vtkIdTypeArray> cid;
      cid->SetName("_vtkPedigreeIds");
      if (this->BoundaryStyle == IMPORTED_POLYGON)
        {
        if (!this->BoundaryFile || !this->BoundaryFile[0])
          {
          vtkErrorMacro(<< "Must have a valid boundary filename to import.\n");
          return 0;
          }
        vtkNew<vtkGDALVectorReader> rbd;
        vtkNew<vtkCleanPolyData> cpd;
        rbd->SetFileName(this->BoundaryFile);
        //rbd->AddFeatureIdsOn();
        rbd->SetAppendFeatures(1);
        rbd->Update();

        cpd->SetInputConnection(rbd->GetOutputPort());
        cpd->Update();
        mbds = vtkMultiBlockDataSet::SafeDownCast(
            cpd->GetOutputDataObject(0));
        // Now we do some manual processing:
        // (a) create a polydata for a bounding box
        // (b) add a pedigree ID indicating the original ID of the geometry.
        vtkPolyData* rdp2 = mbds ?
          vtkPolyData::SafeDownCast(mbds->GetBlock(0)) : NULL;

        // Remove the "label" attribute if it exists... string arrays
        // can cause problems.
        vtkNew<vtkPassArrays> ps2;
        ps2->SetInputDataObject(rdp2);
        ps2->RemoveArraysOn();
        ps2->UseFieldTypesOn();
        ps2->AddFieldType(vtkDataObject::CELL);
        ps2->AddCellDataArray("label");
        ps2->Update();

        ctp->ShallowCopy(ps2->GetOutputDataObject(0));
        cid->SetNumberOfTuples(ctp->GetNumberOfCells());
        cid->FillComponent(0, -1.);

        wri->SetInputConnection(ps2->GetOutputPort());
        wri->SetDataModeToAscii();
        wri->SetFileName("/tmp/bdy.vtp");
        wri->Write();
        /*
        */
        }
      else // generate a rectangle, somehow
        {
        cid->SetNumberOfTuples(1);
        cid->SetValue(0,-1);
        double bds[6];
        double margin[4];
        if (this->BoundaryStyle == RELATIVE_MARGIN)
          {
          this->GetRelativeMargin(margin);
          // convert from percent to world coordinates:
          double dataLength = rdp->GetLength();
          for (int i = 0; i < 4; ++i)
            {
            margin[i] *= dataLength / 100.;
            }
          }
        else if (this->BoundaryStyle == ABSOLUTE_MARGIN)
          {
          this->GetAbsoluteMargin(margin);
          }
        else // (this->BoundaryStyle == ABSOLUTE_BOUNDS)
          {
          this->GetAbsoluteBounds(margin);
          for (int i = 0; i < 4; ++i)
            {
            bds[i] = margin[i];
            margin[i] = 0.;
            }
          }
        rdp->GetBounds(bds);
        ctp->SetPoints(ctpPts.GetPointer());
        ctp->SetLines(ctpLin.GetPointer());
        ctpPts->SetNumberOfPoints(4);
        ctpPts->SetPoint(0, bds[0] - margin[0], bds[2] - margin[2], bds[4]);
        ctpPts->SetPoint(1, bds[1] + margin[1], bds[2] - margin[2], bds[4]);
        ctpPts->SetPoint(2, bds[1] + margin[1], bds[3] + margin[3], bds[4]);
        ctpPts->SetPoint(3, bds[0] - margin[0], bds[3] + margin[3], bds[4]);
        vtkIdType rectConn[] = { 0, 1, 2, 3, 0 };
        ctp->InsertNextCell(VTK_POLY_LINE, sizeof(rectConn)/sizeof(rectConn[0]), rectConn);
        }
      ctp->GetCellData()->SetPedigreeIds(cid.GetPointer());
      if (!rdp->GetCellData()->GetPedigreeIds())
        {
        vtkNew<vtkIdTypeArray> rid;
        rid->SetName("_vtkPedigreeIds");
        vtkIdType ncell = rdp->GetNumberOfCells();
        rid->SetNumberOfTuples(ncell);
        for (vtkIdType i = 0; i < ncell; ++i)
          {
          rid->SetValue(i, i);
          }
        rdp->GetCellData()->SetPedigreeIds(rid.GetPointer());
        }

      app->AddInputDataObject(rdp);
      app->AddInputDataObject(ctp.GetPointer());

      /*
      wri->SetInputConnection(app->GetOutputPort());
      wri->SetDataModeToAscii();
      wri->SetFileName("/tmp/append.vtp");
      wri->Write();
      */

      slf->SetInputConnection(app->GetOutputPort());
      }
    else
      {
      slf->SetInputConnection(psa->GetOutputPort());
      }
    slf->Update();

    /*
    wri->SetInputConnection(slf->GetOutputPort());
    wri->SetDataModeToAscii();
    wri->SetFileName("/tmp/splitLines.vtp");
    wri->Write();
    */

    drg->SetInputConnection(slf->GetOutputPort());
    drg->GenerateRegionInteriorPointsOn();
    drg->Update();

    /*
    wri->SetInputConnection(drg->GetOutputPort());
    wri->SetDataModeToAscii();
    wri->SetFileName("/tmp/discoveredRegions.vtp");
    wri->Write();

    wri->SetInputConnection(drg->GetOutputPort(2));
    wri->SetDataModeToAscii();
    wri->SetFileName("/tmp/regionPoints.vtp");
    wri->Write();
    */

    rtl->SetInputConnection(drg->GetOutputPort());
    rtl->SetInputConnection(1, drg->GetOutputPort(1));
    rtl->SetInputConnection(2, drg->GetOutputPort(2));
    rtl->Update();

    /*
    wri->SetInputConnection(rtl->GetOutputPort());
    wri->SetDataModeToAscii();
    wri->SetFileName("/tmp/regionsToLoops.vtp");
    wri->Write();

    vtkTable* containment = vtkTable::SafeDownCast(drg->GetOutputDataObject(1));
    twr->SetInputConnection(drg->GetOutputPort(1));
    twr->SetFileTypeToASCII();
    twr->SetFileName("/tmp/splitLinesContainment.vtk");
    twr->Write();
    */

    pdn->SetInputConnection(rtl->GetOutputPort());
    pdn->NonManifoldTraversalOn();
    pdn->ConsistencyOn();
    pdn->SplittingOff();
    pdn->ComputePointNormalsOff();
    pdn->ComputeCellNormalsOn();
    pdn->Update();

    /*
    wri->SetInputConnection(pdn->GetOutputPort());
    wri->SetDataModeToAscii();
    wri->SetFileName("/tmp/pdnormals.vtp");
    wri->Write();
    */

    output->ShallowCopy(pdn->GetOutput());
    }
  else // unrecognized file type
    {
    vtkErrorMacro(<< "Unrecognized file type: "<< fileNameStr);
    this->SetErrorCode( vtkErrorCode::UnrecognizedFileTypeError );
    return 0;
    }

  // Append File name to output
  vtkNew<vtkStringArray> filenameFD;
  filenameFD->SetName("FileName");
  filenameFD->InsertNextValue(fullName);
  output->GetFieldData()->AddArray( filenameFD.GetPointer() );
  return 1;
}
  } // namespace vtk
} // namespace smtk
