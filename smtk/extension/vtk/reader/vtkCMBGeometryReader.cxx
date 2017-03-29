//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkCMBGeometryReader.h"

#include "vtkAppendPolyData.h"
#include "vtkCellData.h"
#include "vtkCleanPolyData.h"
#include "vtkDataSetRegionSurfaceFilter.h"
#include "vtkErrorCode.h"
#include "vtkFeatureEdges.h"
#include "vtkFieldData.h"
#include "vtkGenericDataObjectReader.h"
#include "vtkIdTypeArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkNew.h"
#include "vtkOBJReader.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkPolyDataNormals.h"
#include "vtkPolyDataReader.h"
#include "vtkSmartPointer.h"
#include "vtkStringArray.h"
#include "vtkUnstructuredGrid.h"
#include "vtkXMLPolyDataReader.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <vtksys/SystemTools.hxx>

#include "smtk/Options.h" // for SMTK_ENABLE_REMUS_SUPPORT
#ifdef SMTK_ENABLE_REMUS_SUPPORT
  #include "smtk/extension/vtk/meshing/vtkDiscoverRegions.h"
  #include "smtk/extension/vtk/meshing/vtkPolylineTriangulator.h"
  #include "smtk/extension/vtk/reader/vtkPolyFileReader.h"
#endif

#include "smtk/extension/vtk/reader/vtkCMBMeshReader.h"
#include "smtk/extension/vtk/reader/vtkCMBReaderHelperFunctions.h"
#include "smtk/extension/vtk/reader/vtkCMBSTLReader.h"
#include "smtk/extension/vtk/reader/vtkCUBITReader.h"
#include "smtk/extension/vtk/reader/vtkExtractRegionEdges.h"
#include "smtk/extension/vtk/reader/vtkGMSSolidReader.h"
#include "smtk/extension/vtk/reader/vtkGMSTINReader.h"
#include "smtk/extension/vtk/reader/vtkLIDARReader.h"

vtkStandardNewMacro(vtkCMBGeometryReader);

//-----------------------------------------------------------------------------
vtkCMBGeometryReader::vtkCMBGeometryReader()
{
  this->FileName = NULL;
  this->SetNumberOfInputPorts(0);
  this->PrepNonClosedSurfaceForModelCreation = false;
  this->HasBoundaryEdges = false;
  this->RegionIdentifiersModified = false;
  this->EnablePostProcessMesh = true;
}

//-----------------------------------------------------------------------------
vtkCMBGeometryReader::~vtkCMBGeometryReader()
{
  this->SetFileName(0);
}

//-----------------------------------------------------------------------------
int vtkCMBGeometryReader::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **vtkNotUsed(inputVector),
  vtkInformationVector *outputVector)
{

  /*
   * Presumptions CMBGeometryReader makes about input data
   *
   * VTK 3D Files:
   *   - Expects cell based int array called Material
   *   - Expects cell based int array called Region ,
   *      which should have same values as material array.
   *      This dependency is from vtkMasterPolyDataNormals, and
   *      MergeDuplicateCells
   *
   * 3DM Files:
   *    - Expects the reader to always produce an int array called Region
   *
   */
  // get the info object
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // get the ouptut
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

  this->HasBoundaryEdges = false;
  this->RegionIdentifiersModified = false;

  if (fileNameStr.find(".vtk") != std::string::npos)
    {
    vtkNew<vtkGenericDataObjectReader> reader;
    reader->SetFileName( fileNameStr.c_str() );
    reader->Update();
    vtkDataSet *dataset = vtkDataSet::SafeDownCast( reader->GetOutput() );
    if (!dataset)
      {
      vtkErrorMacro("Data type must be a subclass of vtkDataSet: " << fileNameStr);
      return 0;
      }

    bool is3DVolumeMesh = false;
    // If the data is an unstructured grid we need to see what types of elements
    // it has to determine if it represents a volume mesh
    vtkUnstructuredGrid *uGrid = vtkUnstructuredGrid::SafeDownCast( dataset );
    if (uGrid != NULL)
      {
      vtkNew<vtkCellTypes> types;
      uGrid->GetCellTypes(types.GetPointer());
      if (types->IsType(VTK_TETRA) || types->IsType(VTK_VOXEL) ||
          types->IsType(VTK_HEXAHEDRON) || types->IsType(VTK_WEDGE) ||
          types->IsType(VTK_PYRAMID) || types->IsType(VTK_PENTAGONAL_PRISM) ||
          types->IsType(VTK_HEXAGONAL_PRISM))
        {
        is3DVolumeMesh = true;
        }
      }
    if (this->EnablePostProcessMesh)
      {
      std::string regionArrayName;
      if (dataset->GetCellData()->GetArray("Material"))
        {
        regionArrayName = "Material";
        }
      this->PostProcessMesh(dataset, is3DVolumeMesh, false,
        regionArrayName.c_str(), output);
      }
    else if (!vtkPolyData::SafeDownCast(dataset))
      {
      // must convert it to polydata, so just use the genreic DataSetSurface filter
      vtkNew<vtkDataSetSurfaceFilter> surfaceFilter;
      surfaceFilter->SetInputConnection( reader->GetOutputPort() );
      surfaceFilter->Update();
      output->ShallowCopy( surfaceFilter->GetOutput() );
      }
    }
  else if (fileNameStr.find(".vtp") != std::string::npos)
    {
    vtkNew<vtkXMLPolyDataReader> reader;
    reader->SetFileName( fileNameStr.c_str() );
    reader->Update();
    output->ShallowCopy( reader->GetOutput() );
    }
  else if (fileNameStr.find(".2dm") != std::string::npos ||
    fileNameStr.find(".3dm") != std::string::npos)
    {
    vtkNew<vtkCMBMeshReader> reader;
    reader->SetCreateMeshElementIdArray(true);
    reader->SetRenameMaterialAsRegion(true);
    reader->SetCreateMeshMaterialIdArray(true);;
    reader->SetFileName( fileNameStr.c_str() );
    reader->Update();

    // process mesh if its 2D or 3D (ignore a 1D mesh)
    if (reader->GetMeshDimension() == vtkCMBMeshReader::MESH1D)
      {
      vtkErrorMacro(<<"The file contains a 1D mesh and will be ignored.");
      this->SetErrorCode(vtkErrorCode::UserError);
      return 0;
      }

    if (this->EnablePostProcessMesh)
      {
      this->PostProcessMesh(reader->GetOutput(),
        reader->GetMeshDimension() == vtkCMBMeshReader::MESH3D, true,
        ReaderHelperFunctions::GetShellTagName(), output);
      }
    else
      {
      // must convert it to polydata, so just use the generic DataSetSurface filter
      vtkNew<vtkDataSetSurfaceFilter> surfaceFilter;
      surfaceFilter->SetInputConnection( reader->GetOutputPort() );
      surfaceFilter->Update();
      output->ShallowCopy( surfaceFilter->GetOutput() );
      }
    }
  else if (fileNameStr.find(".bin") != std::string::npos ||
    fileNameStr.find(".pts") != std::string::npos)
    {
    // binary or ASCII automatically determined
    vtkNew<vtkLIDARReader> reader;
    reader->SetFileName( fileNameStr.c_str() );
    reader->Update();
    output->ShallowCopy( reader->GetOutput() );
    }
  else if (fileNameStr.find(".tin") != std::string::npos)
    {
    vtkNew<vtkGMSTINReader> reader;
    reader->SetFileName( fileNameStr.c_str() );
    reader->Update();
    output->ShallowCopy( reader->GetOutput()->GetBlock(0));
    }
#ifdef SMTK_ENABLE_REMUS_SUPPORT
  else if (
    (fileNameStr.find(".poly") != std::string::npos) ||
    (fileNameStr.find(".smesh") != std::string::npos)
    )
    {
    //modelFaceArray->SetName( ReaderHelperFunctions::GetModelFaceTagName() );

    vtkNew<vtkPolyFileReader> rdr;
    vtkNew<vtkPolylineTriangulator> tmp;

    // Read in the polygonal data as polyline strips
    rdr->SetFileName(fileNameStr.c_str());
    rdr->SetSimpleMeshFormat(-1);
    rdr->FacetMarksAsCellDataOff();
    rdr->Update();

    vtkPolyData* loops = rdr->GetOutput();
    vtkDataArray* adim = loops ? loops->GetFieldData()->GetArray("Dimension") : NULL;
    int dim = adim ? adim->GetTuple1(0) : 3;

    if (dim == 2)
      {
      output->ShallowCopy(rdr->GetOutput());
      }
    else
      {
      // Now triangulate the polyline strips using Triangle
      tmp->SetInputConnection(0, rdr->GetOutputPort(0)); // polygonal loops
      tmp->SetInputConnection(1, rdr->GetOutputPort(1)); // facet hole markers
      tmp->SetModelFaceArrayName(VTK_POLYFILE_MODEL_FACE_ID);
      tmp->Update();

      // Do our best to consistently orient triangles
      vtkNew<vtkPolyDataNormals> nrm;
      nrm->SetInputConnection(tmp->GetOutputPort(0));
      nrm->SetConsistency(1);
      nrm->SetNonManifoldTraversal(0);
      nrm->SetSplitting(0);
      nrm->ComputePointNormalsOff();
      nrm->Update();

      // Discover regions and assign region IDs
      vtkNew<vtkDiscoverRegions> drg;
      drg->SetModelFaceArrayName(VTK_POLYFILE_MODEL_FACE_ID);
      drg->ReportRegionsByModelFaceOn();
      drg->SetFaceGroupArrayName(VTK_POLYFILE_FACE_GROUP);
      drg->SetRegionGroupArrayName(VTK_POLYFILE_REGION_GROUP);
      drg->SetInputConnection(0, nrm->GetOutputPort(0));
      drg->SetInputConnection(1, rdr->GetOutputPort(2));
      drg->SetInputConnection(2, rdr->GetOutputPort(3));
      drg->Update();

      // Compute point-normals and allow splitting now that
      // regions have been discovered. This keeps ModelBuilder
      // from complaining about missing normals.
      vtkNew<vtkPolyDataNormals> pnm;
      pnm->SetInputConnection(drg->GetOutputPort(0));
      // These settings horked from vtkMasterPolyDataNormals, which
      // we can't use since it's in CMBModel, not VTKExtensions:
      pnm->ComputePointNormalsOff();
      pnm->ComputeCellNormalsOn();
      pnm->SplittingOff();
      pnm->AutoOrientNormalsOn();
      pnm->Update();

      output->ShallowCopy(pnm->GetOutput());
      }
    }
#endif
  else if (fileNameStr.find(".obj") != std::string::npos)
    {
    vtkNew<vtkOBJReader> reader;
    reader->SetFileName( fileNameStr.c_str() );
    reader->Update();
    if (reader->GetOutput()->GetPointData()->GetTCoords())
      {
      reader->GetOutput()->GetPointData()->GetTCoords()->SetName("TCoords");
      }
    if (reader->GetOutput()->GetPointData()->GetNormals())
      {
      reader->GetOutput()->GetPointData()->GetNormals()->SetName("Normals");
      }
    output->ShallowCopy( reader->GetOutput() );
    }
  else if (fileNameStr.find(".fac") != std::string::npos)
    {
    vtkNew<vtkCUBITReader> reader;
    reader->SetFileName( fileNameStr.c_str() );
    reader->Update();
    output->ShallowCopy( reader->GetOutput() );
    }
  else if (fileNameStr.find(".sol") != std::string::npos)
    {
    vtkNew<vtkGMSSolidReader> reader;
    reader->SetFileName(this->FileName);
    reader->Update();
    vtkMultiBlockDataSet *multiblockData = reader->GetOutput();

    vtkNew<vtkAppendPolyData> append;
    for (unsigned int solidID = 0;
      solidID < multiblockData->GetNumberOfBlocks(); solidID++)
      {
      vtkNew<vtkPolyData> tmpPD;
      tmpPD->DeepCopy( multiblockData->GetBlock(solidID) );

      vtkIntArray *matValue = vtkIntArray::SafeDownCast(
        tmpPD->GetFieldData()->GetArray(ReaderHelperFunctions::GetMaterialTagName()) );
      int materialID = VTK_INT_MAX;
      if (matValue)
        {
        materialID = matValue->GetValue(0);
        }

      // material IDs
      vtkNew<vtkIntArray> matArray;
      matArray->SetNumberOfValues( tmpPD->GetNumberOfCells() );
      matArray->SetName( ReaderHelperFunctions::GetMaterialTagName() );

      // region IDs
      vtkNew<vtkIntArray> regionArray;
      regionArray->SetNumberOfValues( tmpPD->GetNumberOfCells() );
      regionArray->SetName( ReaderHelperFunctions::GetShellTagName() );

      // fill
      for (int i = 0; i < tmpPD->GetNumberOfCells(); i++)
        {
        matArray->SetValue(i, materialID);
        regionArray->SetValue(i, solidID);
        }

      tmpPD->GetCellData()->AddArray( matArray.GetPointer() );
      tmpPD->GetCellData()->AddArray( regionArray.GetPointer() );
      append->AddInputData( tmpPD.GetPointer());
      }

    append->Update();
    vtkNew<vtkCleanPolyData> clean;
    clean->SetInputConnection( append->GetOutputPort() );
    clean->Update();

    output->ShallowCopy( clean->GetOutput() );
    }
  else if (fileNameStr.find(".stl") != std::string::npos)
    {
    vtkNew<vtkCMBSTLReader> reader;
    reader->SetFileName( fileNameStr.c_str() );
    reader->Update();
    output->ShallowCopy( reader->GetOutput() );
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

//-----------------------------------------------------------------------------
void vtkCMBGeometryReader::PostProcessMesh(vtkDataSet *dataset,
                                            bool is3DVolumeMesh,
                                            bool passThroughPointIds,
                                            const char *regionArrayName,
                                            vtkPolyData *output)
{
  // do we need to run feature edges?
  if (!is3DVolumeMesh)
    {
    // check if any boundary edges... if not, then 2D surface mesh
    vtkNew<vtkFeatureEdges> featureEdges;
    featureEdges->BoundaryEdgesOn();
    featureEdges->FeatureEdgesOff();
    featureEdges->ManifoldEdgesOff();
    featureEdges->NonManifoldEdgesOff();

    // if not polydata, 1st make it polydata before feature edges filter
    vtkSmartPointer<vtkDataSetSurfaceFilter> surfaceFilter;
    if ( !vtkPolyData::SafeDownCast(dataset) )
      {
      surfaceFilter = vtkSmartPointer<vtkDataSetSurfaceFilter>::New();
      surfaceFilter->SetInputData( dataset );
      featureEdges->SetInputConnection( surfaceFilter->GetOutputPort() );
      }
    else
      {
      featureEdges->SetInputData( dataset );
      }
    featureEdges->Update();
    this->HasBoundaryEdges = featureEdges->GetOutput()->GetNumberOfLines() > 0 ?
      true : false;

    // we have what we need, either a closed shell or we have found that it has
    // edges but we're not preping for model creation
    if (!this->HasBoundaryEdges || !this->PrepNonClosedSurfaceForModelCreation)
      {
      if ( !vtkPolyData::SafeDownCast(dataset) )
        {
        output->ShallowCopy( surfaceFilter->GetOutput() );
        }
      else
        {
        output->ShallowCopy( dataset );
        }
      }
    else
      {
      vtkNew<vtkExtractRegionEdges> edgesFilter;
      edgesFilter->SetRegionArrayName(ReaderHelperFunctions::GetShellTagName());
      edgesFilter->SetInputData( dataset );
      edgesFilter->Update();
      this->RegionIdentifiersModified = edgesFilter->GetRegionIdentifiersModified();
      output->ShallowCopy(edgesFilter->GetOutput(0));
      }
    }
  else
    {
    vtkNew<vtkDataSetRegionSurfaceFilter> surfaceFilter;
    surfaceFilter->SetInputData( dataset );
    surfaceFilter->SetRegionArrayName(regionArrayName);
    surfaceFilter->SetPassThroughPointIds( passThroughPointIds );
    surfaceFilter->Update();
    output->ShallowCopy(surfaceFilter->GetOutput(0));
    }
}

//-----------------------------------------------------------------------------
void vtkCMBGeometryReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "File Name: "
     << (this->FileName ? this->FileName : "(none)") << "\n";
}


//----------------------------------------------------------------------------
int vtkCMBGeometryReader::RequestInformation(
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
