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

#include "smtk/extension/vtk/reader/vtkCMBGeometryReader.h"
#include "smtk/extension/vtk/reader/vtkLASReader.h"
#include "smtk/mesh/Collection.h"
#include "smtk/mesh/MeshSet.h"
#include "smtk/mesh/PointSet.h"

#include "smtk/model/Model.h"
#include "smtk/model/CellEntity.h"
#include "smtk/model/Group.h"
#include "smtk/model/Session.h"

#include "vtkAppendPoints.h"
#include "vtkCompositeDataIterator.h"
#include "vtkDoubleArray.h"
#include "vtkFloatArray.h"
#include "vtkGDALRasterReader.h"
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

#include <algorithm> // for std::transform

using namespace smtk::model;

namespace smtk {
  namespace model {

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
  else if (ext == ".dem")
    {
    vtkNew<vtkGDALRasterReader> reader;
    reader->SetFileName(filename.c_str());
    reader->Update();
    vtkUniformGrid* ugOutput = vtkUniformGrid::New();
    ugOutput->ShallowCopy( reader->GetOutput() );
    dataOutput = ugOutput;
    }
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

bool BathymetryHelper::storeMeshPointsZ(smtk::mesh::CollectionPtr collection)
{
  if(!collection->isValid())
    {
    std::cout << "invalid collection"<< std::endl;
    return false;
    }

  // if this mesh is already cached, return true;
  if(collection->hasFloatProperty(collection->meshes(),BO_elevation))
    {
    std::cout << "We alrady have this float property\n";
    return true;
    }

  std::vector<double> zvals;
  smtk::mesh::MeshSet meshes = collection->meshes();
  bool result = meshes.points().get(zvals);
  if (!result)
    zvals = this->m_dummy;
  collection->setFloatProperty(collection->meshes(),BO_elevation,static_cast<smtk::model::FloatList>(zvals));
  return true;
}

bool BathymetryHelper::resetMeshPointsZ(smtk::mesh::CollectionPtr collection)
{
  if(!collection->isValid())
    {
    std::cout << "collection is invalid!\n";
    return false;
    }
  // if removing, and we don't have a cached Z-value array, we did not apply bathy yet.
  if(!collection->hasFloatProperty(collection->meshes(),BO_elevation))
    {
    std::cout << "original Z value cache does not exist!\n";
    return true;
    }
  std::vector<double> originalZs(collection->floatProperty(collection->meshes(),BO_elevation));
  // Check whether the originalZ is empty
  if(originalZs.size() == 0)
    {
    std::cout << "original Z value cache is empty!\n";
    return true;
    }
  smtk::mesh::MeshSet meshes = collection->meshes();
  meshes.points().set(originalZs);
  collection->removeFloatProperty(collection->meshes(),BO_elevation);
  return true;
}

void BathymetryHelper::clear()
{
  this->m_filesToSources.clear();
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

/// get the points from EntityRef and feed it into pts. Possible duplcate points (Vertex, Edges, faces) possible performance improvement
vtkIdType BathymetryHelper::GenerateRepresentationFromModel(
    vtkPoints *pts, const EntityRef &entityref)
  {
  const smtk::model::Tessellation* tess;
  if (!(tess = entityref.hasTessellation()))
    { // Oops.
    return 0;
    }
  vtkIdType npts = tess->coords().size() / 3;
  smtk::model::Entity* entity;
  if (entityref.isValid(&entity))
    {
    for (vtkIdType i = 0; i < npts; ++i)
      {
      pts->InsertNextPoint(&tess->coords()[3*i]);
      }
    }
  return npts;
  }

/// set points from pts in EntityRef
void BathymetryHelper::CopyCoordinatesToTessellation(
    vtkPoints *pts, const EntityRef &entityref, const vtkIdType startingIndex)
  {
  smtk::model::Tessellation* tess;
  if (!(tess = const_cast<smtk::model::Tessellation*>(entityref.hasTessellation())))
    { // Oops.
    std::cerr << "Do not have tessellation" << std::endl;
    }
  smtk::model::Entity* entity;
  vtkIdType npts = tess->coords().size() / 3;
  if (entityref.isValid(&entity) && (startingIndex + npts <= pts->GetNumberOfPoints()))
    {
    for (vtkIdType i = 0; i < npts; ++i)
      {
      //set the points in pts into tess( a double vector)
      double currentPoint[3];
      pts->GetPoint(startingIndex + i, currentPoint);
      tess->setPoint(static_cast<std::size_t>(i),currentPoint);
      }
    }

  }

  /// get points' z value as a vector from masterModelPts
  void BathymetryHelper::GetZValuesFromMasterModelPts(
        vtkPoints *pts, std::vector<double> & zValues)
  {
    vtkIdType npts = pts->GetNumberOfPoints();
    for (vtkIdType i = 0; i< npts; ++i)
    {
      double tmp[3];
      pts->GetPoint(i, tmp);
      zValues.push_back(tmp[2]);
    }

  }

  /// set points' z value inside masterModelPts
  bool BathymetryHelper::SetZValuesIntoMasterModelPts(vtkPoints *pts, const std::vector<double>* zValues)
  {
    if (static_cast<size_t>(pts->GetNumberOfPoints()) != zValues->size())
    {
      std::cerr << "When removing Bathymetry, the size of points and zValues "
                   "does not match!\n";
      return false;
    }
    for (vtkIdType i = 0; i < pts->GetNumberOfPoints(); ++i)
    {
      double tmp[3];
      pts->GetPoint(i,tmp);
      pts->SetPoint(i,tmp[0], tmp[1], (*zValues)[i]);
    }
    return true;
  }


/**************************************************************************/
  } // namespace model
} // namespace smtk
