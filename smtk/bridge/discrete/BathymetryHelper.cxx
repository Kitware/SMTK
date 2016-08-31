//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "BathymetryHelper.h"

#ifdef HAS_GDAL_RASTER_READER
#include "smtk/extension/vtk/reader/vtkGDALRasterReader.h"
#include "smtk/extension/vtk/reader/vtkGDALRasterPolydataWrapper.h"
#endif

#include "smtk/extension/vtk/reader/vtkCMBGeometryReader.h"
#include "smtk/extension/vtk/reader/vtkLASReader.h"
#include "smtk/bridge/discrete/Session.h"
#include "smtk/mesh/Collection.h"
#include "smtk/mesh/MeshSet.h"
#include "smtk/mesh/PointSet.h"

#include "vtkAppendPoints.h"
#include "vtkCompositeDataIterator.h"
#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelWrapper.h"
#include "vtkDoubleArray.h"
#include "vtkFloatArray.h"
#include "vtkImageData.h"
#include "vtkImageToStructuredGrid.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkNew.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkStructuredGrid.h"
#include "vtkUniformGrid.h"
#include "vtkXMLImageDataReader.h"
#include <vtksys/SystemTools.hxx>
#include "DiscreteMesh.h"

#include <algorithm> // for std::transform

namespace smtk {
  namespace bridge {
    namespace discrete {

  template<class T>
  void copyArrayZValues(T *t, vtkImageData* imageInput, vtkPoints* outputPoints)
    {
    vtkIdType i, size = imageInput->GetNumberOfPoints();
    double p[3];
    for (i=0; i < size; ++i)
      {
      //get the point
      imageInput->GetPoint(i,p);
      p[2] = t[i];
      outputPoints->SetPoint(i,p);
      }
    }

/// Private constructor since this class is a base class which should not be instantiated.
BathymetryHelper::BathymetryHelper()
{
}

BathymetryHelper::~BathymetryHelper()
{
  this->m_filesToSources.clear();
}

bool BathymetryHelper::loadBathymetryFile(const std::string& filename)
{
  if (filename.empty())
    {
    std::cerr << "File name is empty!\n";
    return false;
    }
  // if the data is already loaded, return true;
  if(this->bathymetryData(filename) != NULL)
    return true;

  vtkDataSet* dataOutput = NULL;
  std::string ext = vtksys::SystemTools::GetFilenameLastExtension(filename);
  std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
  if(ext == ".pts" || ext == ".bin"
     || ext == ".vtk"
     || ext == ".vtp"
     || ext == ".2dm" || ext == ".3dm"
     || ext == ".tin"
     || ext == ".poly" || ext == ".smesh"
     || ext == ".obj"
     || ext == ".fac"
     || ext == ".sol"
     || ext == ".stl"
     )
    {
    vtkNew<vtkCMBGeometryReader> reader;
    reader->SetFileName( filename.c_str() );
    reader->SetPrepNonClosedSurfaceForModelCreation(false);
    reader->SetEnablePostProcessMesh(false);
    reader->Update();
    vtkPolyData* polyOutput = vtkPolyData::New();
    polyOutput->ShallowCopy( reader->GetOutput() );
    dataOutput = polyOutput;
    }
#ifdef HAS_GDAL_RASTER_READER
  else if (ext == ".dem")
    {
    vtkNew<vtkGDALRasterReader> reader;
    reader->SetFileName(filename.c_str());
    reader->Update();
    vtkUniformGrid* ugOutput = vtkUniformGrid::New();
    ugOutput->ShallowCopy( reader->GetOutput() );
    dataOutput = ugOutput;
    }
#endif
  else if(ext == ".las")
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
      if(!blockPoly)
        {
        std::cerr << "This block from LAS reader is not a polydata!\n";
        continue;
        }
      appendPoints->AddInputData(blockPoly);
      }
    iter->Delete();
    appendPoints->Update();

    vtkPolyData* polyOutput = vtkPolyData::New();
    polyOutput->ShallowCopy( appendPoints->GetOutput() );
    dataOutput = polyOutput;
    }
  else if(ext == ".vti")
    {
    vtkNew<vtkXMLImageDataReader> reader;
    reader->SetFileName(filename.c_str());
    reader->Update();

//    vtkNew<vtkImageToStructuredGrid> image2struct;
//    image2struct->SetInputData(readout);
//    image2struct->Update();
//    imagepoly->SetPoints(image2struct->GetOutput()->GetPoints());
    vtkImageData* imgOutput = vtkImageData::New();
    imgOutput->ShallowCopy( reader->GetOutput() );
    dataOutput = imgOutput;
    }
  else
    {
    std::cerr << "File type is not supported!\n";
    return false;
    }

  this->m_filesToSources[filename].TakeReference(dataOutput);
  return true;
}

vtkDataSet* BathymetryHelper::bathymetryData(const std::string& filename)
{
  if(this->m_filesToSources.find(filename) !=
    this->m_filesToSources.end())
    return this->m_filesToSources[filename];
  return NULL;
}

void BathymetryHelper::loadedBathymetryFiles(
  std::vector<std::string> &result) const
{
  result.clear();
  std::map<std::string, vtkSmartPointer<vtkDataSet> >::const_iterator it;
  for(it=this->m_filesToSources.begin(); it!=this->m_filesToSources.end(); ++it)
    result.push_back(it->first);
}

void BathymetryHelper::addModelBathymetry(const smtk::common::UUID& modelId,
                          const std::string& bathyfile)
{
  this->m_modelIdsToBathymetrys[modelId] = bathyfile;
}

void BathymetryHelper::removeModelBathymetry(const smtk::common::UUID& modelId)
{
  this->m_modelIdsToBathymetrys.erase(modelId);
}

bool BathymetryHelper::hasModelBathymetry(const smtk::common::UUID& modelId)
{
  return this->m_modelIdsToBathymetrys.find(modelId) != this->m_modelIdsToBathymetrys.end();
}

vtkPolyData* BathymetryHelper::findOrShallowCopyModelPoly(
  const smtk::common::UUID& modelId, smtk::bridge::discrete::Session* session)
{
  if(this->m_modelIdsToMasterPolys.find(modelId) != this->m_modelIdsToMasterPolys.end())
    return this->m_modelIdsToMasterPolys[modelId].GetPointer();

  vtkDiscreteModelWrapper* modelWrap = session->findModelEntity(modelId);
  if(!modelWrap)
    return NULL;

  vtkPolyData* tmpModelPoly = vtkPolyData::New();
  tmpModelPoly->Initialize();
  vtkNew<vtkPoints> points;
  points->ShallowCopy(modelWrap->GetModel()->GetMesh().SharePointsPtr());
//  tmmModelPoly->ShallowCopy(modelWrap->GetModel()->GetMesh().ShallowCopyFaceData);
  tmpModelPoly->SetPoints(points.GetPointer());

  this->m_modelIdsToMasterPolys[modelId].TakeReference(tmpModelPoly);
  return this->m_modelIdsToMasterPolys[modelId].GetPointer();
}

bool BathymetryHelper::storeMeshPointsZ(smtk::mesh::CollectionPtr collection)
{
  if(!collection->isValid())
    {
    return false;
    }
  // if this mesh is already cached, return true;
  std::vector<double> originalZs(this->cachedMeshPointsZ(collection->entity()));
  if(originalZs.size() > 0)
    {
    return true;
    }
  // don't use originalZs here, because that's m_dummy
  std::vector<double> zvals;
  smtk::mesh::MeshSet meshes = collection->meshes();
  ZValueHelper functorA(zvals, false);
  smtk::mesh::for_each( meshes.points(), functorA );
  this->m_meshIdsToPoints[collection->entity()] = zvals;
  return true;
}

bool BathymetryHelper::resetMeshPointsZ(smtk::mesh::CollectionPtr collection)
{
  if(!collection->isValid())
    {
    return false;
    }
  // if removing, and we don't have a cached Z-value array, we did not apply bathy yet
  std::vector<double> originalZs(this->cachedMeshPointsZ(collection->entity()));
  if(originalZs.size() == 0)
    {
    return true;
    }

  smtk::mesh::MeshSet meshes = collection->meshes();
  ZValueHelper functorA(originalZs, true);
  smtk::mesh::for_each( meshes.points(), functorA );
  return true;
}

const std::vector<double>& BathymetryHelper::cachedMeshPointsZ(
  const smtk::common::UUID& collectionId) const
{
  BathymetryHelper::MeshIdToPointsMap::const_iterator it =
    this->m_meshIdsToPoints.find(collectionId);

  if(it != this->m_meshIdsToPoints.end())
    {
    return it->second;
    }
  return m_dummy;
}

void BathymetryHelper::clear()
{
  this->m_filesToSources.clear();
  this->m_modelIdsToMasterPolys.clear();
  this->m_modelIdsToBathymetrys.clear();
}

bool BathymetryHelper::computeBathymetryPoints(
  vtkDataSet* input, vtkPoints* outputPoints)
{
  if(!input || !outputPoints)
    {
    return false;
    }
  vtkIdType numPoints = 0;
  vtkPolyData* pd = vtkPolyData::SafeDownCast(input);
  vtkUniformGrid* gridInput = vtkUniformGrid::SafeDownCast(input);
  vtkImageData* imageInput = vtkImageData::SafeDownCast(input);
  if (pd)
    {
    numPoints = pd->GetNumberOfPoints();
    }
  else if(imageInput)
    {
    outputPoints->SetDataType(VTK_DOUBLE);
    numPoints = imageInput->GetNumberOfPoints();
    }
  if(numPoints <=0)
    {
    return false;
    }

  //second iteration is building the point set and elevation mapping
  vtkIdType size, i;
  double p[3];
  // Uniform Grids may not have the max number of points
  if (gridInput)
    {
    vtkDataArray *dataArray = gridInput->GetPointData()->GetScalars("Elevation");
    if(!dataArray || dataArray->GetNumberOfTuples() != numPoints)
      {
      return false;
      }
    outputPoints->SetNumberOfPoints(numPoints);
    vtkIdType at = 0;
    for (i = 0; i < numPoints; ++i)
      {
      if (!gridInput->IsPointVisible(i))
        {
        continue;
        }
      imageInput->GetPoint(i,p);

      //copy z from "Elevation"
      p[2] = dataArray->GetTuple1(i);
      outputPoints->SetPoint(at, p);
      ++at;
      }
    outputPoints->Resize(at);
    }
  else
    {
    outputPoints->SetNumberOfPoints(numPoints);
    if (pd)
      {
      outputPoints->ShallowCopy(pd->GetPoints());
      }
    else if(imageInput)
      {
      size = imageInput->GetNumberOfPoints();
      outputPoints->SetNumberOfPoints(size);
      vtkDataArray *dataArray = imageInput->GetPointData()->GetScalars("Elevation");
      if(!dataArray || dataArray->GetNumberOfTuples() != size)
        {
        return false;
        }

      //copy the z value of the "Elevation"
      if (dataArray->GetDataType() == VTK_FLOAT)
        {
        vtkFloatArray *floatArray = static_cast<vtkFloatArray *>(dataArray);
        copyArrayZValues(floatArray->GetPointer(0), imageInput, outputPoints);
        }
      else if (dataArray->GetDataType() == VTK_DOUBLE)
        {
        vtkDoubleArray *doubleArray = static_cast<vtkDoubleArray *>(dataArray);
        copyArrayZValues(doubleArray->GetPointer(0), imageInput, outputPoints);
        }
      }
    }
  return true;
}

    } // namespace discrete
  } // namespace bridge
} // namespace smtk
